#ifndef CSCRYPT_DES_H_
#define CSCRYPT_DES_H_

#ifdef  __cplusplus
extern "C" {
#endif

#define DES_IP              1
#define DES_IP_1            2
#define DES_RIGHT           4
#define DES_HASH            8

#define DES_ECM_CRYPT       0
#define DES_ECM_HASH        DES_HASH
#define DES_ECS2_DECRYPT    (DES_IP | DES_IP_1 | DES_RIGHT)
#define DES_ECS2_CRYPT      (DES_IP | DES_IP_1)

#define DES_LONG            unsigned long
#define DES_ENCRYPT	1
#define DES_DECRYPT	0
#define DES_KEY_SZ 	(sizeof(DES_cblock))
#define DES_SCHEDULE_SZ (sizeof(DES_key_schedule))
#define ITERATIONS 16

typedef unsigned char DES_cblock[8];
typedef unsigned char const_DES_cblock[8];

typedef struct DES_ks
{
	union
	{
	DES_cblock cblock;
	DES_LONG deslong[2];
	} ks[16];
} DES_key_schedule;

extern int DES_key_sched(const_DES_cblock *key, DES_key_schedule *schedule);
extern void DES_ede3_cbc_encrypt(const unsigned char *input, unsigned char *output,
			  long length, DES_key_schedule *ks1,
			  DES_key_schedule *ks2, DES_key_schedule *ks3,
			  DES_cblock *ivec, int enc);
			  
#define DES_ede2_cbc_encrypt(i,o,l,k1,k2,iv,e) \
DES_ede3_cbc_encrypt((i),(o),(l),(k1),(k2),(k1),(iv),(e))	

extern int des_encrypt(unsigned char *buffer, int len, unsigned char *deskey);
extern int des_decrypt(unsigned char *buffer, int len, unsigned char *deskey);
extern unsigned char *des_login_key_get(unsigned char *key1, unsigned char *key2, int len, unsigned char *des16);
extern void doPC1(unsigned char data[]);
extern void des(unsigned char key[], unsigned char mode, unsigned char data[]);

#ifdef  __cplusplus
}
#endif

#endif
