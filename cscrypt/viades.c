#include "../globals.h"
#include "../helpfunctions.h"
#include "viades.h"

#define DES_IP              1
#define DES_IP_1            2
#define DES_RIGHT           4

static unsigned char PC2[8][6] =
{
	{ 14, 17, 11, 24,  1,  5 },
	{  3, 28, 15,  6, 21, 10 },
	{ 23, 19, 12,  4, 26,  8 },
	{ 16,  7, 27, 20, 13,  2 },
	{ 41, 52, 31, 37, 47, 55 },
	{ 30, 40, 51, 45, 33, 48 },
	{ 44, 49, 39, 56, 34, 53 },
	{ 46, 42, 50, 36, 29, 32 }
};

static unsigned char E[8][6] =
{
	{ 32,  1,  2,  3,  4,  5 },
	{  4,  5,  6,  7,  8,  9 },
	{  8,  9, 10, 11, 12, 13 },
	{ 12, 13, 14, 15, 16, 17 },
	{ 16, 17, 18, 19, 20, 21 },
	{ 20, 21, 22, 23, 24, 25 },
	{ 24, 25, 26, 27, 28, 29 },
	{ 28, 29, 30, 31, 32,  1 }
};

static unsigned char P[32] =
{
	16,  7, 20, 21, 29, 12, 28, 17,  1, 15, 23, 26,  5, 18, 31, 10,
	2,  8, 24, 14, 32, 27,  3,  9, 19, 13, 30,  6, 22, 11,  4, 25
};

static unsigned char SBOXES[4][64] =
{
	{
		0x2e, 0xe0, 0xc4, 0xbf, 0x4d, 0x27, 0x11, 0xc4,
		0x72, 0x4e, 0xaf, 0x72, 0xbb, 0xdd, 0x68, 0x11,
		0x83, 0x5a, 0x5a, 0x06, 0x36, 0xfc, 0xfc, 0xab,
		0xd5, 0x39, 0x09, 0x95, 0xe0, 0x83, 0x97, 0x68,
		0x44, 0xbf, 0x21, 0x8c, 0x1e, 0xc8, 0xb8, 0x72,
		0xad, 0x14, 0xd6, 0xe9, 0x72, 0x21, 0x8b, 0xd7,
		0xff, 0x65, 0x9c, 0xfb, 0xc9, 0x03, 0x57, 0x9e,
		0x63, 0xaa, 0x3a, 0x40, 0x05, 0x56, 0xe0, 0x3d
	},
	{
		0xcf, 0xa3, 0x11, 0xfd, 0xa8, 0x44, 0xfe, 0x27,
		0x96, 0x7f, 0x2b, 0xc2, 0x63, 0x98, 0x84, 0x5e,
		0x09, 0x6c, 0xd7, 0x10, 0x32, 0xd1, 0x4d, 0xea,
		0xec, 0x06, 0x70, 0xb9, 0x55, 0x3b, 0xba, 0x85,
		0x90, 0x4d, 0xee, 0x38, 0xf7, 0x2a, 0x5b, 0xc1,
		0x2a, 0x93, 0x84, 0x5f, 0xcd, 0xf4, 0x31, 0xa2,
		0x75, 0xbb, 0x08, 0xe6, 0x4c, 0x17, 0xa6, 0x7c,
		0x19, 0x60, 0xd3, 0x05, 0xb2, 0x8e, 0x6f, 0xd9
	},
	{
		0x4a, 0xdd, 0xb0, 0x07, 0x29, 0xb0, 0xee, 0x79,
		0xf6, 0x43, 0x03, 0x94, 0x8f, 0x16, 0xd5, 0xaa,
		0x31, 0xe2, 0xcd, 0x38, 0x9c, 0x55, 0x77, 0xce,
		0x5b, 0x2c, 0xa4, 0xfb, 0x62, 0x8f, 0x18, 0x61,
		0x1d, 0x61, 0x46, 0xba, 0xb4, 0xdd, 0xd9, 0x80,
		0xc8, 0x16, 0x3f, 0x49, 0x73, 0xa8, 0xe0, 0x77,
		0xab, 0x94, 0xf1, 0x5f, 0x62, 0x0e, 0x8c, 0xf3,
		0x05, 0xeb, 0x5a, 0x25, 0x9e, 0x32, 0x27, 0xcc
	},
	{
		0xd7, 0x1d, 0x2d, 0xf8, 0x8e, 0xdb, 0x43, 0x85,
		0x60, 0xa6, 0xf6, 0x3f, 0xb9, 0x70, 0x1a, 0x43,
		0xa1, 0xc4, 0x92, 0x57, 0x38, 0x62, 0xe5, 0xbc,
		0x5b, 0x01, 0x0c, 0xea, 0xc4, 0x9e, 0x7f, 0x29,
		0x7a, 0x23, 0xb6, 0x1f, 0x49, 0xe0, 0x10, 0x76,
		0x9c, 0x4a, 0xcb, 0xa1, 0xe7, 0x8d, 0x2d, 0xd8,
		0x0f, 0xf9, 0x61, 0xc4, 0xa3, 0x95, 0xde, 0x0b,
		0xf5, 0x3c, 0x32, 0x57, 0x58, 0x62, 0x84, 0xbe
	}
};


static void doIp(unsigned char data[])
{
	unsigned char j, k;
	unsigned char val;
	unsigned char buf[8];
	unsigned char *p;
	unsigned char i = 8;
	get_random_bytes(buf, sizeof(buf));

	for(i=0; i<8; i++)
	{
		val = data[i];
		p = &buf[3];
		j = 4;

		do
		{
			for(k=0; k<=4; k+=4)
			{
				p[k] >>= 1;
				if(val & 1) { p[k] |= 0x80; }
				val >>= 1;
			}
			p--;
		}
		while(--j);
	}

	memcpy(data, buf, 8);
}

static void doIp_1(unsigned char data[])
{
	unsigned char j, k;
	unsigned char r = 0;
	unsigned char buf[8];
	unsigned char *p;
	unsigned char i = 8;

	for(i=0; i<8; i++)
	{
		p = &data[3];
		j = 4;

		do
		{
			for(k=0; k<=4; k+=4)
			{
				r >>= 1;
				if(p[k] & 1) { r |= 0x80; }
				p[k] >>= 1;
			}
			p--;
		}
		while(--j);
		buf[i] = r;
	}

	memcpy(data, buf, 8);
}

static void makeK(unsigned char *left, unsigned char *right, unsigned char *K)
{
	unsigned char i, j;
	unsigned char bit, val;
	unsigned char *p;

	for(i=0; i<8; i++)
	{
		val = 0;
		for(j=0; j<6; j++)
		{
			bit = PC2[i][j];
			if(bit < 29)
			{
				bit = 28-bit;
				p   = left;
			}
			else
			{
				bit = 56-bit;
				p   = right;
			}
			val <<= 1;
			if( p[bit >> 3] & (1 << (bit & 7)) ) { val |= 1; }
		}
		*K = val;
		K++;
	}
}

static void rightRot(unsigned char key[])
{
	unsigned char *p     = key;
	unsigned char  i     = 3;
	unsigned char  carry = 0;

	carry = 0;

	if(*p & 1) { carry = 0x08; }

	do {
		*p = (*p >> 1) | ((p[1] & 1) ? 0x80 : 0);
		p++;
	}
	while(--i);

	*p = (*p >> 1) | carry;
}

static void rightRotKeys(unsigned char left[], unsigned char right[])
{
	rightRot(left);
	rightRot(right);
}

static void leftRot(unsigned char key[])
{
	unsigned char i = 27;

	do {
		rightRot(key);
	}
	while(--i);
}

static void leftRotKeys(unsigned char left[], unsigned char right[])
{
	leftRot(left);
	leftRot(right);
}

static void desCore(unsigned char data[], unsigned char K[], unsigned char result[])
{
	unsigned char i, j;
	unsigned char bit, val;

	memset(result, 0, 4);

	for(i=0; i<8; i++)
	{
		val = 0;
		for(j=0; j<6; j++)
		{
			bit = 32-E[i][j];
			val <<= 1;
			if( data[3 - (bit >> 3)] & (1 << (bit & 7)) ) { val |= 1; }
		}
		val ^= K[i];
		val  = SBOXES[i & 3][val];
		if(i > 3)
		{
			val >>= 4;
		}
		val &= 0x0f;
		result[i >> 1] |= (i & 1) ? val : (val << 4);
	}
}

static void permut32(unsigned char data[])
{
	unsigned char i, j;
	unsigned char bit;
	unsigned char r[4] = {0}; // init to keep Valgrind happy
	unsigned char *p;

	for(i=0; i<32; i++)
	{
		bit = 32-P[i];
		p = r;
		for(j=0; j<3; j++)
		{
			*p = (*p << 1) | ((p[1] & 0x80) ? 1 : 0);
			p++;
		}
		*p <<= 1;
		if( data[3 - (bit >> 3)] & (1 << (bit & 7)) ) { *p |= 1; }
	}

	memcpy(data, r, 4);
}

static void swap(unsigned char left[], unsigned char right[])
{
	unsigned char x[4];

	memcpy(x, right, 4);
	memcpy(right, left, 4);
	memcpy(left, x, 4);
}

static void desRound(unsigned char left[], unsigned char right[], unsigned char data[], unsigned char mode, unsigned char k8)
{
	unsigned char i;
	unsigned char K[8];
	unsigned char r[4];
	unsigned char tempr[4];
	unsigned short temp;

	memcpy(tempr, data+4, 4);

	/* Viaccess */
	temp = (short)k8*(short)tempr[0]+(short)k8+(short)tempr[0];
	tempr[0] = (temp & 0xff) - ((temp>>8) & 0xff);
	if((temp & 0xff) - (temp>>8) < 0)
	{ tempr[0]++; }

	makeK(left, right, K);
	desCore(tempr, K, r);
	permut32(r);

	if(mode & DES_HASH)
	{
		i    = r[0];
		r[0] = r[1];
		r[1] = i;
	}

	for(i=0; i<4; i++)
	{
		*data ^= r[i];
		data++;
	}

	swap(data-4, data);
}

void nc_des(unsigned char key[], unsigned char mode, unsigned char data[])
{
	unsigned char i;
	unsigned char left[8];
	unsigned char right[8];
	unsigned char *p = left;

	short DESShift = (mode & DES_RIGHT) ? 0x8103 : 0xc081;

	for(i=3; i>0; i--)
	{
		*p = (key[i-1] << 4) | (key[i] >> 4);
		p++;
	}
	left[3] =  key[0] >> 4;
	right[0] = key[6];
	right[1] = key[5];
	right[2] = key[4];
	right[3] = key[3] & 0x0f;

	if(mode & DES_IP) { doIp(data); }

	do {
		if(!(mode & DES_RIGHT))
		{
			leftRotKeys(left, right);
			if(!(DESShift & 0x8000)) { leftRotKeys(left, right); }
		}
		desRound(left, right, data, mode, key[7]);

		if(mode & DES_RIGHT)
		{
			rightRotKeys(left, right);
			if(!(DESShift & 0x8000)) { rightRotKeys(left, right); }
		}
		DESShift <<= 1;
	}
	while(DESShift);

	swap(data, data+4);
	if(mode & DES_IP_1) { doIp_1(data); }

}
