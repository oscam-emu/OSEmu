#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "des.h"
#include "bn.h"
#include "idea.h"
#include "via3surenc.h"
#include "emulator.h"
#include "globals.h"
#include "helpfunctions.h"

// Key DB
int CharToBin(unsigned char *out, char *in, unsigned int inLen)
{
  unsigned int i, tmp;
  for(i=0; i<inLen/2; i++) {
    if(sscanf(in + i*2, "%02X", &tmp) != 1) return 0;
    out[i] = (unsigned char)tmp;
  }
  return 1;
}

typedef struct {
  char identifier;
  unsigned int provider;
  char keyName[8];
  unsigned char *key;
  unsigned int keyLength;
} KeyData;

typedef struct {
  KeyData *EmuKeys;
  unsigned int keyCount;
  unsigned int keyMax;
} KeyDataContainer;

KeyDataContainer CwKeys = { .EmuKeys=NULL, .keyCount=0, .keyMax=0 };
KeyDataContainer ViKeys = { .EmuKeys=NULL, .keyCount=0, .keyMax=0 };
KeyDataContainer NagraKeys = { .EmuKeys=NULL, .keyCount=0, .keyMax=0 };
KeyDataContainer IrdetoKeys = { .EmuKeys=NULL, .keyCount=0, .keyMax=0 };

KeyDataContainer *GetKeyContainer(char identifier) 
{
  switch(identifier) {
    case 'W': return &CwKeys;
    case 'V': return &ViKeys;
    case 'N': return &NagraKeys;
    case 'I': return &IrdetoKeys;  	
    default:  return NULL;
  }
  return NULL;
}

int SetKey(char identifier, unsigned int provider, char *keyName, unsigned char *key, unsigned int keyLength) 
{
  unsigned int i;
  KeyDataContainer *KeyDB;
  identifier = (char)toupper((int)identifier);
  
  KeyDB = NULL;
  KeyDB = GetKeyContainer(identifier);
  if(KeyDB == NULL) return 0;

  for(i=0; i<KeyDB->keyCount; i++) {
    if(KeyDB->EmuKeys[i].provider != provider) continue;  
    if(strcmp(KeyDB->EmuKeys[i].keyName, keyName)) continue;
	
	free(KeyDB->EmuKeys[i].key);
    KeyDB->EmuKeys[i].key = key;
    KeyDB->EmuKeys[i].keyLength = keyLength;
    return 1;
  }	

  if(KeyDB->keyCount+1 > KeyDB->keyMax) {
   if(KeyDB->EmuKeys == NULL) {
     KeyDB->EmuKeys = (KeyData*)malloc(sizeof(KeyData)*(KeyDB->keyMax+64));
     if(KeyDB->EmuKeys == NULL) return 0;
     KeyDB->keyMax+=64;
   }
   else {
     KeyDB->EmuKeys = (KeyData*)realloc(KeyDB->EmuKeys, sizeof(KeyData)*(KeyDB->keyMax+16));
     if(KeyDB->EmuKeys == NULL) return 0;
     KeyDB->keyMax+=16;
   }
  }

  KeyDB->EmuKeys[KeyDB->keyCount].identifier = identifier;
  KeyDB->EmuKeys[KeyDB->keyCount].provider = provider;
  memcpy(KeyDB->EmuKeys[KeyDB->keyCount].keyName, keyName, 8);
  KeyDB->EmuKeys[KeyDB->keyCount].keyName[7] = 0;
  KeyDB->EmuKeys[KeyDB->keyCount].key = key;
  KeyDB->EmuKeys[KeyDB->keyCount].keyLength = keyLength;  
  KeyDB->keyCount++;
  return 1;
}

int FindKey(char identifier, unsigned int provider, char *keyName, unsigned char *key, unsigned int maxKeyLength)
{
  unsigned int i;
  KeyDataContainer *KeyDB;
  
  KeyDB = NULL;
  KeyDB = GetKeyContainer(identifier);
  if(KeyDB == NULL) return 0; 
  
  for(i=0; i<KeyDB->keyCount; i++) {
    if(KeyDB->EmuKeys[i].provider != provider) continue;  
    if(strcmp(KeyDB->EmuKeys[i].keyName, keyName)) continue;	
    memcpy(key, KeyDB->EmuKeys[i].key, KeyDB->EmuKeys[i].keyLength > maxKeyLength ? maxKeyLength : KeyDB->EmuKeys[i].keyLength);
    return 1;
  }
  return 0;  
}

void ReadKeyFile(char *path)
{
  char line[2048], keyName[8], keyString[1026];
  unsigned int pathLength, provider, keyLength;
  unsigned char *key;
  char *filepath, *filename;
  FILE *file = NULL;
  char identifier;

  if(strstr(path, "SoftCam.Key")) filename = "";
  else {
    pathLength = strlen(path);
    if(path[pathLength-1] == '/' || path[pathLength-1] == '\\') 
      filename = "SoftCam.Key";
    else
      filename = "/SoftCam.Key";
  }
  pathLength = strlen(path)+strlen(filename)+1;
  filepath = (char*)malloc(pathLength);
  if(filepath == NULL) return;
  snprintf(filepath, pathLength, "%s%s", path, filename);
  file = fopen(filepath, "r");
  free(filepath);
  if(file == NULL) return;
  
  while(fgets(line, 2048, file)) {
    if(sscanf(line, "%c %8x %7s %1024s", &identifier, &provider, keyName, keyString) != 4) continue;
    
    keyLength = strlen(keyString)/2;
    key = (unsigned char*)malloc(keyLength);
    if(key == NULL)  { fclose(file); return; }
    
    if(!CharToBin(key, keyString, strlen(keyString))
      || !SetKey(identifier, provider, keyName, key, keyLength));
      free(key);
  }
  fclose(file);
}

#ifndef __APPLE__
extern char SoftCamKey_Data[]    asm("_binary_SoftCam_Key_start");
extern char SoftCamKey_DataEnd[] asm("_binary_SoftCam_Key_end");

void ReadKeyMemory(void)
{
  char *keyData, *line, *saveptr, keyName[8], keyString[1026];
  unsigned int provider, keyLength;
  unsigned char *key;
  char identifier;

  keyData = (unsigned char*)malloc(SoftCamKey_DataEnd-SoftCamKey_Data+1);
  if(keyData == NULL) return;
  memcpy(keyData, SoftCamKey_Data, SoftCamKey_DataEnd-SoftCamKey_Data);
  keyData[SoftCamKey_DataEnd-SoftCamKey_Data] = 0x00;
  
  line = strtok_r(keyData, "\n", &saveptr);
  while(line != NULL) {
    if(sscanf(line, "%c %8x %7s %1024s", &identifier, &provider, keyName, keyString) != 4) {
    	 line = strtok_r(NULL, "\n", &saveptr);
    	 continue;
    }
    keyLength = strlen(keyString)/2;
    key = (unsigned char*)malloc(keyLength);
    if(key == NULL) { free(keyData); return; }
    
    if(!CharToBin(key, keyString, strlen(keyString))
      || !SetKey(identifier, provider, keyName, key, keyLength));
      free(key);  
    line = strtok_r(NULL, "\n", &saveptr);
  }
  free(keyData);
}
#endif

// Shared functions
void ReverseMem(unsigned char *in, int len) 
{
  unsigned char temp;
  int32_t i;
  for(i = 0; i < (len / 2); i++) {
    temp = in[i];
    in[i] = in[len - i - 1];
    in[len - i - 1] = temp;
  }
}

void ReverseMemInOut(unsigned char *out, const unsigned char *in, int n)
{
  if(n>0) {
    out+=n;
    do { *(--out)=*(in++); } while(--n);
  }
}

char EmuRSAInput(BIGNUM *d, const unsigned char *in, int n, char LE)
{
  if(LE) {
    unsigned char tmp[256];
    ReverseMemInOut(tmp,in,n);
    return BN_bin2bn(tmp,n,d)!=0;
  }
  else
    return BN_bin2bn(in,n,d)!=0;
}

int EmuRSAOutput(unsigned char *out, int n, BIGNUM *r, char LE)
{
  int s=BN_num_bytes(r);
  if(s>n) {
    unsigned char buff[256];
    BN_bn2bin(r,buff);
    memcpy(out,buff+s-n,n);
  }
  else if(s<n) {
    int l=n-s;
    memset(out,0,l);
    BN_bn2bin(r,out+l);
  }
  else BN_bn2bin(r,out);
  if(LE) ReverseMem(out,n);
  return s;
}

int EmuRSA(unsigned char *out, const unsigned char *in, int n, BIGNUM *exp, BIGNUM *mod, char LE)
{
  BN_CTX *ctx;
  BIGNUM *r, *d;
  int result = 0;

  ctx = BN_CTX_new();
  r = BN_new();
  d = BN_new();
  
  if(EmuRSAInput(d,in,n,LE) && BN_mod_exp(r,d,exp,mod,ctx)) 
    result = EmuRSAOutput(out,n,r,LE);
  
  BN_free(d);
  BN_free(r);
  BN_CTX_free(ctx);
  return result;
}

// Cryptoworks EMU
char GetCwKey(unsigned char *buf,unsigned int ident, unsigned char keyIndex, unsigned int keyLength) {
  
  char keyName[8];
  unsigned int tmp;
  
  if((ident>>4)== 0xD02A) keyIndex &=0xFE; // map to even number key indexes
  if((ident>>4)== 0xD00C) ident = 0x0D00C0; // map provider C? to C0
  else if(keyIndex==6 && ((ident>>8) == 0x0D05)) ident = 0x0D0504; // always use provider 04 system key
  
  tmp = keyIndex;
  snprintf(keyName, 8, "%.2X", tmp);
  if(FindKey('W', ident, keyName, buf, keyLength))
   return 1;  

  return 0;
}

static const unsigned char cw_sbox1[64] =
{
    0xD8,0xD7,0x83,0x3D,0x1C,0x8A,0xF0,0xCF,0x72,0x4C,0x4D,0xF2,0xED,0x33,0x16,0xE0,
    0x8F,0x28,0x7C,0x82,0x62,0x37,0xAF,0x59,0xB7,0xE0,0x00,0x3F,0x09,0x4D,0xF3,0x94,
    0x16,0xA5,0x58,0x83,0xF2,0x4F,0x67,0x30,0x49,0x72,0xBF,0xCD,0xBE,0x98,0x81,0x7F,
    0xA5,0xDA,0xA7,0x7F,0x89,0xC8,0x78,0xA7,0x8C,0x05,0x72,0x84,0x52,0x72,0x4D,0x38
};
static const unsigned char cw_sbox2[64] =
{
    0xD8,0x35,0x06,0xAB,0xEC,0x40,0x79,0x34,0x17,0xFE,0xEA,0x47,0xA3,0x8F,0xD5,0x48,
    0x0A,0xBC,0xD5,0x40,0x23,0xD7,0x9F,0xBB,0x7C,0x81,0xA1,0x7A,0x14,0x69,0x6A,0x96,
    0x47,0xDA,0x7B,0xE8,0xA1,0xBF,0x98,0x46,0xB8,0x41,0x45,0x9E,0x5E,0x20,0xB2,0x35,
    0xE4,0x2F,0x9A,0xB5,0xDE,0x01,0x65,0xF8,0x0F,0xB2,0xD2,0x45,0x21,0x4E,0x2D,0xDB
};
static const unsigned char cw_sbox3[64] =
{
    0xDB,0x59,0xF4,0xEA,0x95,0x8E,0x25,0xD5,0x26,0xF2,0xDA,0x1A,0x4B,0xA8,0x08,0x25,
    0x46,0x16,0x6B,0xBF,0xAB,0xE0,0xD4,0x1B,0x89,0x05,0x34,0xE5,0x74,0x7B,0xBB,0x44,
    0xA9,0xC6,0x18,0xBD,0xE6,0x01,0x69,0x5A,0x99,0xE0,0x87,0x61,0x56,0x35,0x76,0x8E,
    0xF7,0xE8,0x84,0x13,0x04,0x7B,0x9B,0xA6,0x7A,0x1F,0x6B,0x5C,0xA9,0x86,0x54,0xF9
};
static const unsigned char cw_sbox4[64] =
{
    0xBC,0xC1,0x41,0xFE,0x42,0xFB,0x3F,0x10,0xB5,0x1C,0xA6,0xC9,0xCF,0x26,0xD1,0x3F,
    0x02,0x3D,0x19,0x20,0xC1,0xA8,0xBC,0xCF,0x7E,0x92,0x4B,0x67,0xBC,0x47,0x62,0xD0,
    0x60,0x9A,0x9E,0x45,0x79,0x21,0x89,0xA9,0xC3,0x64,0x74,0x9A,0xBC,0xDB,0x43,0x66,
    0xDF,0xE3,0x21,0xBE,0x1E,0x16,0x73,0x5D,0xA2,0xCD,0x8C,0x30,0x67,0x34,0x9C,0xCB
};
static const unsigned char AND_bit1[8] = {0x00,0x40,0x04,0x80,0x21,0x10,0x02,0x08};
static const unsigned char AND_bit2[8] = {0x80,0x08,0x01,0x40,0x04,0x20,0x10,0x02};
static const unsigned char AND_bit3[8] = {0x82,0x40,0x01,0x10,0x00,0x20,0x04,0x08};
static const unsigned char AND_bit4[8] = {0x02,0x10,0x04,0x40,0x80,0x08,0x01,0x20};

void CW_SWAP_KEY(unsigned char *key)
{
    unsigned char k[8];
    memcpy(k, key, 8);
    memcpy(key, key + 8, 8);
    memcpy(key + 8, k, 8);
}
void CW_SWAP_DATA(unsigned char *k)
{
    unsigned char d[4];
    memcpy(d, k + 4, 4);
    memcpy(k + 4 ,k ,4);
    memcpy(k, d, 4);
}
void CW_DES_ROUND(unsigned char *d,unsigned char *k)
{
    unsigned char aa[44] = {1,0,3,1,2,2,3,2,1,3,1,1,3,0,1,2,3,1,3,2,2,0,7
    ,6,5,4,7,6,5,7,6,5,6,7,5,7,5,7,6,6,7,5,4,4},
    bb[44] = {0x80,0x08,0x10,0x02,0x08,0x40,0x01,0x20,0x40,0x80,0x04,0x10,0x04,
        0x01,0x01,0x02,0x20,0x20,0x02,0x01,
        0x80,0x04,0x02,0x02,0x08,0x02,0x10,0x80,0x01,0x20,0x08,0x80,0x01
    ,0x08,0x40,0x01,0x02,0x80,0x10,0x40,0x40,0x10,0x08,0x01},
    ff[4] = {0x02,0x10,0x04,0x04},
    l[24] = {0,2,4,6,7,5,3,1,4,5,6,7,7,6,5,4,7,4,5,6,4,7,6,5};
    unsigned char des_td[8], i, o, n, c = 1, m = 0, r = 0, *a = aa,*b = bb, *f = ff, *p1 = l,*p2 = l+8,*p3 = l+16;
    for (m = 0; m < 2; m++)
    for(i = 0; i < 4; i++)
    des_td[*p1++] = (m) ? ((d[*p2++]*2) & 0x3F) | ((d[*p3++] & 0x80) ? 0x01 : 0x00):
    (d[*p2++]/2) | ((d[*p3++] & 0x01) ? 0x80 : 0x00);
    for (i = 0; i < 8; i++)
    {
        c = (c) ? 0 : 1; r = (c) ? 6 : 7; n = (i) ? i-1 : 1;
        o = (c) ? ((k[n] & *f++) ? 1 : 0) : des_td[n];
        for (m = 1; m < r; m++)
        o = (c) ? (o*2) | ((k[*a++] & *b++) ? 0x01 : 0x00) :
        (o/2) | ((k[*a++] & *b++) ? 0x80 : 0x00);
        n = (i) ? n+1 : 0;
        des_td[n] = (c) ? des_td[n] ^ o : (o ^ des_td[n] )/4;
    }
    for( i = 0; i < 8; i++)
    {
        d[0] ^= (AND_bit1[i] & cw_sbox1[des_td[i]]);
        d[1] ^= (AND_bit2[i] & cw_sbox2[des_td[i]]);
        d[2] ^= (AND_bit3[i] & cw_sbox3[des_td[i]]);
        d[3] ^= (AND_bit4[i] & cw_sbox4[des_td[i]]);
    }
    CW_SWAP_DATA(d);
}
void CW_48_Key(unsigned char *inkey, unsigned char *outkey, unsigned char algotype)
{
    unsigned char Round_Counter ,i = 8,*key128 = inkey, *key48 = inkey + 0x10;
    Round_Counter = 7 - (algotype & 7);
    memset (outkey, 0, 16);
    memcpy(outkey, key48, 6);
    for( ; i > Round_Counter;i--)
    if (i > 1)
    outkey[i-2] = key128[i];
}
void CW_LS_DES_KEY(unsigned char *key,unsigned char Rotate_Counter)
{
    unsigned char round[] = {1,2,2,2,2,2,2,1,2,2,2,2,2,2,1,1}, i, n;
    unsigned short k[8];
    n = round[Rotate_Counter];
    for (i = 0; i < 8; i++) k[i] = key[i];
    for (i = 1; i < n + 1; i++)
    {
        k[7] = (k[7]*2) | ((k[4] & 0x008) ? 1 : 0);
        k[6] = (k[6]*2) | ((k[7] & 0xF00) ? 1 : 0); k[7] &=0xff;
        k[5] = (k[5]*2) | ((k[6] & 0xF00) ? 1 : 0); k[6] &=0xff;
        k[4] = ((k[4]*2) | ((k[5] & 0xF00) ? 1 : 0)) & 0xFF; k[5] &= 0xff;
        k[3] = (k[3]*2) | ((k[0] & 0x008) ? 1 : 0);
        k[2] = (k[2]*2) | ((k[3] & 0xF00) ? 1 : 0); k[3] &= 0xff;
        k[1] = (k[1]*2) | ((k[2] & 0xF00) ? 1 : 0); k[2] &= 0xff;
        k[0] = ((k[0]*2) | ((k[1] & 0xF00) ? 1 : 0)) & 0xFF; k[1] &= 0xff;
    }
    for (i = 0; i < 8; i++) key[i] = (unsigned char) k[i];
}
void CW_RS_DES_KEY(unsigned char *k, unsigned char Rotate_Counter)
{
    unsigned char i,c;
    for (i = 1; i < Rotate_Counter+1; i++)
    {
        c = (k[3] & 0x10) ? 0x80 : 0;
        k[3] /= 2; if (k[2] & 1) k[3] |= 0x80;
        k[2] /= 2; if (k[1] & 1) k[2] |= 0x80;
        k[1] /= 2; if (k[0] & 1) k[1] |= 0x80;
        k[0] /= 2; k[0] |= c ;
        c = (k[7] & 0x10) ? 0x80 : 0;
        k[7] /= 2; if (k[6] & 1) k[7] |= 0x80;
        k[6] /= 2; if (k[5] & 1) k[6] |= 0x80;
        k[5] /= 2; if (k[4] & 1) k[5] |= 0x80;
        k[4] /= 2; k[4] |= c;
    }
}
void CW_RS_DES_SUBKEY(unsigned char *k, unsigned char Rotate_Counter)
{
    unsigned char round[] = {1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};
    CW_RS_DES_KEY(k, round[Rotate_Counter]);
}
void CW_PREP_KEY(unsigned char *key )
{
    unsigned char DES_key[8],j;
    int Round_Counter = 6,i,a;
    key[7] = 6;
    memset(DES_key, 0 , 8);
    do
    {
        a = 7;
        i = key[7];
        j = key[Round_Counter];
        do
        {
            DES_key[i] = ( (DES_key[i] * 2) | ((j & 1) ? 1: 0) ) & 0xFF;
            j /=2;
            i--;
            if (i < 0) i = 6;
            a--;
        } while (a >= 0);
        key[7] = i;
        Round_Counter--;
    } while ( Round_Counter >= 0 );
    a = DES_key[4];
    DES_key[4] = DES_key[6];
    DES_key[6] = a;
    DES_key[7] = (DES_key[3] * 16) & 0xFF;
    memcpy(key,DES_key,8);
    CW_RS_DES_KEY(key,4);
}
void CW_L2DES(unsigned char *data, unsigned char *key, unsigned char algo)
{
    unsigned char i, k0[22], k1[22];
    memcpy(k0,key,22);
    memcpy(k1,key,22);
    CW_48_Key(k0, k1,algo);
    CW_PREP_KEY(k1);
    for (i = 0; i< 2; i++)
    {
        CW_LS_DES_KEY( k1,15);
        CW_DES_ROUND( data ,k1);
    }
}
void CW_R2DES(unsigned char *data, unsigned char *key, unsigned char algo)
{
    unsigned char i, k0[22],k1[22];
    memcpy(k0,key,22);
    memcpy(k1,key,22);
    CW_48_Key(k0, k1, algo);
    CW_PREP_KEY(k1);
    for (i = 0;i< 2; i++)
    CW_LS_DES_KEY(k1,15);
    for (i = 0;i< 2; i++)
    {
        CW_DES_ROUND( data ,k1);
        CW_RS_DES_SUBKEY(k1,1);
    }
    CW_SWAP_DATA(data);
}
void CW_DES(unsigned char *data, unsigned char *inkey, unsigned char m)
{
    unsigned char key[22], i;
    memcpy(key, inkey + 9, 8);
    CW_PREP_KEY( key );
    for (i = 16; i > 0; i--)
    {
        if (m == 1) CW_LS_DES_KEY(key, (unsigned char) (i-1));
        CW_DES_ROUND( data ,key);
        if (m == 0) CW_RS_DES_SUBKEY(key, (unsigned char) (i-1));
    }
}
void CW_DEC_ENC(unsigned char *d, unsigned char *k, unsigned char a,unsigned char m)
{
    unsigned char n = m & 1;
    CW_L2DES(d , k, a);
    CW_DES (d , k, n);
    CW_R2DES(d , k, a);
    if (m & 2) CW_SWAP_KEY(k);
}
void CW_DEC(unsigned char *d, unsigned char *key, unsigned char algo)
{
    unsigned char k0[22], algo_type = algo & 7,mode = 0, i;
    memcpy(k0,key,22);
    if (algo_type < 7)
    CW_DEC_ENC(d , k0, algo_type, mode);
    else
    for (i = 0; i < 3; i++)
    {
        mode = !mode;
        CW_DEC_ENC(d , k0, algo_type ,(unsigned char) (mode | 2));
    }
}
void CW_ENC(unsigned char *d, unsigned char *key, unsigned char algo)
{
    unsigned char k0[22], algo_type = algo & 7,mode = 1, i;
    memcpy(k0,key,22);
    if (algo_type < 7)
    CW_DEC_ENC(d , k0, algo_type, mode);
    else
    for (i = 0; i < 3; i++)
    {
        mode = !mode;
        CW_DEC_ENC(d , k0, algo_type , (unsigned char) (mode | 2));
    }
}

void Cryptoworks3DES(unsigned char *data, unsigned char *key) {
  doPC1(key);
  doPC1(&key[8]);
  des(key, DES_ECS2_DECRYPT, data);
  des(&key[8], DES_ECS2_CRYPT, data);
  des(key, DES_ECS2_DECRYPT, data);
}

unsigned char CryptoworksProcessNano80(unsigned char *data, int provider, unsigned char *opKey, unsigned char nanoLength) 
{
  int i, j;
  unsigned char key[16], desKey[16], t[8], dat1[8], dat2[8], k0D00C000[16];
  if(provider != 0xA0 && !GetCwKey(k0D00C000, 0x0D00C000, 0, 16)) return 2;
  memset(t, 0, 8);

  memcpy(dat1, data, 8);
  if(provider == 0xA0) memcpy(key, opKey, 16);
  else memcpy(key, k0D00C000, 16);
  Cryptoworks3DES(data, key);
  memcpy(desKey, data, 8);  
  
  memcpy(data, dat1, 8);  
  if(provider == 0xA0){
    memcpy(key, &opKey[8], 8);
    memcpy(&key[8], opKey, 8);
  }
  else{
    memcpy(key, &k0D00C000[8], 8);
    memcpy(&key[8], k0D00C000, 8);
  }
  Cryptoworks3DES(data, key);
  memcpy(&desKey[8], data, 8);
  
  for(i=8; i+7<nanoLength; i+=8) {
    memcpy(dat1, &data[i], 8);
    memcpy(dat2, dat1, 8);
    memcpy(key, desKey, 16);
    Cryptoworks3DES(dat1, key);       
    for(j=0; j<8; j++) dat1[j] ^= t[j];	    
    memcpy(&data[i], dat1, 8);
    memcpy(t, dat2, 8);
  }   
  return data[10] + 5;
} 

void CryptoworksSignature(const unsigned char *data, unsigned int length, unsigned char *key, unsigned char *signature)
{
  unsigned int i, sigPos;
  char algo, first;

  algo = data[0] & 7;
  if(algo == 7) algo = 6;   
  memset(signature, 0, 8);
  first = 1;
  sigPos = 0;
  for(i=0; i<length; i++)
  {
    signature[sigPos] ^= data[i];
    sigPos++;
    
    if(sigPos > 7) {    
      if (first) {
        CW_L2DES(signature, key, algo);
      }      
      CW_DES(signature, key, 1);
      
      sigPos = 0;
      first = 0;
    }
  } 
  if(sigPos > 0) CW_DES(signature, key, 1);
  CW_R2DES(signature, key, algo);
}

void CryptoworksDecryptDes(unsigned char *data, unsigned char algo, unsigned char *key)
{
  int i;
  unsigned char k[22], t[8];
    
  algo &= 7;
  if(algo<7) CW_DEC_ENC(data, key, algo, DES_RIGHT);
  else {
    memcpy(k, key, 22);
    for(i=0; i<3; i++) {
      CW_DEC_ENC(data, k, algo, i&1);
      memcpy(t,k,8); 
      memcpy(k,k+8,8);
      memcpy(k+8,t,8);
    }
  }
}

char CryptoworksECM(uint32_t CAID, unsigned char *ecm, unsigned char *cw)
{
  unsigned int ident;
  unsigned char keyIndex, nanoLength, newEcmLength, key[22], signature[8], nano80Mode = 0;
  int provider = -1;
  uint16_t i, j, ecmLen = (((ecm[1] & 0x0f)<< 8) | ecm[2])+3;  
  memset(key, 0, 22);
  
  if(ecm[7] != ecmLen - 8) return 1;

  for(i = 8; i < ecmLen; i += ecm[i+1] + 2) {
    if(ecm[i] == 0x83) {
      provider = ecm[i+2] & 0xFC;
      keyIndex = ecm[i+2] & 3;
      keyIndex = keyIndex ? 1 : 0;
    }
    else if(ecm[i] == 0x84) {
      nano80Mode = ecm[i+3]; 
    }
  }
  if(provider < 0) return 1;

  ident = (CAID << 8) | provider; 
  if(!GetCwKey(key, ident, keyIndex, 16)) return 2;
  if(!GetCwKey(&key[16], ident, 6, 6)) return 2;
  
  for(i = 8; i < ecmLen; i += ecm[i+1] + 2) {
    if(ecm[i] == 0x80 && (provider == 0xA0 || provider == 0xC0 || provider == 0xC4 || provider == 0xC8)) {
      if(nano80Mode != 0x01) return 3; 
      nanoLength = ecm[i+1];
      newEcmLength = CryptoworksProcessNano80(ecm+i+2, provider, key, nanoLength);
      ecm[i+2+3] = 0x81;
      ecm[i+2+4] = 0x70;
      ecm[i+2+5] = newEcmLength;
      ecm[i+2+6] = 0x81;
      ecm[i+2+7] = 0xFF;
      return CryptoworksECM(CAID, ecm+i+2+3, cw);
    }
  }
 
  CryptoworksSignature(ecm + 5, ecmLen - 15, key, signature);
   for(i=8; i<ecmLen; i+=ecm[i+1]+2) {
     switch(ecm[i]) {
       case 0xDA: case 0xDB: case 0xDC:
         for(j=0; j<ecm[i+1]; j+=8)
           CryptoworksDecryptDes(&ecm[i+2+j], ecm[5], key);
         break;
       case 0xDF:
         if(memcmp(&ecm[i+2], signature, 8)) {
           return 6;
         }
         break;
      }
   }
  
  for(i=8; i<ecmLen; i+=ecm[i+1]+2) {
    switch(ecm[i]) {
      case 0xDB:
        if(ecm[i+1]==16) {
          memcpy(cw, &ecm[i+2], 16);
          return 0;
          }
        break;
      }
    }  
    
  return 5; 
}

// SoftNDS EMU
const unsigned char nds_const[]={0x0F,0x1E,0x2D,0x3C,0x4B,0x5A,0x69,0x78,0x87,0x96,0xA5,0xB4,0xC3,0xD2,0xE1,0xF0};

const unsigned char viasat_const[]=
{
  0x15,0x85,0xC5,0xE4,0xB8,0x52,0xEC,0xF7,0xC3,0xD9,0x08,0xBA,0x22,0x4A,0x66,0xF2,
  0x82,0x15,0x4F,0xB2,0x18,0x48,0x63,0x97,0xDC,0x19,0xD8,0x51,0x9A,0x39,0xFC,0xCA,
  0x1C,0x24,0xD0,0x65,0xA9,0x66,0x2D,0xD6,0x53,0x3B,0x86,0xBA,0x40,0xEA,0x4C,0x6D,
  0xD9,0x1E,0x41,0x14,0xFE,0x15,0xAF,0xC3,0x18,0xC5,0xF8,0xA7,0xA8,0x01,0x00,0x01,
};

char SoftNDSECM(unsigned char *ecm, unsigned char *dw)
{
  int i;
  unsigned char *tDW;
  unsigned char digest[16];
  MD5_CTX mdContext;
  memset(dw,0,16);
  tDW = &dw[ecm[0]==0x81 ? 8 : 0];
  if (ecm[6]!=0x21) return 1;
  MD5_Init (&mdContext);
  MD5_Update (&mdContext, ecm+7, 10);
  MD5_Update (&mdContext, ecm+0x20, 4);
  MD5_Update (&mdContext, viasat_const, 0x40);
  MD5_Update (&mdContext, nds_const, 0x10);
  MD5_Final (digest, &mdContext);
  for (i=0; i<8; i++) tDW[i] = digest[i+8] ^ ecm[0x17+i];
  if(((tDW[0]+tDW[1]+tDW[2])&0xFF)-tDW[3]) return 6;
  if(((tDW[4]+tDW[5]+tDW[6])&0xFF)-tDW[7]) return 6;
  return 0;
}

// Viaccess EMU
char GetViaKey(unsigned char *buf, unsigned int ident, char keyName, unsigned int keyIndex, unsigned int keyLength) {
  
  char keyStr[8];
  snprintf(keyStr, 8, "%c%X", keyName, keyIndex);  
  if(FindKey('V', ident, keyStr, buf, keyLength))
   return 1;    
  
  return 0;
}

void Via1Mod(const unsigned char* key2, unsigned char* data)
{
  int kb, db;
  for (db=7; db>=0; db--) {
    for (kb=7; kb>3; kb--) {
      int a0=kb^db;
      int pos=7;
      if (a0&4) {
        a0^=7;
        pos^=7;
      }
      a0=(a0^(kb&3)) + (kb&3);
      if (!(a0&4))
        data[db]^=(key2[kb] ^ ((data[kb^pos]*key2[kb^4]) & 0xFF));
    }
  }
  for (db=0; db<8; db++) {
    for (kb=0; kb<4; kb++) {
      int a0=kb^db;
      int pos=7;
      if (a0&4) {
        a0^=7;
        pos^=7;
      }
      a0=(a0^(kb&3)) + (kb&3);
      if (!(a0&4))
        data[db]^=(key2[kb] ^ ((data[kb^pos]*key2[kb^4]) & 0xFF));
    }
  }
}

void Via1Decode(unsigned char *data, unsigned char *key)
{
  Via1Mod(key+8, data);
  des(key, DES_ECM_CRYPT, data);
  Via1Mod(key+8, data);
}

void Via1Hash(unsigned char *data, unsigned char *key)
{
  Via1Mod(key+8, data);
  des(key, DES_ECM_HASH, data);
  Via1Mod(key+8, data);
}

char Via1Decrypt(unsigned char* source, unsigned char* dw, unsigned int ident, unsigned char desKeyIndex)
{
    unsigned char work_key[16];
    unsigned char *data, *des_data1, *des_data2;
    int len;
    int msg_pos;
    int encStart = 0, hash_start, i;
    unsigned char signature[8];
    unsigned char hashbuffer[8], prepared_key[16], tmp, k, hashkey[16], pH;

    if (ident == 0) return 4;
    memset(work_key, 0, 16);
    if(!GetViaKey(work_key, ident, '0', desKeyIndex, 8)) return 2;

    data = source+9;
    len = source[2]-6;
    des_data1 = dw;
    des_data2 = dw+8;
 
    msg_pos = 0;
    pH = 0; memset(hashbuffer, 0, sizeof(hashbuffer));
    memcpy(hashkey, work_key, sizeof(hashkey));
    memset(signature, 0, 8);
    
    while (msg_pos<len) {
        switch (data[msg_pos]) {
        case 0xea:
            encStart = msg_pos + 2;
            memcpy(des_data1,&data[msg_pos+2],8);
            memcpy(des_data2,&data[msg_pos+2+8],8);
            break;
        case 0xf0:
            memcpy(signature,&data[msg_pos+2],8);
            break;
        }
        msg_pos += data[msg_pos+1]+2;
    }
    pH=i=0;
    if (data[0] == 0x9f) {
        { hashbuffer[pH] ^= data[i++]; pH++; if(pH == 8) { Via1Hash(hashbuffer, hashkey); pH = 0; } }
        { hashbuffer[pH] ^= data[i++]; pH++; if(pH == 8) { Via1Hash(hashbuffer, hashkey); pH = 0; } }
        for (hash_start=0; hash_start < data[1];hash_start++) 
          { hashbuffer[pH] ^= data[i++]; pH++; if(pH == 8) { Via1Hash(hashbuffer, hashkey); pH = 0; } }
        while (pH != 0) { hashbuffer[pH] ^= (unsigned char) 0; pH++; if(pH == 8) 
         { Via1Hash(hashbuffer, hashkey); pH = 0; } }
    }   
    if (work_key[7] == 0) {
        for (; i < encStart + 16; i++) 
          { hashbuffer[pH] ^= data[i]; pH++; if(pH == 8) { Via1Hash(hashbuffer, hashkey); pH = 0; } }
        memcpy(prepared_key, work_key, 8);
    }
    else {
        prepared_key[0] = work_key[2];
        prepared_key[1] = work_key[3];
        prepared_key[2] = work_key[4];
        prepared_key[3] = work_key[5];
        prepared_key[4] = work_key[6];
        prepared_key[5] = work_key[0];
        prepared_key[6] = work_key[1];
        prepared_key[7] = work_key[7];
        memcpy(prepared_key+8,work_key+8,8);

        if (work_key[7] & 1) {
            for (; i < encStart; i++) 
              { hashbuffer[pH] ^= data[i]; pH++; if(pH == 8) { Via1Hash(hashbuffer, hashkey); pH = 0; } }
            k = ((work_key[7] & 0xf0) == 0) ? 0x5a : 0xa5;
            for (i=0; i<8; i++) {
                tmp = des_data1[i];
                des_data1[i] = (k & hashbuffer[pH] ) ^ tmp;
                { hashbuffer[pH] ^= tmp; pH++; if(pH == 8) { Via1Hash(hashbuffer, hashkey); pH = 0; } };
            }
            for (i = 0; i < 8; i++) {
                tmp = des_data2[i];
                des_data2[i] = (k & hashbuffer[pH] ) ^ tmp;
                { hashbuffer[pH] ^= tmp; pH++; if(pH == 8) { Via1Hash(hashbuffer, hashkey); pH = 0; } };
            }
        }
        else {
            for (; i < encStart + 16; i++)
              { hashbuffer[pH] ^= data[i]; pH++; if(pH == 8) { Via1Hash(hashbuffer, hashkey); pH = 0; } };
        }
    }
    Via1Decode(des_data1, prepared_key);
    Via1Decode(des_data2, prepared_key);
    Via1Hash(hashbuffer, hashkey);
    if(memcmp(signature, hashbuffer, 8)) return 6;
    return 0;
}

char Via26ProcessDw(unsigned char *indata, unsigned int ident, unsigned char desKeyIndex)
{
  unsigned char pv1,pv2, i;
  unsigned char Tmp[8], tmpKey[16], T1Key[300], P1Key[8], KeyDes1[16], KeyDes2[16], XorKey[8];
  
  if(!GetViaKey(T1Key, ident, 'T', 1, 300)) return 2;
  if(!GetViaKey(P1Key, ident, 'P', 1, 8)) return 2;
  if(!GetViaKey(KeyDes1, ident, 'D', 1, 16)) return 2;
  if(!GetViaKey(KeyDes2, ident, '0', desKeyIndex, 16)) return 2;  
  if(!GetViaKey(XorKey, ident, 'X', 1, 8)) return 2;

  for (i=0;i<8;i++){
    pv1 = indata[i];
    Tmp[i] = T1Key[pv1];
  }
  for (i=0;i<8;i++) {
    pv1 = P1Key[i];
    pv2 = Tmp[pv1];
    indata[i]=pv2;
  }
  memcpy(tmpKey, KeyDes1,8);
  doPC1(tmpKey) ;  
  des(tmpKey, DES_ECS2_CRYPT,indata);
  for (i=0;i<8;i++) indata[i] ^= XorKey[i];
  memcpy(tmpKey,KeyDes2,16);
  doPC1(tmpKey);
  doPC1(tmpKey+8);
  des(tmpKey,DES_ECS2_DECRYPT,indata);
  des(tmpKey+8, DES_ECS2_CRYPT,indata);
  des(tmpKey,DES_ECS2_DECRYPT,indata);
  for (i=0;i<8;i++) indata[i] ^= XorKey[i];
  memcpy(tmpKey, KeyDes1,8);
  doPC1(tmpKey);
  des(tmpKey,DES_ECS2_DECRYPT,indata);
    
  for (i=0;i<8;i++) {
    pv1 = indata[i];
    pv2 = P1Key[i];
    Tmp[pv2] = pv1;
  }
  for (i=0;i<8;i++) {
    pv1 =  Tmp[i];
    pv2 =  T1Key[pv1];
    indata[i] = pv2;
  }
  return 0;
}

char Via26Decrypt(unsigned char* source, unsigned char* dw, unsigned int ident, unsigned char desKeyIndex)
{
  unsigned char tmpData[8], C1[8];
  unsigned char *pXorVector;
  int i,j;
  
  if (ident == 0) return 4;
  if(!GetViaKey(C1, ident, 'C', 1, 8)) return 2;
        
  for (i=0; i<2; i++)
  {
    memcpy(tmpData, source+ i*8, 8);
    Via26ProcessDw(tmpData, ident, desKeyIndex);
    if (i!=0) pXorVector = source;
    else pXorVector = &C1[0];  
    for (j=0;j<8;j++) dw[i*8+j] = tmpData[j]^pXorVector[j];
  }  
  return 0;
}

void Via3Core(unsigned char *data, unsigned char Off, unsigned int ident, unsigned char* XorKey, unsigned char* T1Key)
{
  unsigned char i;
  unsigned long R2, R3, R4, R6, R7;

  switch (ident) {
  case 0x032820: {
    for (i=0; i<4; i++) data[i]^= XorKey[(Off+i) & 0x07];
    R2 = (data[0]^0xBD)+data[0];
    R3 = (data[3]^0xEB)+data[3];
    R2 = (R2-R3)^data[2];
    R3 = ((0x39*data[1])<<2);
    data[4] = (R2|R3)+data[2];
    R3 = ((((data[0]+6)^data[0]) | (data[2]<<1))^0x65)+data[0];
    R2 = (data[1]^0xED)+data[1];
    R7 = ((data[3]+0x29)^data[3])*R2;
    data[5] = R7+R3;  
    R2 = ((data[2]^0x33)+data[2]) & 0x0A;
    R3 = (data[0]+0xAD)^data[0];
    R3 = R3+R2;
    R2 = data[3]*data[3];
    R7 = (R2 | 1) + data[1];
    data[6] = (R3|R7)+data[1];
    R3 = data[1] & 0x07;
    R2 = (R3-data[2]) & (data[0] | R2 |0x01);
    data[7] = R2+data[3];
    for (i=0;i<4;i++) data[i+4] = T1Key[data[i+4]];
  }
  break;
  case 0x030B00: {
    for (i=0; i<4; i++) data[i]^= XorKey[(Off+i) & 0x07];
    R6 = (data[3] + 0x6E) ^ data[3];
    R6 = (R6*(data[2] << 1)) + 0x17;
    R3 = (data[1] + 0x77) ^ data[1];
    R4 = (data[0] + 0xD7) ^ data[0];
    data[4] = ((R4 & R3) | R6) + data[0];
    R4 = ((data[3] + 0x71) ^ data[3]) ^ 0x90;
    R6 = (data[1] + 0x1B) ^ data[1];
    R4 = (R4*R6) ^ data[0];
    data[5] = (R4 ^ (data[2] << 1)) + data[1];  
    R3 = (data[3] * data[3])| 0x01;
    R4 = (((data[2] ^ 0x35) + data[2]) | R3) + data[2];
    R6 = data[1] ^ (data[0] + 0x4A);
    data[6] = R6 + R4;  
    R3 = (data[0] * (data[2] << 1)) | data[1];
    R4 = 0xFE - data[3];
    R3 = R4 ^ R3;
    data[7] = R3 + data[3];  
    for (i=0; i<4; i++) data[4+i] = T1Key[data[4+i]];
  }
  break;
  default:
      break;
  }
}

void Via3Fct1(unsigned char *data, unsigned int ident, unsigned char* XorKey, unsigned char* T1Key)
{
  unsigned char t;
  Via3Core(data, 0, ident, XorKey, T1Key);
  
  switch (ident) {
  case 0x032820: {
    t = data[4];
    data[4] = data[7];
    data[7] = t;
  }
  break;
  case 0x030B00: {
    t = data[5];
    data[5] = data[7];
    data[7] = t;
  }
  break;
  default:
    break;
  }
}

void Via3Fct2(unsigned char *data, unsigned int ident, unsigned char* XorKey, unsigned char* T1Key)
{
  unsigned char t;
  Via3Core(data, 4, ident, XorKey, T1Key);
  
  switch (ident) {
  case 0x032820: {
    t = data[4];
    data[4] = data[7];
    data[7] = data[5];
    data[5] = data[6];
    data[6] = t;
  }
  break;
  case 0x030B00: {
    t = data[6];
    data[6] = data[7];
    data[7] = t;
  }
  break;
  default:
    break;
  }
}

char Via3ProcessDw(unsigned char *data, unsigned int ident, unsigned char desKeyIndex)
{
  unsigned char i;
  unsigned char tmp[8], tmpKey[16], T1Key[300], P1Key[8], KeyDes[16], XorKey[8];

  if(!GetViaKey(T1Key, ident, 'T', 1, 300)) return 2;
  if(!GetViaKey(P1Key, ident, 'P', 1, 8)) return 2;
  if(!GetViaKey(KeyDes, ident, '0', desKeyIndex, 16)) return 2;  
  if(!GetViaKey(XorKey, ident, 'X', 1, 8)) return 2;

  for (i=0; i<4; i++) tmp[i] = data[i+4];
  Via3Fct1(tmp, ident, XorKey, T1Key);
  for (i=0; i<4; i++) tmp[i] = data[i]^tmp[i+4];
  Via3Fct2(tmp, ident, XorKey, T1Key);
  for (i=0; i<4; i++) tmp[i]^= XorKey[i+4];
  for (i=0; i<4; i++) {
    data[i] = data[i+4]^tmp[i+4];
    data[i+4] = tmp[i];
  }
  memcpy(tmpKey,KeyDes,16);
  doPC1(tmpKey);
  doPC1(tmpKey+8);
  des(tmpKey, DES_ECS2_DECRYPT, data);
  des(tmpKey+8, DES_ECS2_CRYPT, data);
  des(tmpKey, DES_ECS2_DECRYPT, data);
  for (i=0; i<4; i++) tmp[i] = data[i+4];
  Via3Fct2(tmp, ident, XorKey, T1Key);
  for (i=0; i<4; i++) tmp[i] = data[i]^tmp[i+4];
  Via3Fct1(tmp, ident, XorKey, T1Key);
  for (i=0; i<4; i++) tmp[i]^= XorKey[i];
  for (i=0; i<4; i++) {
    data[i] = data[i+4]^tmp[i+4];
    data[i+4] = tmp[i];
  }
  return 0;
}

void Via3FinalMix(unsigned char *dw)
{
  unsigned int tmp; 

  tmp = *(unsigned int *)dw;
  *(unsigned int *)dw = *(unsigned int *)(dw + 4);
  *(unsigned int *)(dw + 4) = tmp;
  
  tmp = *(unsigned int *)(dw + 8);
  *(unsigned int *)(dw + 8) = *(unsigned int *)(dw + 12);
  *(unsigned int *)(dw + 12) = tmp;
}

char Via3HasValidCRC(unsigned char *dw)
{
  if (((dw[0]+dw[1]+dw[2]) & 0xFF) != dw[3]) return 0;
  if (((dw[4]+dw[5]+dw[6]) & 0xFF) != dw[7]) return 0;
  if (((dw[8]+dw[9]+dw[10]) & 0xFF) != dw[11]) return 0;
  if (((dw[12]+dw[13]+dw[14]) & 0xFF) != dw[15]) return 0;
  return 1;
}

char Via3Decrypt(unsigned char* source, unsigned char* dw, unsigned int ident, unsigned char desKeyIndex, unsigned char aesKeyIndex, unsigned char aesMode, char doFinalMix)
{
  char aesAfterCore = 0;
  char needsAES = (aesKeyIndex != 0xFF);
  unsigned char tmpData[8], C1[8];
  unsigned char *pXorVector;
  char aesKey[16];
  int i, j;
    
  if(ident == 0) return 4;
  if(!GetViaKey(C1, ident, 'C', 1, 8)) return 2;    
  if(needsAES && !GetViaKey((unsigned char*)aesKey, ident, 'E', aesKeyIndex, 16)) return 2;
  if(aesMode==0x0D || aesMode==0x11 || aesMode==0x15) aesAfterCore = 1;

  if(needsAES && !aesAfterCore) {
    if(aesMode == 0x0F) {
      hdSurEncPhase1_D2_0F_11(source);
      hdSurEncPhase2_D2_0F_11(source);
    }
    else if(aesMode == 0x13) {
      hdSurEncPhase1_D2_13_15(source);
    }
    struct aes_keys aes;
    aes_set_key(&aes, aesKey);
    aes_decrypt(&aes, source, 16);
    if(aesMode == 0x0F) {
      hdSurEncPhase1_D2_0F_11(source);
    }
    else if(aesMode == 0x13) {
      hdSurEncPhase2_D2_13_15(source);
    }
  }
  
  for(i=0; i<2; i++) {
    memcpy(tmpData, source+i*8, 8);
    Via3ProcessDw(tmpData, ident, desKeyIndex);
    if (i!=0) pXorVector = source;
    else pXorVector = &C1[0];      
    for (j=0;j<8;j++) dw[i*8+j] = tmpData[j]^pXorVector[j];
  }

  if(needsAES && aesAfterCore){
    if(aesMode == 0x11) {
      hdSurEncPhase1_D2_0F_11(dw);
      hdSurEncPhase2_D2_0F_11(dw);
    }
    else if(aesMode == 0x15) {
      hdSurEncPhase1_D2_13_15(dw);
    }
    struct aes_keys aes;
    aes_set_key(&aes, aesKey);
    aes_decrypt(&aes, dw, 16);
    if(aesMode == 0x11) {
      hdSurEncPhase1_D2_0F_11(dw);
    }
    if(aesMode == 0x15) {
      hdSurEncPhase2_D2_13_15(dw);
    }
  }

  if(ident == 0x030B00) {
    if(doFinalMix) Via3FinalMix(dw);
    if(!Via3HasValidCRC(dw)) return 6;
  }
  return 0;
}

char ViaccessECM(unsigned char *ecm, unsigned char *dw)
{
  unsigned int currentIdent = 0;
  unsigned char nanoCmd = 0, nanoLen = 0, version = 0, providerKeyLen = 0, desKeyIndex = 0, aesMode = 0, aesKeyIndex = 0xFF;
  char doFinalMix = 0;
  uint16_t i = 0, keySelectPos = 0, ecmLen = (((ecm[1] & 0x0f)<< 8) | ecm[2])+3;
  
  for (i=4; i<ecmLen; )
  {
    nanoCmd = ecm[i++];
    nanoLen = ecm[i++];
      
    switch (nanoCmd) {
      case 0x40:
        if (nanoLen < 0x03) break;
        version = ecm[i];
        if (nanoLen == 3){
           currentIdent=((ecm[i]<<16)|(ecm[i+1]<<8))|(ecm[i+2]&0xF0);
          desKeyIndex = ecm[i+2]&0x0F;
          keySelectPos = i+3;
        }
        else {
          currentIdent =(ecm[i]<<16)|(ecm[i+1]<<8)|((ecm[i+2]>>4)&0x0F);
          desKeyIndex = ecm[i+3];
          keySelectPos = i+4;
        }
        providerKeyLen = nanoLen;
        break;
      case 0x90:
        if (nanoLen < 0x03) break;
        version = ecm[i];
        currentIdent= ((ecm[i]<<16)|(ecm[i+1]<<8))|(ecm[i+2]&0xF0);
        desKeyIndex = ecm[i+2]&0x0F;
        keySelectPos = i+4;    
        if((version == 3) && (nanoLen > 3)) desKeyIndex = ecm[i+(nanoLen-4)]&0x0F;
        providerKeyLen = nanoLen;
        break;
      case 0x80:
        nanoLen = 0;
        break;
      case 0xD2:
        if (nanoLen < 0x02) break;
        aesMode = ecm[i];          
        aesKeyIndex = ecm[i+1];
        break;
      case 0xDD:
        nanoLen = 0;
        break;
      case 0xEA:
        if (nanoLen < 0x10) break;
        if (version < 2) return Via1Decrypt(ecm, dw, currentIdent, desKeyIndex); else
        if (version == 2) return Via26Decrypt(ecm + i, dw, currentIdent, desKeyIndex); else
        if (version == 3) {
          doFinalMix = 0;
          if (currentIdent == 0x030B00 && providerKeyLen>3) {
            if (ecm[keySelectPos]==0x05 && ecm[keySelectPos+1]==0x67 && (ecm[keySelectPos+2]==0x00 || ecm[keySelectPos+2]==0x01)) {
              if(ecm[keySelectPos+2]==0x01) doFinalMix = 1;
            }
            else break;
          }
          return Via3Decrypt(ecm + i, dw, currentIdent, desKeyIndex, aesKeyIndex, aesMode, doFinalMix);
        }
        break;
      default:
        break;
    }
    i += nanoLen;
  }
  return 5;
}

// Nagra EMU
char GetNagraKey(unsigned char *buf, unsigned int ident, char keyName, unsigned int keyIndex)
{
  char keyStr[8];
  snprintf(keyStr, 8, "%c%X", keyName, keyIndex);	
  if(FindKey('N', ident, keyStr, buf, keyName == 'M' ? 64 : 16))
   return 1;	  

  return 0;
}

char Nagra2Signature(const unsigned char *vkey, const unsigned char *sig, const unsigned char *msg, int len)
{
  unsigned char buff[16];    
  unsigned char iv[8];
  int i,j;
  
  memcpy(buff,vkey,sizeof(buff));
  for(i=0; i<len; i+=8) {
    IDEA_KEY_SCHEDULE ek;
    idea_set_encrypt_key(buff, &ek);
    memcpy(buff,buff+8,8);  
    memset(iv,0,sizeof(iv));
    idea_cbc_encrypt(msg+i,buff+8,8,&ek,iv,IDEA_ENCRYPT);   
    for(j=7; j>=0; j--) buff[j+8]^=msg[i+j];
  }
  buff[8]&=0x7F;
  return (memcmp(sig,buff+8,8)==0);
}

char DecryptNagra2ECM(unsigned char *in, unsigned char *out, const unsigned char *key, int len, const unsigned char *vkey, unsigned char *keyM)
{
  BIGNUM *exp, *mod;
  unsigned char iv[8];
  int i = 0, sign = in[0] & 0x80;
  unsigned char binExp = 3;
  char result = 1;
  
  exp = BN_new(); 
  mod = BN_new();
  BN_bin2bn(&binExp, 1, exp); 
  BN_bin2bn(keyM, 64, mod);
     
  if(EmuRSA(out,in+1,64,exp,mod,1)<=0) { BN_free(exp); BN_free(mod); return 0; }
  out[63]|=sign;
  if(len>64) memcpy(out+64,in+65,len-64);

  if(in[0]&0x04) {
    unsigned char tmp[8];
    DES_key_schedule ks1, ks2;
    ReverseMemInOut(tmp,&key[0],8);
    DES_key_sched((DES_cblock *)tmp,&ks1);
    ReverseMemInOut(tmp,&key[8],8);
    DES_key_sched((DES_cblock *)tmp,&ks2);
    memset(tmp,0,sizeof(tmp));
    for(i=7; i>=0; i--) ReverseMem(out+8*i,8);
    DES_ede2_cbc_encrypt(out,out,len,&ks1,&ks2,(DES_cblock *)tmp,DES_DECRYPT);
    for(i=7; i>=0; i--) ReverseMem(out+8*i,8);
  }
  else  {
    memset(iv,0,sizeof(iv));        
    IDEA_KEY_SCHEDULE ek;
    idea_set_encrypt_key(key, &ek);
    idea_cbc_encrypt(out, out, len&~7, &ek, iv, IDEA_DECRYPT);
  }

  ReverseMem(out,64);
  if(result && EmuRSA(out,out,64,exp,mod,0)<=0) result = 0;
  if(result && vkey && !Nagra2Signature(vkey,out,out+8,len-8)) result = 0;
    
  BN_free(exp); BN_free(mod);     
  return result;
}

char Nagra2ECM(unsigned char *ecm, unsigned char *dw)
{
  unsigned int ident, identMask, tmp1, tmp2, tmp3;
  unsigned char cmdLen, ideaKeyNr, dec[256], ideaKey[16], vKey[16], m1Key[64], mecmAlgo = 0;
  char useVerifyKey = 0;
  int l=0, s;
  uint16_t i = 0, ecmLen = (((ecm[1] & 0x0f)<< 8) | ecm[2])+3;
  
  cmdLen = ecm[4] - 5;  
  ident = (ecm[5] << 8) + ecm[6];  
  ideaKeyNr = (ecm[7]&0x10)>>4;
  if(ideaKeyNr) ideaKeyNr = 1;
  if(ident == 1283 || ident == 1285 || ident == 1297) ident = 1281;
  if(cmdLen <= 63 || ecmLen < cmdLen + 10) return 1;
  
  if(!GetNagraKey(ideaKey, ident, '0', ideaKeyNr)) return 2;    
  if(GetNagraKey(vKey, ident, 'V', 0)) { useVerifyKey = 1; }
  if(!GetNagraKey(m1Key, ident, 'M', 1)) return 2;    
  ReverseMem(m1Key, 64);
  
  if(!DecryptNagra2ECM(ecm+9, dec, ideaKey, cmdLen, useVerifyKey?vKey:0, m1Key)) return 1;    

  for(i=(dec[14]&0x10)?16:20; i<cmdLen-10 && l!=3; ) {
    switch(dec[i]) {
      case 0x10: case 0x11:
        if(dec[i+1]==0x09) {
          s = (~dec[i])&1;
          mecmAlgo = dec[i+2]&0x60;
          memcpy(dw+(s<<3),&dec[i+3],8);
          i+=11; l|=(s+1);
        } else i++;
        break;
      case 0x00: 
        i+=2; break;
      case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0xB0:
        i+=dec[i+1]+2; break;
      default:
        i++; continue;
    }
  }
  
  if(l!=3) return 1;
  if(mecmAlgo>0) return 1;
   
  identMask = ident & 0xFF00;
  if (identMask == 0x1100 || identMask == 0x500 || identMask == 0x3100) {   
    tmp1 = *(unsigned int*)dw;
    tmp2 = *(unsigned int*)(dw + 4);
    tmp3 = *(unsigned int*)(dw + 12);  
    *(unsigned int*)dw = *(unsigned int*)(dw + 8);
    *(unsigned int*)(dw + 4) = tmp3;
    *(unsigned int*)(dw + 8) = tmp1;
    *(unsigned int*)(dw + 12) = tmp2;
  }
  return 0;
}

// Irdeto EMU
char GetIrdetoKey(unsigned char *buf, unsigned int ident, char keyName, unsigned int keyIndex)
{
  char keyStr[8];
  snprintf(keyStr, 8, "%c%X", keyName, keyIndex);	
  if(FindKey('I', ident, keyStr, buf, 16))
   return 1;

  return 0;
}

static inline void xxor(unsigned char *data, int len, const unsigned char *v1, const unsigned char *v2)
{
  switch(len) {
    case 16:
      *((unsigned int *)data+3) = *((unsigned int *)v1+3) ^ *((unsigned int *)v2+3);
      *((unsigned int *)data+2) = *((unsigned int *)v1+2) ^ *((unsigned int *)v2+2);
    case 8:
      *((unsigned int *)data+1) = *((unsigned int *)v1+1) ^ *((unsigned int *)v2+1);
    case 4:
      *((unsigned int *)data+0) = *((unsigned int *)v1+0) ^ *((unsigned int *)v2+0);
      break;
    default:
      while(len--) *data++ = *v1++ ^ *v2++;
      break;
    }
}

void Irdeto2Encrypt(unsigned char *data, const unsigned char *seed, const unsigned char *okey, int len)
{
  const unsigned char *tmp = seed; 
  unsigned char key[16];
  int i; 
  
  memcpy(key, okey, 16);
  doPC1(key);
  doPC1(&key[8]);
  len&=~7;
   
  for(i=0; i<len; i+=8) {
    xxor(&data[i],8,&data[i],tmp); 
    tmp=&data[i];
    des(key,DES_ECS2_CRYPT,&data[i]);
    des(&key[8],DES_ECS2_DECRYPT,&data[i]);
    des(key,DES_ECS2_CRYPT,&data[i]);    
  }
}

void Irdeto2Decrypt(unsigned char *data, const unsigned char *seed, const unsigned char *okey, int len)
{
  unsigned char buf[2][8];
  unsigned char key[16];
  int i, n=0;

  memcpy(key, okey, 16);
  doPC1(key);
  doPC1(&key[8]);
  len&=~7;

  memcpy(buf[n],seed,8);
  for(i=0; i<len; i+=8,data+=8,n^=1) {
    memcpy(buf[1-n],data,8);
    des(key,DES_ECS2_DECRYPT,data);
    des(&key[8],DES_ECS2_CRYPT,data);
    des(key,DES_ECS2_DECRYPT,data);    
    xxor(data,8,data,buf[n]);
  }
}

char Irdeto2CalculateHash(const unsigned char *okey, const unsigned char *iv, const unsigned char *data, int len)
{
  unsigned char cbuff[8];
  unsigned char key[16];
  int l, y;
  
  memcpy(key, okey, 16);
  doPC1(key);
  doPC1(&key[8]);

  memset(cbuff,0,sizeof(cbuff));
  len-=8;
  
  for(y=0; y<len; y+=8) {
    if(y<len-8) xxor(cbuff,8,cbuff,&data[y]);
    else {
      l=len-y;
      xxor(cbuff,l,cbuff,&data[y]);
      xxor(cbuff+l,8-l,cbuff+l,iv+8);
    }
    des(key,DES_ECS2_CRYPT,cbuff);
    des(&key[8],DES_ECS2_DECRYPT,cbuff);
    des(key,DES_ECS2_CRYPT,cbuff);
  }

  return memcmp(cbuff,&data[len],8)==0;
}

char Irdeto2ECM(uint16_t CAID, unsigned char *ecm, unsigned char *dw)
{
  unsigned char keyNr=0, length, end, key[16], keyM1[16], keyIV[16], tmp[16];
  unsigned int i, l, ident;

  uint16_t ecmLen = (((ecm[1] & 0x0f)<< 8) | ecm[2])+3;
  
  length = ecm[11]; 
  keyNr = ecm[9];
  ident = ecm[8] | CAID << 8;
    
  if(ecmLen < length+12) return 1;
  if(!GetIrdetoKey(key, ident, '0', keyNr)) return 2;
  if(!GetIrdetoKey(keyM1, ident, 'M', 1)) return 2;
  if(!GetIrdetoKey(keyIV, ident, 'M', 2)) return 2;
    
  memset(tmp, 0, 16);
  Irdeto2Encrypt(keyM1, tmp, key, 16);
  ecm+=12;
  Irdeto2Decrypt(ecm, keyIV, keyM1, length); 
  i=(ecm[0]&7)+1;
  end = length-8 < 0 ? 0 : length-8;
    
  while(i<end) {
    l = ecm[i+1] ? (ecm[i+1]&0x3F)+2 : 1;
    switch(ecm[i]) {
      case 0x10: case 0x50: 
        if(l==0x13 && i<=length-8-l) Irdeto2Decrypt(&ecm[i+3], keyIV, key, 16); 
      break;
      case 0x78: 
        if(l==0x14 && i<=length-8-l) Irdeto2Decrypt(&ecm[i+4], keyIV, key, 16); 
      break;
    }
    i+=l;
  } 
  
  i=(ecm[0]&7)+1;
  if(Irdeto2CalculateHash(keyM1, keyIV, ecm-6, length+6)) {
    while(i<end) {
      l = ecm[i+1] ? (ecm[i+1]&0x3F)+2 : 1;
      switch(ecm[i]) {
        case 0x78:
          memcpy(dw, &ecm[i+4], 16);
          return 0;
      }
      i+=l;
    }
  }
  return 1;
}


/* Error codes
0  OK
1  ECM not supported  
2  Key not found
3  Nano80 problem
4  Corrupt data
5  CW not found
6  CW checksum error
7  Out of memory
*/
char ProcessECM(uint16_t CAID, const unsigned char *ecm, unsigned char *dw) {
  char result;
  unsigned char *ecmCopy;
  uint16_t ecmLen = (((ecm[1] & 0x0f)<< 8) | ecm[2])+3;
  
  if(ecmLen > 1024) return 1;
  ecmCopy = (unsigned char*)malloc(ecmLen);
  if(ecmCopy == NULL) return 7;
  memcpy(ecmCopy, ecm, ecmLen);
  result = 1;
  
  if((CAID>>8)==0x0D)
    result = CryptoworksECM(CAID,ecmCopy,dw);
  else if(CAID==0x090F)
    result = SoftNDSECM(ecmCopy,dw);    
  else if(CAID==0x500)
    result = ViaccessECM(ecmCopy,dw);
  else if(CAID==0x1801)
    result = Nagra2ECM(ecmCopy,dw);
  else if(CAID==0x0604)
    result= Irdeto2ECM(CAID,ecmCopy,dw);
  
  free(ecmCopy);
  return result;
}
