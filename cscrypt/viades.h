#ifndef VIADES_H_
#define VIADES_H_

#define DES_HASH            8

#define DES_ECM_CRYPT       0
#define DES_ECM_HASH        DES_HASH

extern void nc_des(unsigned char key[], unsigned char mode, unsigned char data[]);

#endif
