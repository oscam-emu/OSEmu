#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "des.h"
#include "bn.h"
#include "idea.h"
#include "via3surenc.h"
#include "emulator.h"
#include "globals.h"
#include "helpfunctions.h"

// Cryptoworks EMU
static char crw_0D00C000[]={0x22,0xEC,0xB8,0xB2,0x43,0x85,0xC3,0xB2,0x94,0x1E,0xF7,0xEC,0xC2,0xB5,0x4A,0x09}; // DigiTurk
static char crw_0D00C001[]={0xC0,0xC6,0x86,0x55,0xB6,0x33,0x04,0x08,0x5B,0x5F,0x72,0x2D,0xD4,0x71,0xFE,0x27}; // DigiTurk
static char crw_0D00C006[]={0x01,0x56,0x12,0xE8,0xEE,0x33}; // DigiTurk
static char crw_0D00C400[]={0x22,0xEC,0xB8,0xB2,0x43,0x85,0xC3,0xB2,0x94,0x1E,0xF7,0xEC,0xC2,0xB5,0x4A,0x09}; // DigiTurk
static char crw_0D00C401[]={0xC0,0xC6,0x86,0x55,0xB6,0x33,0x04,0x08,0x5B,0x5F,0x72,0x2D,0xD4,0x71,0xFE,0x27}; // DigiTurk
static char crw_0D00C406[]={0x01,0x56,0x12,0xE8,0xEE,0x33}; // DigiTurk
static char crw_0D028C00[]={0x6E,0x75,0x6E,0x64,0x85,0x88,0x6E,0x9B,0x08,0x59,0xC4,0x03,0xEA,0xFB,0xD1,0x8A}; // UPC 0.8°W
static char crw_0D028C01[]={0xA9,0xE9,0x97,0x70,0xCC,0x04,0x50,0x48,0x05,0xA5,0x1F,0xEC,0x67,0x9D,0x03,0x28}; // UPC 0.8°W
static char crw_0D02A000[]={0x30,0x72,0x49,0xC2,0xE6,0xCA,0x1B,0xFC,0x69,0x8C,0x79,0xB4,0xC6,0x4E,0x81,0x0C}; // UPC 0.8°W
static char crw_0D02A006[]={0xC4,0xFF,0xB3,0x30,0x79,0x5B}; // UPC 0.8°W
static char crw_0D02A400[]={0x6B,0x9C,0x61,0xF2,0xAD,0xF7,0x6C,0x7F,0x23,0xCF,0x10,0xE2,0xE4,0xD5,0xF5,0xE2}; // UPC 0.8°W
static char crw_0D02A401[]={0x3E,0x55,0x24,0xFC,0x02,0x23,0x8F,0x1B,0xE1,0x5B,0x45,0xA9,0x6D,0x52,0xCC,0xF5}; // UPC 0.8°W
static char crw_0D02A800[]={0xC2,0x29,0xA4,0x03,0x61,0x00,0xE8,0x1A,0x42,0xBA,0xC3,0xC7,0x0E,0xBB,0xC8,0x52}; // UPC 0.8°W
static char crw_0D02A801[]={0xC2,0x29,0xA4,0x03,0x61,0x00,0xE8,0x1A,0x42,0xBA,0xC3,0xC7,0x0E,0xBB,0xC8,0x52}; // UPC 0.8°
static char crw_0D030400[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x19,0xE8,0xF5,0x24,0xC6,0xC0,0x53,0xE4}; // Sky Link 23.5E
static char crw_0D030406[]={0x27,0xB4,0x88,0x34,0x8D,0x54}; // Sky Link 23.5E
static char crw_0D030801[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x42,0xF0,0x7B,0xE9,0x20,0xC7,0xE0,0xF3}; // Sky Link 23.5E
static char crw_0D030806[]={0x27,0xB4,0x88,0x34,0x8D,0x54}; // Sky Link 23.5E
static char crw_0D050400[]={0xA9,0xD7,0x32,0xF5,0xE7,0x38,0xE4,0x8A,0x91,0xC8,0x63,0x8B,0x12,0x74,0x66,0x61}; // ORF
static char crw_0D050406[]={0x09,0x87,0xCF,0x2A,0x61,0x1D}; // ORF
static char crw_0D051000[]={0x00,0x00,0x6A,0xB3,0x25,0xE6,0xAB,0xF4,0xDA,0x47,0x28,0x0F,0x35,0x29,0x4B,0x5C}; // Austriasat
static char crw_0D051006[]={0x09,0x87,0xCF,0x2A,0x61,0x1D}; // ORF

char GetCwKey(unsigned char *buf, uint32_t CAID, unsigned char ident) {
	if ((CAID>>4)== 0xD02A) ident &=0xFE; else
	if ((CAID>>4)== 0xD00C) CAID = 0x0D00C0; else
	if (ident==6 && ((CAID>>8) == 0x0D05)) CAID = 0x0D0504;
	switch ((CAID << 8) + ident) {
		case 0x0D00C000 : memcpy(buf,crw_0D00C000,16);	return 1; // DigiTurk
		case 0x0D00C001 : memcpy(buf,crw_0D00C001,16);	return 1; // DigiTurk
		case 0x0D00C006 : memcpy(buf,crw_0D00C006,6);	return 1; // DigiTurk
		case 0x0D00C400 : memcpy(buf,crw_0D00C400,16);	return 1; // DigiTurk
		case 0x0D00C401 : memcpy(buf,crw_0D00C401,16);	return 1; // DigiTurk
		case 0x0D00C406 : memcpy(buf,crw_0D00C406,6);   return 1; // DigiTurk
		case 0x0D028C00 : memcpy(buf,crw_0D028C00,16);  return 1; // UPC 0.8°W
		case 0x0D028C01 : memcpy(buf,crw_0D028C01,16);  return 1; // UPC 0.8°W
		case 0x0D02A000 : memcpy(buf,crw_0D02A000,16);  return 1; // UPC 0.8°W
		case 0x0D02A006 : memcpy(buf,crw_0D02A006,6);	return 1; // UPC 0.8°W
		case 0x0D02A400 : memcpy(buf,crw_0D02A400,16);  return 1; // UPC 0.8°W
		case 0x0D02A401 : memcpy(buf,crw_0D02A401,16);  return 1; // UPC 0.8°W
		case 0x0D02A800 : memcpy(buf,crw_0D02A800,16);  return 1; // UPC 0.8°W
		case 0x0D02A801 : memcpy(buf,crw_0D02A801,16);  return 1; // UPC 0.8°W
		case 0x0D030400 : memcpy(buf,crw_0D030400,16);	return 1; // Sky Link 23.5E
		case 0x0D030406 : memcpy(buf,crw_0D030406,6);	return 1; // Sky Link 23.5E
		case 0x0D030801 : memcpy(buf,crw_0D030801,16);	return 1; // Sky Link 23.5E
		case 0x0D030806 : memcpy(buf,crw_0D030806,6);	return 1; // Sky Link 23.5E
		case 0x0D050400 : memcpy(buf,crw_0D050400,16);	return 1; // ORF
		case 0x0D050406 : memcpy(buf,crw_0D050406,6);	return 1; // ORF
		case 0x0D051000 : memcpy(buf,crw_0D051000,16);  return 1; // Austriasat
		case 0x0D051006 : memcpy(buf,crw_0D051006,6);   return 1; // Austriasat
	}
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
        // according to clang de following does nothing...
		// c=0;
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

unsigned char CW_PDUSDEC(unsigned char* d, int l)
{
    unsigned char i = 0, pi, li, buf[128];
    memset(buf,0,sizeof(buf));
    while ( i < l )
    {
        pi = d[i];
        li = d[i + 1];
        if (i == 0) li = 1;
        switch (pi)
        {
            case 0x86 :
            {
                memcpy(&d[i], &d[i + 2 + li],l-i-li+2);
                l -= (li +2);
                d[2] -= (li +2);
                continue;
            }
            case 0xdf : return l;
        }
        i += ( 2 + li );
    }
    return l;
}
void CW_PDUDEC(unsigned char* d, int l, unsigned char* k )
{
    unsigned char pi, li, i = 0, j,key[22], algo = d[0], buf[250];
    memcpy(key,k,22);
    memcpy(buf, d, l);
    memset(d,0,l);
    l = CW_PDUSDEC(buf,l);
    memcpy(d,buf,l);
    while ( i < l )
    {
        pi = d[i];
        li = d[i + 1];
        if (i == 0 && li == 0) li = 1;
        if(pi == 0xdf) return;
        if ( ( pi == 0xda ) || ( pi == 0xdb ) || ( pi == 0xdc ) )
        for ( j = 0; j < ( li / 8 ); j++ )
        CW_DEC(&d[i + 2 + ( j * 8 )], key, algo);
        i += ( 2 + li );
    }
    return;
}

char CW_DCW(unsigned char *d ,unsigned char l , unsigned char *dcw)
{
    unsigned char i = 0, pi, li;
    while ( i < l )
    {
        pi = d[i];
        li = d[i + 1];
        if (i == 0) li = 1;
        if ( pi == 0xdf ) return 5;
        if ( pi == 0xdb ) {
	    memcpy(dcw,&d[i+2], li);
	    if(dcw[3]==((dcw[0]+dcw[1]+dcw[2])&0xFF))
	     if(dcw[7]==((dcw[4]+dcw[5]+dcw[6])&0xFF))
	      if(dcw[11]==((dcw[8]+dcw[9]+dcw[10])&0xFF))
	       if(dcw[15]==((dcw[12]+dcw[13]+dcw[14])&0xFF))
		return 0;
	    return 6;
        }
        i += ( 2 + li );
    }
    return 4;
}

#define DES_LEFT	    0
#define DES_IP              1
#define DES_IP_1            2
#define DES_RIGHT           4
#define DES_HASH            8
#define DES_ECS2_DECRYPT    (DES_IP | DES_IP_1 | DES_RIGHT)
#define DES_ECS2_CRYPT      (DES_IP | DES_IP_1) 

void cryptoworks_3des(unsigned char *data, unsigned char *key) {
	doPC1(key);
	doPC1(&key[8]);
	des(key,DES_ECS2_DECRYPT,data);
	des(&key[8],DES_ECS2_CRYPT,data);
	des(key,DES_ECS2_DECRYPT,data);
}

char nano80(unsigned char *buf, unsigned char *key, unsigned char *ECM, unsigned char len) {
  unsigned char t[16],b[16],k[16],dat1[16],dat2[16],deskey[16],i,j;
  memset(t,0,16);
  memcpy(b,buf,8);
  memcpy(k,key,16);
  cryptoworks_3des(b,k);
  memcpy(deskey,b,8);
  memcpy(&k[8],key,8);
  memcpy(k,&key[8],8);
  memcpy(b,buf,8);
  cryptoworks_3des(b,k);
  memcpy(&deskey[8],b,8);
  for(i=8;i<len;i+=8) {
    memcpy(dat1,&buf[i],8);
    memcpy(dat2,dat1,8);
    memcpy(k,deskey,16);
    cryptoworks_3des(dat1,k);
    for(j=0;j<8;j++)
        dat1[j] ^= t[j];
    if(i==8)
	if (dat1[2]!=(len-11)) return 0;
    memcpy(&ECM[i-3],dat1,8);
    memcpy(t,dat2,8);
  }
  ECM[0] = 0x80; ECM[1] = 0x70; ECM[2] = len - 6; ECM[3] = 0x81; ECM[4] = 0xFF;
  return 1;
}

char CryptoworksDec(unsigned char *cw, unsigned char *ecm, uint32_t CAID)
{
    unsigned char key[22],prov=-1, can8060=0 ,i , len, keyid=-1;

    ecm += 5; len = ecm[2] + 3;

    for (i = 3; i < len;) {
	    if(ecm[i]==0x84){
		    can8060 = ecm[i + 3];
	            break;
	    } else if(ecm[i]==0x83) {
 	            prov = (ecm[i + 2]) & 0xFC;
        	    keyid = (ecm[i + 2]) & 3;
	    }
        i += ecm[i + 1] + 2;
    }
    if(!(GetCwKey(key+16,(CAID<<8) | (prov),0x06)))
        return -2;
    if(can8060==0x01 && ecm[3]==0x80)
    if((CAID==0x0D00 && (prov&0xF0)==0xC0) || (CAID==0x0D02 && (prov&0xF0)==0xA0)){
        if(!(GetCwKey(key,(CAID<<8) | prov, 1 - (keyid>0)))) return 2;
	if (!nano80(&ecm[5],key,ecm,ecm[4])) return 3;
	ecm += 5; len = ecm[2] + 3;
    }
    if(!(GetCwKey(key,(CAID<<8) | (prov),keyid))) return 2;
    CW_PDUDEC (ecm, len-10, key);
    return CW_DCW(ecm, len-10, cw);
}

char CryptoworksProcessECM(unsigned char *ecm, unsigned char *dw) {
	uint32_t caid;
	if (ecm[2]==0x99 && ecm[0x73]==0x0B && ecm[0x74]==0x05)
		caid = 0x0D03; else
	if (ecm[2]==0x99 && ecm[0x73]==0x08 && ecm[0x74]==0x00)
		caid = 0x0D05; else
	if (ecm[2]>=0x6C && (ecm[ecm[9]+12]&0xF0)==0xA0)
	        caid = 0x0D02; else
	if (ecm[2]>=0x6C && (ecm[ecm[9]+12]&0xF0)==0xC0)
		caid = 0x0D00; else
	return 1;
	return CryptoworksDec(dw,ecm,caid);
}

// SoftNDS EMU
const unsigned char nds_const[]={0x0F,0x1E,0x2D,0x3C,0x4B,0x5A,0x69,0x78,0x87,0x96,0xA5,0xB4,0xC3,0xD2,0xE1,0xF0};

const unsigned char viasat_key[]=
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
 	MD5_Update (&mdContext, viasat_key, 0x40);
 	MD5_Update (&mdContext, nds_const, 0x10);
 	MD5_Final (digest, &mdContext);
	for (i=0; i<8; i++) tDW[i] = digest[i+8] ^ ecm[0x17+i];
	if(((tDW[0]+tDW[1]+tDW[2])&0xFF)-tDW[3])return 6;
	if(((tDW[4]+tDW[5]+tDW[6])&0xFF)-tDW[7])return 6;
	return 0;
}

// Viaccess EMU
static char via_030B00_X1[]={0x8B, 0x29, 0x08, 0xAE, 0x39, 0xB0, 0x10, 0x1A}; // TNTSat 19.2°E
static char via_030B00_C1[]={0xFC, 0x3C, 0x5F, 0x15, 0xD7, 0xA0, 0xB4, 0x37}; // TNTSat 19.2°E
static char via_030B00_D1[]={0x89, 0xCB, 0xB5, 0x4E, 0xEB, 0x94, 0xE3, 0x50}; // TNTSat 19.2°E
static char via_030B00_P1[]={0x07, 0x04, 0x02, 0x03, 0x05, 0x00, 0x06, 0x01}; // TNTSat 19.2°E
static char via_030B00_T1[]={0x7F, 0x30, 0x8A, 0x9B, 0x0B, 0x80, 0x3C, 0x4B, 0x6B, 0xBF, 0xEF, 0xB0, 0x41, 0xF0, 0x3B, 0x58, 0xE3,
	 0xBC, 0xAF, 0xBE, 0xE5, 0x83, 0xB4, 0x15, 0xA5, 0x35, 0x71, 0x95, 0x2B, 0xAB, 0xE9, 0xA1, 0x79, 0x64, 0xA8, 0xBA, 0x8B, 0xE4,
	 0xE0, 0x53, 0xDB, 0x6A, 0xD5, 0x51, 0x1C, 0x06, 0x37, 0x39, 0xD8, 0x17, 0x86, 0xDD, 0x92, 0x87, 0x1B, 0xD7, 0xB7, 0x18, 0xE7,
	 0x31, 0x2D, 0xDC, 0x77, 0xF3, 0xDE, 0x1A, 0xD4, 0xFC, 0x60, 0x93, 0x29, 0x09, 0x70, 0x3D, 0x97, 0xB9, 0x68, 0x8F, 0x2C, 0x5B,
	 0x62, 0x21, 0xF5, 0x01, 0xD0, 0x89, 0x4F, 0x99, 0x50, 0x28, 0xA9, 0xFD, 0x4A, 0x3F, 0x98, 0x61, 0xF7, 0x1F, 0x20, 0xAD, 0x03,
	 0x08, 0x46, 0xCF, 0x54, 0x6C, 0x44, 0xDF, 0x76, 0xC1, 0x8D, 0x00, 0x78, 0xF6, 0x07, 0xA0, 0xF1, 0x7A, 0x2E, 0x32, 0x11, 0x9F,
	 0xF4, 0x85, 0x2F, 0x49, 0x13, 0xC2, 0x59, 0x9E, 0x96, 0x23, 0xEB, 0x19, 0x24, 0xED, 0xDA, 0x25, 0xFE, 0xCA, 0xF8, 0x94, 0x22,
	 0x65, 0x33, 0x57, 0x9D, 0xC3, 0xA3, 0x9C, 0x0E, 0x56, 0x04, 0x48, 0x82, 0xBD, 0x75, 0x5A, 0xAC, 0xFA, 0x26, 0xA6, 0xB3, 0xE8,
	 0x05, 0x67, 0xEC, 0xB1, 0x36, 0xE6, 0xFF, 0x4D, 0x10, 0x7E, 0x8C, 0xB8, 0xAA, 0xE1, 0xEE, 0x8E, 0x5F, 0xC4, 0x2A, 0x9A, 0x52,
	 0x5D, 0xC7, 0x02, 0xCD, 0x72, 0xB2, 0x0F, 0x88, 0xD9, 0x43, 0x7D, 0x16, 0xD6, 0x5E, 0xCB, 0x40, 0x6D, 0x14, 0x4C, 0xF9, 0x74,
	 0x5C, 0x12, 0x7C, 0xC0, 0xC9, 0xA7, 0xA4, 0x90, 0x69, 0xBB, 0x0D, 0xEA, 0xC8, 0xC5, 0x63, 0xAE, 0x66, 0x4E, 0xD2, 0x6F, 0xB5,
	 0x7B, 0x27, 0x1D, 0x6E, 0x3E, 0xCE, 0xC6, 0x45, 0xD3, 0xD1, 0x42, 0xE2, 0x38, 0xA2, 0xB6, 0x3A, 0x34, 0x0A, 0xFB, 0x73, 0x91,
	 0x1E, 0x84, 0xCC, 0x47, 0xF2, 0x0C, 0x81, 0x55}; // TNTSat 19.2°E
static char via_030B00_08[]={0xD9, 0x2A, 0x82, 0xE3, 0x11, 0x1C, 0x9A, 0xFD, 0xED, 0x46, 0x26, 0xAD, 0x0B, 0x54, 0x2E, 0x5B}; // TNTSat 19.2°E
static char via_030B00_09[]={0xCC, 0x72, 0xAA, 0xFE, 0x52, 0x65, 0xD6, 0xD0, 0x46, 0x08, 0xE8, 0x32, 0xCE, 0xB9, 0xEC, 0x4D}; // TNTSat 19.2°E
static char via_030B00_0A[]={0xEE, 0x26, 0x79, 0x60, 0x4A, 0x7D, 0xDD, 0x32, 0xE4, 0xF8, 0xCB, 0x91, 0x35, 0x00, 0xD3, 0x8E}; // TNTSat 19.2°E
static char via_030B00_E1[]={0xF7, 0x86, 0xE2, 0x1F, 0x17, 0x82, 0xBE, 0x09, 0x75, 0x53, 0xB0, 0xD3, 0x49, 0xE0, 0x36, 0x2A}; // TNTSat 19.2°E
static char via_030B00_E2[]={0x48, 0xC1, 0x9B, 0x86, 0xA4, 0xE2, 0xEB, 0x72, 0x88, 0xDF, 0xDC, 0xE7, 0xC2, 0xBB, 0x75, 0x77}; // TNTSat 19.2°E
static char via_030B00_ED[]={0x25, 0x04, 0xB3, 0x82, 0xB1, 0x6D, 0x8C, 0x67, 0x58, 0xDB, 0x96, 0x0E, 0x31, 0x1E, 0x93, 0x51}; // TNTSat 19.2°E
static char via_030B00_EF[]={0x4A, 0x26, 0xAD, 0x25, 0x17, 0x95, 0xA5, 0x8A, 0x11, 0xBB, 0xC0, 0x7B, 0x53, 0xC4, 0x43, 0x48}; // TNTSat 19.2°E
static char via_030B00_E10[]={0x28, 0x44, 0x25, 0x87, 0x92, 0xF6, 0xD9, 0x52, 0x9A, 0x32, 0x8B, 0x3E, 0x8C, 0xD2, 0xFD, 0x0E}; // TNTSat 19.2°E
static char via_030B00_E11[]={0xF6, 0x2C, 0x3B, 0xE3, 0x00, 0xE9, 0x8B, 0xBB, 0x37, 0x8D, 0xFA, 0x38, 0xBB, 0x6E, 0xEE, 0xF1}; // TNTSat 19.2°E
static char via_030B00_E12[]={0xDD, 0x39, 0x0F, 0xED, 0x21, 0x91, 0xD2, 0x82, 0x01, 0x60, 0x25, 0x09, 0x3E, 0xE0, 0x91, 0xBA}; // TNTSat 19.2°E
static char via_030B00_E13[]={0x8C, 0x61, 0x99, 0xE0, 0xB8, 0x87, 0x7F, 0x4B, 0xD2, 0x13, 0x6E, 0xF6, 0xD1, 0x14, 0x2D, 0x15}; // TNTSat 19.2°E
static char via_030B00_E14[]={0x6B, 0x1B, 0x02, 0x4D, 0x36, 0xF6, 0x09, 0x74, 0x97, 0x3C, 0xB8, 0x1F, 0xA5, 0xE8, 0xF0, 0x1C}; // TNTSat 19.2°E
static char via_023800_X1[]={0x3D, 0x12, 0x29, 0xC2, 0xED, 0x29, 0xC1, 0x30}; // SSR/SRG Swiss 13.0°E
static char via_023800_C1[]={0xDA, 0x4E, 0x0B, 0x42, 0x13, 0x4F, 0x43, 0x3B}; // SSR/SRG Swiss 13.0°E
static char via_023800_D1[]={0x0B, 0x55, 0x2C, 0x11, 0xAF, 0x0A, 0x2F, 0xB8}; // SSR/SRG Swiss 13.0°E
static char via_023800_P1[]={0x03, 0x04, 0x00, 0x02, 0x07, 0x05, 0x01, 0x06}; // SSR/SRG Swiss 13.0°E
static char via_023800_T1[]={0x49, 0x5E, 0xE3, 0x83, 0x9D, 0xD6, 0x54, 0x4A, 0xF2, 0x3F, 0xC1, 0xEC, 0x7C, 0x68, 0x4D, 0x97, 0x3C,
	 0xF8, 0x73, 0x92, 0x6F, 0x1C, 0xF0, 0xF5, 0x86, 0xDA, 0x31, 0xA1, 0x15, 0x6B, 0xA6, 0x9A, 0x34, 0x61, 0xA2, 0x8A, 0x96, 0x5D,
	 0xCB, 0x2A, 0x5B, 0x93, 0x27, 0x57, 0xB3, 0xD3, 0x46, 0x40, 0xCF, 0x1A, 0x65, 0x4C, 0x20, 0xA5, 0x48, 0xC3, 0xE6, 0xB9, 0x7D,
	 0xBD, 0x10, 0x43, 0x63, 0x09, 0x2F, 0xC8, 0x9C, 0x3D, 0x67, 0xD8, 0x2E, 0xAE, 0x36, 0x00, 0x07, 0x5C, 0x33, 0x0E, 0x8D, 0x94,
	 0x6A, 0xE8, 0xAD, 0x5F, 0x06, 0xE7, 0xF4, 0x2B, 0xB7, 0x9F, 0xAC, 0x28, 0x4B, 0x25, 0x01, 0x53, 0xBE, 0x21, 0x7A, 0x3E, 0xFD,
	 0x32, 0x91, 0x44, 0x0D, 0x8F, 0x50, 0x1D, 0xC5, 0xA8, 0xA7, 0x14, 0x7B, 0xBB, 0xED, 0x12, 0xC4, 0xBF, 0x89, 0xDB, 0xDC, 0xD1,
	 0x62, 0x70, 0x0C, 0x3A, 0x95, 0xAF, 0xB4, 0xF6, 0xB1, 0x03, 0xC6, 0xFB, 0x18, 0xDD, 0xC7, 0x76, 0x23, 0xE4, 0xFC, 0x4E, 0xF1,
	 0x69, 0xAA, 0x66, 0x13, 0x29, 0x4F, 0x7E, 0x24, 0x0F, 0xB5, 0x9E, 0x1F, 0xAB, 0x42, 0x04, 0x99, 0x59, 0xD9, 0x1B, 0x22, 0xB6,
	 0xF9, 0x35, 0x1E, 0x6E, 0x6D, 0xC2, 0x90, 0x9B, 0x5A, 0x52, 0x47, 0x7F, 0xC0, 0x82, 0xFE, 0x2C, 0x80, 0x98, 0xA3, 0x58, 0xD7,
	 0x39, 0xDE, 0x71, 0xD2, 0x3B, 0x60, 0x75, 0xB0, 0x0A, 0xA9, 0x37, 0x74, 0x6C, 0x84, 0x88, 0x41, 0xCA, 0xC9, 0x26, 0xF7, 0xEA,
	 0xE2, 0x30, 0xDF, 0x79, 0xBC, 0x2D, 0xEE, 0xF3, 0x05, 0xB8, 0x45, 0xA0, 0x19, 0x77, 0x78, 0x87, 0xBA, 0xD0, 0xFA, 0xE5, 0xCE,
	 0x02, 0x8B, 0xE1, 0x38, 0x55, 0x51, 0xEF, 0xCD, 0xFF, 0x0B, 0x72, 0xD4, 0xE9, 0x16, 0x8E, 0x08, 0xD5, 0x56, 0x17, 0x81, 0xCC,
	 0x11, 0xA4, 0xE0, 0x85, 0x8C, 0x64, 0xB2, 0xEB, 0x1E, 0x84, 0xCC, 0x47, 0xF2, 0x0C, 0x81, 0x55}; // SSR/SRG Swiss 13.0°E
static char via_023800_08[]={0x95, 0x5C, 0xF3, 0xC9, 0x28, 0x00, 0xF3, 0x9F, 0x54, 0xB9, 0x30, 0x05, 0xDF, 0x82, 0x6D, 0xBF}; // SSR/SRG Swiss 13.0°E
static char via_021110_X1[]={0x12, 0xBF, 0x4D, 0x2F, 0x2A, 0x10, 0xF5, 0x90}; // ANT 1 Europe 9°E
static char via_021110_C1[]={0xE9, 0x6B, 0x9F, 0xD4, 0x4D, 0x3D, 0x33, 0x5E}; // ANT 1 Europe 9°E
static char via_021110_D1[]={0x89, 0xCB, 0xB5, 0x4E, 0xEB, 0x94, 0xE3, 0x50}; // ANT 1 Europe 9°E
static char via_021110_P1[]={0x07, 0x04, 0x02, 0x03, 0x05, 0x00, 0x06, 0x01}; // ANT 1 Europe 9°E
static char via_021110_T1[]={0x94, 0x53, 0x84, 0x7e, 0xee, 0x73, 0x45, 0xcf, 0xd1, 0xd4, 0x82, 0xd3, 0x60, 0x30, 0x36, 0xec, 0xd6,
	0xcd, 0x9a, 0xf5, 0xda, 0x1f, 0xe5, 0x24, 0x3e, 0x71, 0x5c, 0xea, 0x86, 0x41, 0xba, 0x15, 0x28, 0xa7, 0x47, 0xc2, 0x17, 0x2e,
	0xdc, 0xd9, 0x20, 0x96, 0x8e, 0x75, 0x2f, 0x4a, 0x25, 0x2c, 0x0d, 0x38, 0xab, 0x4c, 0xa5, 0x6e, 0x0e, 0x8d, 0x31, 0x64, 0x4e,
	0x5e, 0x77, 0x61, 0x18, 0x9f, 0x78, 0x1d, 0xfa, 0x85, 0xfd, 0x06, 0x59, 0x22, 0xf7, 0xe9, 0x2d, 0x95, 0x33, 0xa9, 0x3a, 0xe8,
	0xf1, 0xe7, 0x88, 0x01, 0x5d, 0xe3, 0xd2, 0x92, 0x62, 0x46, 0x5f, 0xf2, 0x1a, 0x54, 0x3b, 0x5a, 0x0c, 0x3d, 0x58, 0xc9, 0x39,
	0xd8, 0xae, 0x7f, 0x87, 0x6c, 0xbf, 0xd5, 0x69, 0xce, 0x35, 0xc4, 0x9b, 0x19, 0xc1, 0x05, 0xc8, 0x2b, 0xac, 0x3c, 0x40, 0xed,
	0xb1, 0xfc, 0xbc, 0x99, 0x03, 0x67, 0xa4, 0xb8, 0x0a, 0xa1, 0x02, 0x43, 0x1c, 0x68, 0x52, 0xf8, 0xbe, 0xff, 0xb6, 0x37, 0x2a,
	0xef, 0xb9, 0xa6, 0x57, 0xbb, 0x00, 0x4b, 0x29, 0xb4, 0xdb, 0x7d, 0x12, 0x70, 0xe1, 0xaa, 0xb5, 0x3f, 0xd0, 0x83, 0xb7, 0xe2,
	0x80, 0x34, 0x91, 0x21, 0xe4, 0x4d, 0x9d, 0x32, 0x76, 0xf0, 0x66, 0xdf, 0xde, 0x7a, 0xcc, 0xc7, 0x97, 0x9e, 0x8c, 0xa2, 0x81,
	0x90, 0x1e, 0x93, 0x7c, 0xc3, 0x8a, 0x6a, 0xe6, 0x72, 0x23, 0xbd, 0x6f, 0xf6, 0xca, 0xb3, 0x74, 0x63, 0xc6, 0xfe, 0xb2, 0x11,
	0x6d, 0x07, 0xa0, 0x08, 0x56, 0x0b, 0x09, 0x6b, 0x10, 0xe0, 0x65, 0x27, 0x14, 0x98, 0x26, 0xeb, 0xb0, 0xaf, 0xd7, 0x9c, 0xa3,
	0x55, 0xa8, 0x16, 0xc0, 0x51, 0x4f, 0x49, 0x1b, 0xdd, 0x0f, 0x79, 0x04, 0x8f, 0xad, 0x50, 0x5b, 0xf4, 0xf3, 0x13, 0xc5, 0x48,
	0x89, 0xfb, 0x42, 0xf9, 0x7b, 0x44, 0xcb, 0x8b}; // ANT 1 Europe 9°E
static char via_021110_08[]={0x35, 0xF6, 0xB9, 0x21, 0x7C, 0x2B, 0xA8, 0x3F, 0x38, 0xC3, 0xC1, 0xFD, 0x23, 0xA5, 0xD3, 0x21}; // ANT 1 Europe 9°E
static char via_007400_08[]={0x2F, 0xB6, 0x02, 0x39, 0xD4, 0xD8, 0xC4, 0xD6}; // TV-MCM 13°E
static char via_007800_08[]={0x5B, 0x14, 0x1E, 0x38, 0x9C, 0x46, 0x24, 0x5F}; // SIC 16°E
static char via_007800_09[]={0x60, 0x6C, 0x85, 0xDA, 0x02, 0x41, 0x90, 0x61}; // SIC 16°E
static char via_007800_0A[]={0x71, 0x15, 0xCE, 0x2E, 0xF8, 0x36, 0x42, 0x20}; // SIC 16°E

char GetViaKey(unsigned char *buf, unsigned int ident, char keyName, unsigned char keyIndex) {
	
	switch(ident) {
		case 0x030B00:  // TNTSat 19.2°E
			if(keyName == 'X' && keyIndex == 0x01) { memcpy(buf,via_030B00_X1,8);	return 1; }
			if(keyName == 'C' && keyIndex == 0x01) { memcpy(buf,via_030B00_C1,8);	return 1; }
			if(keyName == 'D' && keyIndex == 0x01) { memcpy(buf,via_030B00_D1,8);	return 1; }
			if(keyName == 'P' && keyIndex == 0x01) { memcpy(buf,via_030B00_P1,8);	return 1; }
			if(keyName == 'T' && keyIndex == 0x01) { memcpy(buf,via_030B00_T1,256);	return 1; }
			if(keyName == 0 && keyIndex == 0x08) { memcpy(buf,via_030B00_08,16); return 1; }
			if(keyName == 0 && keyIndex == 0x09) { memcpy(buf,via_030B00_09,16); return 1; }
			if(keyName == 0 && keyIndex == 0x0A) { memcpy(buf,via_030B00_0A,16); return 1; }
			if(keyName == 'E' && keyIndex == 0x01) { memcpy(buf,via_030B00_E1,16); return 1; }
			if(keyName == 'E' && keyIndex == 0x02) { memcpy(buf,via_030B00_E2,16); return 1; }	
			if(keyName == 'E' && keyIndex == 0x0D) { memcpy(buf,via_030B00_ED,16); return 1; }
			if(keyName == 'E' && keyIndex == 0x0F) { memcpy(buf,via_030B00_EF,16); return 1; }
			if(keyName == 'E' && keyIndex == 0x10) { memcpy(buf,via_030B00_E10,16); return 1; }
			if(keyName == 'E' && keyIndex == 0x11) { memcpy(buf,via_030B00_E11,16); return 1; }
			if(keyName == 'E' && keyIndex == 0x12) { memcpy(buf,via_030B00_E12,16); return 1; }	
			if(keyName == 'E' && keyIndex == 0x13) { memcpy(buf,via_030B00_E13,16); return 1; }
			if(keyName == 'E' && keyIndex == 0x14) { memcpy(buf,via_030B00_E14,16); return 1; }														
			break;
		case 0x023800: // SSR/SRG Swiss 13.0°E
			if(keyName == 'X' && keyIndex == 0x01) { memcpy(buf,via_023800_X1,8);	return 1; }
			if(keyName == 'C' && keyIndex == 0x01) { memcpy(buf,via_023800_C1,8);	return 1; }
			if(keyName == 'D' && keyIndex == 0x01) { memcpy(buf,via_023800_D1,8);	return 1; }
			if(keyName == 'P' && keyIndex == 0x01) { memcpy(buf,via_023800_P1,8);	return 1; }
			if(keyName == 'T' && keyIndex == 0x01) { memcpy(buf,via_023800_T1,264);	return 1; }
			if(keyName == 0 && keyIndex == 0x08) { memcpy(buf,via_023800_08,16); return 1; }
			break;
		case 0x021110: // ANT 1 Europe 9°E
			if(keyName == 'X' && keyIndex == 0x01) { memcpy(buf,via_021110_X1,8);	return 1; }
			if(keyName == 'C' && keyIndex == 0x01) { memcpy(buf,via_021110_C1,8);	return 1; }
			if(keyName == 'D' && keyIndex == 0x01) { memcpy(buf,via_021110_D1,8);	return 1; }
			if(keyName == 'P' && keyIndex == 0x01) { memcpy(buf,via_021110_P1,8);	return 1; }
			if(keyName == 'T' && keyIndex == 0x01) { memcpy(buf,via_021110_T1,256);	return 1; }
			if(keyName == 0 && keyIndex == 0x08) { memcpy(buf,via_021110_08,16); return 1; }
			break;
		case 0x007400: // TV-MCM 13°E
			if(keyName == 0 && keyIndex == 0x08) { memcpy(buf,via_007400_08,8); return 1; }
			break;
		case 0x007800: // SIC 16°E
			if(keyName == 0 && keyIndex == 0x08) { memcpy(buf,via_007800_08,8); return 1; }
			if(keyName == 0 && keyIndex == 0x09) { memcpy(buf,via_007800_09,8); return 1; }
			if(keyName == 0 && keyIndex == 0x0A) { memcpy(buf,via_007800_0A,8); return 1; }				
			break;					
	}
	return 0;
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
    if(!GetViaKey(work_key, ident, 0, desKeyIndex)) return 2;

    data = source+9;
    len = source[2]-6;
    des_data1 = dw;
    des_data2 = dw+8;
 
    msg_pos = 0;
    pH = 0; memset(hashbuffer, 0, sizeof(hashbuffer));
    memcpy(hashkey, work_key, sizeof(hashkey));

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
        { hashbuffer[pH] ^= data[i++]; pH++; if(pH == 8) { des_encrypt(hashbuffer, 8, hashkey); pH = 0; } }
        { hashbuffer[pH] ^= data[i++]; pH++; if(pH == 8) { des_encrypt(hashbuffer, 8, hashkey); pH = 0; } }
        for (hash_start=0; hash_start < data[1];hash_start++) 
        	{ hashbuffer[pH] ^= data[i++]; pH++; if(pH == 8) { des_encrypt(hashbuffer, 8, hashkey); pH = 0; } }
        while (pH != 0) { hashbuffer[pH] ^= (unsigned char) 0; pH++; if(pH == 8) { des_encrypt(hashbuffer, 8, hashkey); pH = 0; } }
    }   
    if (work_key[7] == 0) {
        for (; i < encStart + 16; i++) 
        	{ hashbuffer[pH] ^= data[i]; pH++; if(pH == 8) { des_encrypt(hashbuffer, 8, hashkey); pH = 0; } }
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
            	{ hashbuffer[pH] ^= data[i]; pH++; if(pH == 8) { des_encrypt(hashbuffer, 8, hashkey); pH = 0; } }
            k = ((work_key[7] & 0xf0) == 0) ? 0x5a : 0xa5;
            for (i=0; i<8; i++) {
                tmp = des_data1[i];
                des_data1[i] = (k & hashbuffer[pH] ) ^ tmp;
                { hashbuffer[pH] ^= tmp; pH++; if(pH == 8) { des_encrypt(hashbuffer, 8, hashkey); pH = 0; } };
            }
          	for (i = 0; i < 8; i++) {
                tmp = des_data2[i];
                des_data2[i] = (k & hashbuffer[pH] ) ^ tmp;
                { hashbuffer[pH] ^= tmp; pH++; if(pH == 8) { des_encrypt(hashbuffer, 8, hashkey); pH = 0; } };
            }
        }
        else {
            for (; i < encStart + 16; i++)
            	{ hashbuffer[pH] ^= data[i]; pH++; if(pH == 8) { des_encrypt(hashbuffer, 8, hashkey); pH = 0; } };
        }
    }
		des_decrypt(des_data1, 8, prepared_key);
		des_decrypt(des_data2, 8, prepared_key);
		des_encrypt(hashbuffer, 8, hashkey);
    if (memcmp(signature, hashbuffer, 8)) return 6;
    return 0;
}

char Via26ProcessDw(unsigned char *indata, unsigned int ident, unsigned char desKeyIndex)
{
	unsigned char pv1,pv2, i;
	unsigned char Tmp[8], tmpKey[16], T1Key[300], P1Key[8], KeyDes1[16], KeyDes2[16], XorKey[8];
	
	if(!GetViaKey(T1Key, ident, 'T', 1)) return 2;
	if(!GetViaKey(P1Key, ident, 'P', 1)) return 2;
	if(!GetViaKey(KeyDes1, ident, 'D', 1)) return 2;
	if(!GetViaKey(KeyDes2, ident, 0, desKeyIndex)) return 2;	
	if(!GetViaKey(XorKey, ident, 'X', 1)) return 2;

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
	if(!GetViaKey(C1, ident, 'C', 1)) return 2;
	    	
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

	if(!GetViaKey(T1Key, ident, 'T', 1)) return 2;
	if(!GetViaKey(P1Key, ident, 'P', 1)) return 2;
	if(!GetViaKey(KeyDes, ident, 0, desKeyIndex)) return 2;	
	if(!GetViaKey(XorKey, ident, 'X', 1)) return 2;

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

bool Via3HasValidCRC(unsigned char *dw)
{
	if (((dw[0]+dw[1]+dw[2]) & 0xFF) != dw[3]) return false;
	if (((dw[4]+dw[5]+dw[6]) & 0xFF) != dw[7]) return false;
	if (((dw[8]+dw[9]+dw[10]) & 0xFF) != dw[11]) return false;
	if (((dw[12]+dw[13]+dw[14]) & 0xFF) != dw[15]) return false;
	return true;
}

char Via3Decrypt(unsigned char* source, unsigned char* dw, unsigned int ident, unsigned char desKeyIndex, unsigned char aesKeyIndex, unsigned char aesMode, bool doFinalMix)
{
	bool aesAfterCore = false;
	bool needsAES = (aesKeyIndex != 0xFF);
	unsigned char tmpData[8], C1[8];
	unsigned char *pXorVector;
	char aesKey[16];
	int i, j;
    
	if(ident == 0) return 4;
	if(!GetViaKey(C1, ident, 'C', 1)) return 2;		
	if(needsAES && !GetViaKey((unsigned char*)aesKey, ident, 'E', aesKeyIndex)) return 2;
	if(aesMode==0x0D || aesMode==0x11 || aesMode==0x15) aesAfterCore = true;

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
	bool doFinalMix = false;
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
					doFinalMix = false;
					if (currentIdent == 0x030B00 && providerKeyLen>3) {
						if (ecm[keySelectPos]==0x05 && ecm[keySelectPos+1]==0x67 && (ecm[keySelectPos+2]==0x00 || ecm[keySelectPos+2]==0x01)) {
							if(ecm[keySelectPos+2]==0x01) doFinalMix = true;
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
	return 2;
}

// Nagra EMU
static char nagra_2111_M1[]={0x75, 0x08, 0x66, 0x58, 0x01, 0xDC, 0x68, 0x15, 0xF8, 0xDF, 0x96, 0x56, 0x41, 0xA8, 0xC2, 0x7C, 0xAE,
	0x05, 0x6E, 0xD0, 0x24, 0x9C, 0xF6, 0x0F, 0x60, 0xDE, 0xDD, 0x75, 0xA3, 0xA8, 0x4D, 0x48, 0x6E, 0x85, 0x28, 0x52, 0x20, 0xF1,
	0x33, 0x43, 0x9F, 0x03, 0x8B, 0xFE, 0x54, 0x97, 0x55, 0x43, 0xC8, 0xDF, 0xC4, 0x20, 0x50, 0x67, 0x98, 0x5E, 0x5C, 0xDA, 0xD7,
	0xAB, 0xA4, 0xCF, 0x48, 0xA7}; // Digi TV Cablu (Romania Cable TV)
static char nagra_2111_00[]={0x70, 0x13, 0x9b, 0xa3, 0x2f, 0xa2, 0x53, 0x37, 0x59, 0x94, 0x19, 0x9b, 0xb5, 0x06, 0x57, 0xff}; // Digi TV Cablu (Romania Cable TV)
static char nagra_2111_01[]={0x10, 0xF5, 0xDB, 0x01, 0x64, 0x6F, 0xD8, 0x85, 0x2C, 0x0B, 0xCC, 0xA1, 0x60, 0xF0, 0xB3, 0x37}; // Digi TV Cablu (Romania Cable TV)
static char nagra_7301_M1[]={0x75, 0x44, 0xE9, 0x82, 0x06, 0x5D, 0x53, 0xE7, 0xCF, 0x04, 0x9A, 0xF2, 0xE7, 0x25, 0x54, 0x4A, 0x50,
	0x26, 0x8D, 0xA1, 0x46, 0x2F, 0x15, 0xD8, 0xA1, 0x97, 0xEC, 0xE4, 0xB8, 0x0F, 0xE2, 0x21, 0x12, 0x2F, 0x92, 0xAD, 0x56, 0x32,
	0x43, 0x80, 0xA0, 0x48, 0x78, 0xEB, 0xF5, 0x63, 0x42, 0x7D, 0xD8, 0x5C, 0x38, 0x64, 0x59, 0x07, 0x58, 0x3E, 0x23, 0xAC, 0x51,
	0x53, 0xE3, 0x00, 0xCF, 0x84}; // UPC Cablecom (Swiss Cable TV network) 
static char nagra_7301_00[]={0x79, 0xE8, 0xFD, 0xEB, 0x4E, 0xAB, 0x12, 0x08, 0x92, 0xF6, 0x8D, 0x63, 0x19, 0x99, 0x32, 0x83}; // UPC Cablecom (Swiss Cable TV network) 
static char nagra_7301_01[]={0xC3, 0xD6, 0xA2, 0x28, 0x03, 0xC2, 0x80, 0xA7, 0xAC, 0xDD, 0x7B, 0x2E, 0xB0, 0x0F, 0x33, 0xCC}; // UPC Cablecom (Swiss Cable TV network) 
static char nagra_C102_M1[]={0x31, 0x22, 0xD1, 0xC2, 0x29, 0x07, 0xB3, 0x98, 0xDC, 0x25, 0x3F, 0xEA, 0x29, 0xD0, 0x33, 0x85, 0x97,
	0x1F, 0xD3, 0xDF, 0xBB, 0xD5, 0x23, 0x5A, 0x53, 0x19, 0x59, 0xE9, 0x4C, 0x86, 0x63, 0x78, 0x47, 0xDD, 0xE5, 0x8E, 0x6E, 0x89,
	0x97, 0xA4, 0x11, 0xD7, 0x03, 0x1F, 0x8A, 0xA0, 0xCB, 0x0A, 0x3E, 0xBB, 0x7C, 0x42, 0x0E, 0x04, 0x3B, 0x31, 0xF6, 0xEA, 0xDA,
	0x31, 0x10, 0x0C, 0x2A, 0xA0}; // TV Globo
static char nagra_C102_00[]={0x5B, 0xCE, 0x65, 0x62, 0x5F, 0xA4, 0xED, 0x50, 0x95, 0x94, 0xCE, 0x7C, 0x4B, 0x90, 0x7E, 0xCB}; // TV Globo
static char nagra_C102_01[]={0xD7, 0xCB, 0xE9, 0x3D, 0x30, 0xE2, 0xC9, 0x13, 0x91, 0x07, 0x38, 0x74, 0x57, 0xDB, 0x90, 0x23}; // TV Globo
static char nagra_1101_M1[]={0xC7, 0x5C, 0x2F, 0xEC, 0xF4, 0x94, 0xF6, 0xDD, 0x71, 0xBB, 0xF8, 0x98, 0xC6, 0x3F, 0x7F, 0xEB, 0xA1,
	0x13, 0x45, 0xDF, 0x14, 0xC7, 0x1E, 0xE6, 0x56, 0xBC, 0x23, 0x01, 0x6C, 0x2F, 0x90, 0xA8, 0x81, 0x42, 0x9B, 0x82, 0xA6, 0x8C,
	0x09, 0x5E, 0x47, 0x2C, 0x99, 0x77, 0x52, 0xD5, 0x89, 0x03, 0x41, 0xCF, 0x9E, 0x06, 0x69, 0x16, 0x72, 0x48, 0x27, 0x18, 0x75,
	0x0B, 0x91, 0x8A, 0xAE, 0x92}; // Kabel Deutschland
static char nagra_1101_00[]={0x9A, 0xF1, 0x1C, 0xE0, 0x87, 0x1C, 0x97, 0x91, 0x1F, 0xFF, 0x40, 0x99, 0x85, 0x1B, 0x86, 0x7D}; // Kabel Deutschland

char GetNagraKey(unsigned char *buf, unsigned int ident, char keyName, unsigned char keyIndex)
{
	switch(ident) {
		case 0x2111:  // Digi TV Cablu (Romanian Cable TV)
			if(keyName == 'M' && keyIndex == 0x01) { memcpy(buf,nagra_2111_M1,64); return 1; }
			if(keyName == 00 && keyIndex == 0x00) { memcpy(buf,nagra_2111_00,16); return 1; }
			if(keyName == 00 && keyIndex == 0x01) { memcpy(buf,nagra_2111_01,16); return 1; }												
			break;				
		case 0x7301:  // UPC Cablecom (Swiss Cable TV) 
			if(keyName == 'M' && keyIndex == 0x01) { memcpy(buf,nagra_7301_M1,64); return 1; }
			if(keyName == 00 && keyIndex == 0x00) { memcpy(buf,nagra_7301_00,16); return 1; }
			if(keyName == 00 && keyIndex == 0x01) { memcpy(buf,nagra_7301_01,16); return 1; }												
			break;	
		case 0xC102:  //  TV Globo
			if(keyName == 'M' && keyIndex == 0x01) { memcpy(buf,nagra_C102_M1,64); return 1; }
			if(keyName == 00 && keyIndex == 0x00) { memcpy(buf,nagra_C102_00,16); return 1; }
			if(keyName == 00 && keyIndex == 0x01) { memcpy(buf,nagra_C102_01,16); return 1; }												
			break;	
		case 0x1101:  // Kabel Deutschland
			if(keyName == 'M' && keyIndex == 0x01) { memcpy(buf,nagra_1101_M1,64); return 1; }
			if(keyName == 00 && keyIndex == 0x00) { memcpy(buf,nagra_1101_00,16); return 1; }										
			break;							
	}
	return 0;
}

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

bool Nagra2RSAInput(BIGNUM *d, const unsigned char *in, int n, bool LE)
{
	if(LE) {
		unsigned char tmp[256];
		ReverseMemInOut(tmp,in,n);
		return BN_bin2bn(tmp,n,d)!=0;
	}
	else
		return BN_bin2bn(in,n,d)!=0;
}

int Nagra2RSAOutput(unsigned char *out, int n, BIGNUM *r, bool LE)
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

int Nagra2RSA(unsigned char *out, const unsigned char *in, int n, BIGNUM *exp, BIGNUM *mod, bool LE)
{
	BN_CTX *ctx;
	BIGNUM *r, *d;
	int result = 0;

	ctx = BN_CTX_new();
	r = BN_new();
	d = BN_new();
  
	if(Nagra2RSAInput(d,in,n,LE) && BN_mod_exp(r,d,exp,mod,ctx)) 
		result = Nagra2RSAOutput(out,n,r,LE);
	
	BN_free(d);
	BN_free(r);
	BN_CTX_free(ctx);
	return result;
}

bool Nagra2Signature(const unsigned char *vkey, const unsigned char *sig, const unsigned char *msg, int len)
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

bool DecryptNagra2ECM(unsigned char *in, unsigned char *out, const unsigned char *key, int len, const unsigned char *vkey, unsigned char *keyM)
{
  BIGNUM *exp, *mod;
  unsigned char iv[8];
	int i = 0, sign = in[0] & 0x80;
	unsigned char binExp = 3;
	bool result = true;
	
	exp = BN_new(); 
	mod = BN_new();
	BN_bin2bn(&binExp, 1, exp); 
	BN_bin2bn(keyM, 64, mod);
	 	
	if(Nagra2RSA(out,in+1,64,exp,mod,true)<=0) { BN_free(exp); BN_free(mod); return false; }
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
  if(result && Nagra2RSA(out,out,64,exp,mod,false)<=0) result = false;
	if(result && vkey && !Nagra2Signature(vkey,out,out+8,len-8)) result = false;
    
  BN_free(exp); BN_free(mod);	   
  return result;
}

char Nagra2ECM(unsigned char *ecm, unsigned char *dw)
{
	unsigned int ident, identMask, tmp1, tmp2, tmp3;
	unsigned char cmdLen, ideaKeyNr, dec[256], ideaKey[16], vKey[16], m1Key[64], mecmAlgo = 0;
	bool useVerifyKey = false;
	int l=0, s;
	uint16_t i = 0, ecmLen = (((ecm[1] & 0x0f)<< 8) | ecm[2])+3;
	
	cmdLen = ecm[4] - 5;	
	ident = (ecm[5] << 8) + ecm[6];	
	ideaKeyNr = (ecm[7]&0x10)>>4;
	if(ideaKeyNr) ideaKeyNr = 1;
	if(ident == 1283 || ident == 1285 || ident == 1297) ident = 1281;
	if(cmdLen <= 63 || ecmLen < cmdLen + 10) return 1;
	
	if(!GetNagraKey(ideaKey, ident, 0x00, ideaKeyNr)) return 2;		
	if(GetNagraKey(vKey, ident, 'V', 0)) { useVerifyKey = true; }
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
static char irdeto_60400_M1[]={0x98, 0xB4, 0xDC, 0xAD, 0x44, 0xE8, 0xC9, 0x50, 0x4C, 0x3F, 0x4E, 0x51, 0x69, 0x2A, 0x70, 0x47}; // Bulsat 39°E
static char irdeto_60400_M2[]={0xAE, 0x65, 0x2B, 0x21, 0x0B, 0xF8, 0x9F, 0xC6, 0x95, 0x07, 0x60, 0x98, 0x42, 0xFD, 0x30, 0x3E}; // Bulsat 39°E
static char irdeto_60400_01[]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Bulsat 39°E
static char irdeto_60400_02[]={0xDF, 0xDD, 0x0C, 0x4B, 0x92, 0xB4, 0xAB, 0xD2, 0x65, 0x14, 0xB9, 0xAF, 0x9F, 0x0C, 0x79, 0xC4}; // Bulsat 39°E
static char irdeto_60400_03[]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Bulsat 39°E
static char irdeto_60400_04[]={0xE3, 0x1A, 0x2D, 0xE0, 0x01, 0x21, 0x38, 0x62, 0x36, 0xD3, 0x64, 0x1F, 0xD1, 0x52, 0xD4, 0xB4}; // Bulsat 39°E
static char irdeto_60400_05[]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Bulsat 39°E
static char irdeto_60400_06[]={0xDB, 0x88, 0xB6, 0x68, 0xA9, 0x80, 0xCD, 0xBB, 0xCD, 0x8B, 0x06, 0xCE, 0x49, 0xA0, 0xBA, 0x6B}; // Bulsat 39°E

char GetIrdetoKey(unsigned char *buf, unsigned int ident, char keyName, unsigned char keyIndex)
{	
	// irdeto key names collide, so we must add a workaround like trying all 
	// keys for each provder when we want to support more than 1 provider
	switch(ident) {
		case 0x60400:  // Bulsat 39°E
			if(keyName == 'M' && keyIndex == 0x01) { memcpy(buf,irdeto_60400_M1,16); return 1; }
			if(keyName == 'M' && keyIndex == 0x02) { memcpy(buf,irdeto_60400_M2,16); return 1; }
			if(keyName == 00 && keyIndex == 0x01) { memcpy(buf,irdeto_60400_01,16); return 1; }
			if(keyName == 00 && keyIndex == 0x02) { memcpy(buf,irdeto_60400_02,16); return 1; }												
			if(keyName == 00 && keyIndex == 0x03) { memcpy(buf,irdeto_60400_03,16); return 1; }
			if(keyName == 00 && keyIndex == 0x04) { memcpy(buf,irdeto_60400_04,16); return 1; }	
			if(keyName == 00 && keyIndex == 0x05) { memcpy(buf,irdeto_60400_05,16); return 1; }
			if(keyName == 00 && keyIndex == 0x06) { memcpy(buf,irdeto_60400_06,16); return 1; }				
			break;							
	}
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

bool Irdeto2CalculateHash(const unsigned char *okey, const unsigned char *iv, const unsigned char *data, int len)
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
	unsigned char keyNr=0, length, key[16], keyM1[16], keyIV[16], tmp[16];
	unsigned int i, l, ident;
	uint16_t index, ecmLen = (((ecm[1] & 0x0f)<< 8) | ecm[2])+3;
	
	length = ecm[11]; 
	keyNr = ecm[9];
	ident = ecm[8] | CAID << 8;
		
	if(ecmLen < length+12) return 1;
	if(!GetIrdetoKey(key, ident, 0, keyNr)) return 2;
	if(!GetIrdetoKey(keyM1, ident, 'M', 1)) return 2;
	if(!GetIrdetoKey(keyIV, ident, 'M', 2)) return 2;
	
  memset(tmp, 0, 16);
  Irdeto2Encrypt(keyM1, tmp, key, 16);
  ecm+=12;
  Irdeto2Decrypt(ecm, keyIV, keyM1, length); 
  i=(ecm[0]&7)+1;
  
	while(i<length-8) {
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
		while(i<length-8) {
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
0	OK
1	ECM not supported	
2	Key not found
3	Nano80 problem
4	Corrupt data
5	CW not found
6	CW checksum error
*/
char ProcessECM(uint16_t CAID, unsigned char *ecm, unsigned char *dw) {
	if(CAID==0x090F)
		return SoftNDSECM(ecm,dw);
	else if((CAID>>8)==0x0D)
		return CryptoworksProcessECM(ecm,dw);
	else if(CAID==0x500)
		return ViaccessECM(ecm,dw);
	else if(CAID==0x1801)
		return Nagra2ECM(ecm,dw);
	else if(CAID==0x0604)
		return Irdeto2ECM(CAID,ecm,dw);				
	return 1;
}
