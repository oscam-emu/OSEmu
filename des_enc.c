#include "des.h"

#define D_ENCRYPT(LL,R,S) {\
	LOAD_DATA_tmp(R,S,u,t,E0,E1); \
	t=ROTATE(t,4); \
	LL^=\
		DES_SPtrans[0][(u>> 2L)&0x3f]^ \
		DES_SPtrans[2][(u>>10L)&0x3f]^ \
		DES_SPtrans[4][(u>>18L)&0x3f]^ \
		DES_SPtrans[6][(u>>26L)&0x3f]^ \
		DES_SPtrans[1][(t>> 2L)&0x3f]^ \
		DES_SPtrans[3][(t>>10L)&0x3f]^ \
		DES_SPtrans[5][(t>>18L)&0x3f]^ \
		DES_SPtrans[7][(t>>26L)&0x3f]; }


#define LOAD_DATA_tmp(a,b,c,d,e,f) LOAD_DATA(a,b,c,d,e,f,g)
#define LOAD_DATA(R,S,u,t,E0,E1,tmp) \
	u=R^s[S  ]; \
	t=R^s[S+1]

#define	ROTATE(a,n)	(((a)>>(n))+((a)<<(32-(n))))

#define PERM_OP(a,b,t,n,m) ((t)=((((a)>>(n))^(b))&(m)),\
	(b)^=(t),\
	(a)^=((t)<<(n)))
	
#define IP(l,r) \
	{ \
	register DES_LONG tt; \
	PERM_OP(r,l,tt, 4,0x0f0f0f0fL); \
	PERM_OP(l,r,tt,16,0x0000ffffL); \
	PERM_OP(r,l,tt, 2,0x33333333L); \
	PERM_OP(l,r,tt, 8,0x00ff00ffL); \
	PERM_OP(r,l,tt, 1,0x55555555L); \
	}

#define FP(l,r) \
	{ \
	register DES_LONG tt; \
	PERM_OP(l,r,tt, 1,0x55555555L); \
	PERM_OP(r,l,tt, 8,0x00ff00ffL); \
	PERM_OP(l,r,tt, 2,0x33333333L); \
	PERM_OP(r,l,tt,16,0x0000ffffL); \
	PERM_OP(l,r,tt, 4,0x0f0f0f0fL); \
	}

#define c2l(c,l)	(l =((DES_LONG)(*((c)++)))    , \
			 l|=((DES_LONG)(*((c)++)))<< 8L, \
			 l|=((DES_LONG)(*((c)++)))<<16L, \
			 l|=((DES_LONG)(*((c)++)))<<24L)

#define l2c(l,c)	(*((c)++)=(unsigned char)(((l)     )&0xff), \
			 *((c)++)=(unsigned char)(((l)>> 8L)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>16L)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>24L)&0xff))
			 
#define c2ln(c,l1,l2,n)	{ \
			c+=n; \
			l1=l2=0; \
			switch (n) { \
			case 8: l2 =((DES_LONG)(*(--(c))))<<24L; \
			case 7: l2|=((DES_LONG)(*(--(c))))<<16L; \
			case 6: l2|=((DES_LONG)(*(--(c))))<< 8L; \
			case 5: l2|=((DES_LONG)(*(--(c))));     \
			case 4: l1 =((DES_LONG)(*(--(c))))<<24L; \
			case 3: l1|=((DES_LONG)(*(--(c))))<<16L; \
			case 2: l1|=((DES_LONG)(*(--(c))))<< 8L; \
			case 1: l1|=((DES_LONG)(*(--(c))));     \
				} \
			}
						 
#define l2cn(l1,l2,c,n)	{ \
			c+=n; \
			switch (n) { \
			case 8: *(--(c))=(unsigned char)(((l2)>>24L)&0xff); \
			case 7: *(--(c))=(unsigned char)(((l2)>>16L)&0xff); \
			case 6: *(--(c))=(unsigned char)(((l2)>> 8L)&0xff); \
			case 5: *(--(c))=(unsigned char)(((l2)     )&0xff); \
			case 4: *(--(c))=(unsigned char)(((l1)>>24L)&0xff); \
			case 3: *(--(c))=(unsigned char)(((l1)>>16L)&0xff); \
			case 2: *(--(c))=(unsigned char)(((l1)>> 8L)&0xff); \
			case 1: *(--(c))=(unsigned char)(((l1)     )&0xff); \
				} \
			}			 

const DES_LONG DES_SPtrans[8][64]={
{
/* nibble 0 */
0x02080800L, 0x00080000L, 0x02000002L, 0x02080802L,
0x02000000L, 0x00080802L, 0x00080002L, 0x02000002L,
0x00080802L, 0x02080800L, 0x02080000L, 0x00000802L,
0x02000802L, 0x02000000L, 0x00000000L, 0x00080002L,
0x00080000L, 0x00000002L, 0x02000800L, 0x00080800L,
0x02080802L, 0x02080000L, 0x00000802L, 0x02000800L,
0x00000002L, 0x00000800L, 0x00080800L, 0x02080002L,
0x00000800L, 0x02000802L, 0x02080002L, 0x00000000L,
0x00000000L, 0x02080802L, 0x02000800L, 0x00080002L,
0x02080800L, 0x00080000L, 0x00000802L, 0x02000800L,
0x02080002L, 0x00000800L, 0x00080800L, 0x02000002L,
0x00080802L, 0x00000002L, 0x02000002L, 0x02080000L,
0x02080802L, 0x00080800L, 0x02080000L, 0x02000802L,
0x02000000L, 0x00000802L, 0x00080002L, 0x00000000L,
0x00080000L, 0x02000000L, 0x02000802L, 0x02080800L,
0x00000002L, 0x02080002L, 0x00000800L, 0x00080802L,
},{
/* nibble 1 */
0x40108010L, 0x00000000L, 0x00108000L, 0x40100000L,
0x40000010L, 0x00008010L, 0x40008000L, 0x00108000L,
0x00008000L, 0x40100010L, 0x00000010L, 0x40008000L,
0x00100010L, 0x40108000L, 0x40100000L, 0x00000010L,
0x00100000L, 0x40008010L, 0x40100010L, 0x00008000L,
0x00108010L, 0x40000000L, 0x00000000L, 0x00100010L,
0x40008010L, 0x00108010L, 0x40108000L, 0x40000010L,
0x40000000L, 0x00100000L, 0x00008010L, 0x40108010L,
0x00100010L, 0x40108000L, 0x40008000L, 0x00108010L,
0x40108010L, 0x00100010L, 0x40000010L, 0x00000000L,
0x40000000L, 0x00008010L, 0x00100000L, 0x40100010L,
0x00008000L, 0x40000000L, 0x00108010L, 0x40008010L,
0x40108000L, 0x00008000L, 0x00000000L, 0x40000010L,
0x00000010L, 0x40108010L, 0x00108000L, 0x40100000L,
0x40100010L, 0x00100000L, 0x00008010L, 0x40008000L,
0x40008010L, 0x00000010L, 0x40100000L, 0x00108000L,
},{
/* nibble 2 */
0x04000001L, 0x04040100L, 0x00000100L, 0x04000101L,
0x00040001L, 0x04000000L, 0x04000101L, 0x00040100L,
0x04000100L, 0x00040000L, 0x04040000L, 0x00000001L,
0x04040101L, 0x00000101L, 0x00000001L, 0x04040001L,
0x00000000L, 0x00040001L, 0x04040100L, 0x00000100L,
0x00000101L, 0x04040101L, 0x00040000L, 0x04000001L,
0x04040001L, 0x04000100L, 0x00040101L, 0x04040000L,
0x00040100L, 0x00000000L, 0x04000000L, 0x00040101L,
0x04040100L, 0x00000100L, 0x00000001L, 0x00040000L,
0x00000101L, 0x00040001L, 0x04040000L, 0x04000101L,
0x00000000L, 0x04040100L, 0x00040100L, 0x04040001L,
0x00040001L, 0x04000000L, 0x04040101L, 0x00000001L,
0x00040101L, 0x04000001L, 0x04000000L, 0x04040101L,
0x00040000L, 0x04000100L, 0x04000101L, 0x00040100L,
0x04000100L, 0x00000000L, 0x04040001L, 0x00000101L,
0x04000001L, 0x00040101L, 0x00000100L, 0x04040000L,
},{
/* nibble 3 */
0x00401008L, 0x10001000L, 0x00000008L, 0x10401008L,
0x00000000L, 0x10400000L, 0x10001008L, 0x00400008L,
0x10401000L, 0x10000008L, 0x10000000L, 0x00001008L,
0x10000008L, 0x00401008L, 0x00400000L, 0x10000000L,
0x10400008L, 0x00401000L, 0x00001000L, 0x00000008L,
0x00401000L, 0x10001008L, 0x10400000L, 0x00001000L,
0x00001008L, 0x00000000L, 0x00400008L, 0x10401000L,
0x10001000L, 0x10400008L, 0x10401008L, 0x00400000L,
0x10400008L, 0x00001008L, 0x00400000L, 0x10000008L,
0x00401000L, 0x10001000L, 0x00000008L, 0x10400000L,
0x10001008L, 0x00000000L, 0x00001000L, 0x00400008L,
0x00000000L, 0x10400008L, 0x10401000L, 0x00001000L,
0x10000000L, 0x10401008L, 0x00401008L, 0x00400000L,
0x10401008L, 0x00000008L, 0x10001000L, 0x00401008L,
0x00400008L, 0x00401000L, 0x10400000L, 0x10001008L,
0x00001008L, 0x10000000L, 0x10000008L, 0x10401000L,
},{
/* nibble 4 */
0x08000000L, 0x00010000L, 0x00000400L, 0x08010420L,
0x08010020L, 0x08000400L, 0x00010420L, 0x08010000L,
0x00010000L, 0x00000020L, 0x08000020L, 0x00010400L,
0x08000420L, 0x08010020L, 0x08010400L, 0x00000000L,
0x00010400L, 0x08000000L, 0x00010020L, 0x00000420L,
0x08000400L, 0x00010420L, 0x00000000L, 0x08000020L,
0x00000020L, 0x08000420L, 0x08010420L, 0x00010020L,
0x08010000L, 0x00000400L, 0x00000420L, 0x08010400L,
0x08010400L, 0x08000420L, 0x00010020L, 0x08010000L,
0x00010000L, 0x00000020L, 0x08000020L, 0x08000400L,
0x08000000L, 0x00010400L, 0x08010420L, 0x00000000L,
0x00010420L, 0x08000000L, 0x00000400L, 0x00010020L,
0x08000420L, 0x00000400L, 0x00000000L, 0x08010420L,
0x08010020L, 0x08010400L, 0x00000420L, 0x00010000L,
0x00010400L, 0x08010020L, 0x08000400L, 0x00000420L,
0x00000020L, 0x00010420L, 0x08010000L, 0x08000020L,
},{
/* nibble 5 */
0x80000040L, 0x00200040L, 0x00000000L, 0x80202000L,
0x00200040L, 0x00002000L, 0x80002040L, 0x00200000L,
0x00002040L, 0x80202040L, 0x00202000L, 0x80000000L,
0x80002000L, 0x80000040L, 0x80200000L, 0x00202040L,
0x00200000L, 0x80002040L, 0x80200040L, 0x00000000L,
0x00002000L, 0x00000040L, 0x80202000L, 0x80200040L,
0x80202040L, 0x80200000L, 0x80000000L, 0x00002040L,
0x00000040L, 0x00202000L, 0x00202040L, 0x80002000L,
0x00002040L, 0x80000000L, 0x80002000L, 0x00202040L,
0x80202000L, 0x00200040L, 0x00000000L, 0x80002000L,
0x80000000L, 0x00002000L, 0x80200040L, 0x00200000L,
0x00200040L, 0x80202040L, 0x00202000L, 0x00000040L,
0x80202040L, 0x00202000L, 0x00200000L, 0x80002040L,
0x80000040L, 0x80200000L, 0x00202040L, 0x00000000L,
0x00002000L, 0x80000040L, 0x80002040L, 0x80202000L,
0x80200000L, 0x00002040L, 0x00000040L, 0x80200040L,
},{
/* nibble 6 */
0x00004000L, 0x00000200L, 0x01000200L, 0x01000004L,
0x01004204L, 0x00004004L, 0x00004200L, 0x00000000L,
0x01000000L, 0x01000204L, 0x00000204L, 0x01004000L,
0x00000004L, 0x01004200L, 0x01004000L, 0x00000204L,
0x01000204L, 0x00004000L, 0x00004004L, 0x01004204L,
0x00000000L, 0x01000200L, 0x01000004L, 0x00004200L,
0x01004004L, 0x00004204L, 0x01004200L, 0x00000004L,
0x00004204L, 0x01004004L, 0x00000200L, 0x01000000L,
0x00004204L, 0x01004000L, 0x01004004L, 0x00000204L,
0x00004000L, 0x00000200L, 0x01000000L, 0x01004004L,
0x01000204L, 0x00004204L, 0x00004200L, 0x00000000L,
0x00000200L, 0x01000004L, 0x00000004L, 0x01000200L,
0x00000000L, 0x01000204L, 0x01000200L, 0x00004200L,
0x00000204L, 0x00004000L, 0x01004204L, 0x01000000L,
0x01004200L, 0x00000004L, 0x00004004L, 0x01004204L,
0x01000004L, 0x01004200L, 0x01004000L, 0x00004004L,
},{
/* nibble 7 */
0x20800080L, 0x20820000L, 0x00020080L, 0x00000000L,
0x20020000L, 0x00800080L, 0x20800000L, 0x20820080L,
0x00000080L, 0x20000000L, 0x00820000L, 0x00020080L,
0x00820080L, 0x20020080L, 0x20000080L, 0x20800000L,
0x00020000L, 0x00820080L, 0x00800080L, 0x20020000L,
0x20820080L, 0x20000080L, 0x00000000L, 0x00820000L,
0x20000000L, 0x00800000L, 0x20020080L, 0x20800080L,
0x00800000L, 0x00020000L, 0x20820000L, 0x00000080L,
0x00800000L, 0x00020000L, 0x20000080L, 0x20820080L,
0x00020080L, 0x20000000L, 0x00000000L, 0x00820000L,
0x20800080L, 0x20020080L, 0x20020000L, 0x00800080L,
0x20820000L, 0x00000080L, 0x00800080L, 0x20020000L,
0x20820080L, 0x00800000L, 0x20800000L, 0x20000080L,
0x00820000L, 0x00020080L, 0x20020080L, 0x20800000L,
0x00000080L, 0x20820000L, 0x00820080L, 0x00000000L,
0x20000000L, 0x20800080L, 0x00020000L, 0x00820080L,
}};


void DES_encrypt1(DES_LONG *data, DES_key_schedule *ks, int enc)
	{
	register DES_LONG l,r,t,u;
#ifdef DES_PTR
	register const unsigned char *des_SP=(const unsigned char *)DES_SPtrans;
#endif
#ifndef DES_UNROLL
	register int i;
#endif
	register DES_LONG *s;

	r=data[0];
	l=data[1];

	IP(r,l);
	/* Things have been modified so that the initial rotate is
	 * done outside the loop.  This required the
	 * DES_SPtrans values in sp.h to be rotated 1 bit to the right.
	 * One perl script later and things have a 5% speed up on a sparc2.
	 * Thanks to Richard Outerbridge <71755.204@CompuServe.COM>
	 * for pointing this out. */
	/* clear the top bits on machines with 8byte longs */
	/* shift left by 2 */
	r=ROTATE(r,29)&0xffffffffL;
	l=ROTATE(l,29)&0xffffffffL;

	s=ks->ks->deslong;
	/* I don't know if it is worth the effort of loop unrolling the
	 * inner loop */
	if (enc)
		{
#ifdef DES_UNROLL
		D_ENCRYPT(l,r, 0); /*  1 */
		D_ENCRYPT(r,l, 2); /*  2 */
		D_ENCRYPT(l,r, 4); /*  3 */
		D_ENCRYPT(r,l, 6); /*  4 */
		D_ENCRYPT(l,r, 8); /*  5 */
		D_ENCRYPT(r,l,10); /*  6 */
		D_ENCRYPT(l,r,12); /*  7 */
		D_ENCRYPT(r,l,14); /*  8 */
		D_ENCRYPT(l,r,16); /*  9 */
		D_ENCRYPT(r,l,18); /*  10 */
		D_ENCRYPT(l,r,20); /*  11 */
		D_ENCRYPT(r,l,22); /*  12 */
		D_ENCRYPT(l,r,24); /*  13 */
		D_ENCRYPT(r,l,26); /*  14 */
		D_ENCRYPT(l,r,28); /*  15 */
		D_ENCRYPT(r,l,30); /*  16 */
#else
		for (i=0; i<32; i+=4)
			{
			D_ENCRYPT(l,r,i+0); /*  1 */
			D_ENCRYPT(r,l,i+2); /*  2 */
			}
#endif
		}
	else
		{
#ifdef DES_UNROLL
		D_ENCRYPT(l,r,30); /* 16 */
		D_ENCRYPT(r,l,28); /* 15 */
		D_ENCRYPT(l,r,26); /* 14 */
		D_ENCRYPT(r,l,24); /* 13 */
		D_ENCRYPT(l,r,22); /* 12 */
		D_ENCRYPT(r,l,20); /* 11 */
		D_ENCRYPT(l,r,18); /* 10 */
		D_ENCRYPT(r,l,16); /*  9 */
		D_ENCRYPT(l,r,14); /*  8 */
		D_ENCRYPT(r,l,12); /*  7 */
		D_ENCRYPT(l,r,10); /*  6 */
		D_ENCRYPT(r,l, 8); /*  5 */
		D_ENCRYPT(l,r, 6); /*  4 */
		D_ENCRYPT(r,l, 4); /*  3 */
		D_ENCRYPT(l,r, 2); /*  2 */
		D_ENCRYPT(r,l, 0); /*  1 */
#else
		for (i=30; i>0; i-=4)
			{
			D_ENCRYPT(l,r,i-0); /* 16 */
			D_ENCRYPT(r,l,i-2); /* 15 */
			}
#endif
		}

	/* rotate and clear the top bits on machines with 8byte longs */
	l=ROTATE(l,3)&0xffffffffL;
	r=ROTATE(r,3)&0xffffffffL;

	FP(r,l);
	data[0]=l;
	data[1]=r;
	l=r=t=u=0;
	}

void DES_encrypt2(DES_LONG *data, DES_key_schedule *ks, int enc)
	{
	register DES_LONG l,r,t,u;
#ifdef DES_PTR
	register const unsigned char *des_SP=(const unsigned char *)DES_SPtrans;
#endif
#ifndef DES_UNROLL
	register int i;
#endif
	register DES_LONG *s;

	r=data[0];
	l=data[1];

	/* Things have been modified so that the initial rotate is
	 * done outside the loop.  This required the
	 * DES_SPtrans values in sp.h to be rotated 1 bit to the right.
	 * One perl script later and things have a 5% speed up on a sparc2.
	 * Thanks to Richard Outerbridge <71755.204@CompuServe.COM>
	 * for pointing this out. */
	/* clear the top bits on machines with 8byte longs */
	r=ROTATE(r,29)&0xffffffffL;
	l=ROTATE(l,29)&0xffffffffL;

	s=ks->ks->deslong;
	/* I don't know if it is worth the effort of loop unrolling the
	 * inner loop */
	if (enc)
		{
#ifdef DES_UNROLL
		D_ENCRYPT(l,r, 0); /*  1 */
		D_ENCRYPT(r,l, 2); /*  2 */
		D_ENCRYPT(l,r, 4); /*  3 */
		D_ENCRYPT(r,l, 6); /*  4 */
		D_ENCRYPT(l,r, 8); /*  5 */
		D_ENCRYPT(r,l,10); /*  6 */
		D_ENCRYPT(l,r,12); /*  7 */
		D_ENCRYPT(r,l,14); /*  8 */
		D_ENCRYPT(l,r,16); /*  9 */
		D_ENCRYPT(r,l,18); /*  10 */
		D_ENCRYPT(l,r,20); /*  11 */
		D_ENCRYPT(r,l,22); /*  12 */
		D_ENCRYPT(l,r,24); /*  13 */
		D_ENCRYPT(r,l,26); /*  14 */
		D_ENCRYPT(l,r,28); /*  15 */
		D_ENCRYPT(r,l,30); /*  16 */
#else
		for (i=0; i<32; i+=4)
			{
			D_ENCRYPT(l,r,i+0); /*  1 */
			D_ENCRYPT(r,l,i+2); /*  2 */
			}
#endif
		}
	else
		{
#ifdef DES_UNROLL
		D_ENCRYPT(l,r,30); /* 16 */
		D_ENCRYPT(r,l,28); /* 15 */
		D_ENCRYPT(l,r,26); /* 14 */
		D_ENCRYPT(r,l,24); /* 13 */
		D_ENCRYPT(l,r,22); /* 12 */
		D_ENCRYPT(r,l,20); /* 11 */
		D_ENCRYPT(l,r,18); /* 10 */
		D_ENCRYPT(r,l,16); /*  9 */
		D_ENCRYPT(l,r,14); /*  8 */
		D_ENCRYPT(r,l,12); /*  7 */
		D_ENCRYPT(l,r,10); /*  6 */
		D_ENCRYPT(r,l, 8); /*  5 */
		D_ENCRYPT(l,r, 6); /*  4 */
		D_ENCRYPT(r,l, 4); /*  3 */
		D_ENCRYPT(l,r, 2); /*  2 */
		D_ENCRYPT(r,l, 0); /*  1 */
#else
		for (i=30; i>0; i-=4)
			{
			D_ENCRYPT(l,r,i-0); /* 16 */
			D_ENCRYPT(r,l,i-2); /* 15 */
			}
#endif
		}
	/* rotate and clear the top bits on machines with 8byte longs */
	data[0]=ROTATE(l,3)&0xffffffffL;
	data[1]=ROTATE(r,3)&0xffffffffL;
	l=r=t=u=0;
	}

void DES_encrypt3(DES_LONG *data, DES_key_schedule *ks1,
		  DES_key_schedule *ks2, DES_key_schedule *ks3)
	{
	register DES_LONG l,r;

	l=data[0];
	r=data[1];
	IP(l,r);
	data[0]=l;
	data[1]=r;
	DES_encrypt2((DES_LONG *)data,ks1,DES_ENCRYPT);
	DES_encrypt2((DES_LONG *)data,ks2,DES_DECRYPT);
	DES_encrypt2((DES_LONG *)data,ks3,DES_ENCRYPT);
	l=data[0];
	r=data[1];
	FP(r,l);
	data[0]=l;
	data[1]=r;
	}

void DES_decrypt3(DES_LONG *data, DES_key_schedule *ks1,
		  DES_key_schedule *ks2, DES_key_schedule *ks3)
	{
	register DES_LONG l,r;

	l=data[0];
	r=data[1];
	IP(l,r);
	data[0]=l;
	data[1]=r;
	DES_encrypt2((DES_LONG *)data,ks3,DES_DECRYPT);
	DES_encrypt2((DES_LONG *)data,ks2,DES_ENCRYPT);
	DES_encrypt2((DES_LONG *)data,ks1,DES_DECRYPT);
	l=data[0];
	r=data[1];
	FP(r,l);
	data[0]=l;
	data[1]=r;
	}


void DES_ede3_cbc_encrypt(const unsigned char *input, unsigned char *output,
			  long length, DES_key_schedule *ks1,
			  DES_key_schedule *ks2, DES_key_schedule *ks3,
			  DES_cblock *ivec, int enc)
	{
	register DES_LONG tin0,tin1;
	register DES_LONG tout0,tout1,xor0,xor1;
	register const unsigned char *in;
	unsigned char *out;
	register long l=length;
	DES_LONG tin[2];
	unsigned char *iv;

	in=input;
	out=output;
	iv = &(*ivec)[0];

	if (enc)
		{
		c2l(iv,tout0);
		c2l(iv,tout1);
		for (l-=8; l>=0; l-=8)
			{
			c2l(in,tin0);
			c2l(in,tin1);
			tin0^=tout0;
			tin1^=tout1;

			tin[0]=tin0;
			tin[1]=tin1;
			DES_encrypt3((DES_LONG *)tin,ks1,ks2,ks3);
			tout0=tin[0];
			tout1=tin[1];

			l2c(tout0,out);
			l2c(tout1,out);
			}
		if (l != -8)
			{
			c2ln(in,tin0,tin1,l+8);
			tin0^=tout0;
			tin1^=tout1;

			tin[0]=tin0;
			tin[1]=tin1;
			DES_encrypt3((DES_LONG *)tin,ks1,ks2,ks3);
			tout0=tin[0];
			tout1=tin[1];

			l2c(tout0,out);
			l2c(tout1,out);
			}
		iv = &(*ivec)[0];
		l2c(tout0,iv);
		l2c(tout1,iv);
		}
	else
		{
		register DES_LONG t0,t1;

		c2l(iv,xor0);
		c2l(iv,xor1);
		for (l-=8; l>=0; l-=8)
			{
			c2l(in,tin0);
			c2l(in,tin1);

			t0=tin0;
			t1=tin1;

			tin[0]=tin0;
			tin[1]=tin1;
			DES_decrypt3((DES_LONG *)tin,ks1,ks2,ks3);
			tout0=tin[0];
			tout1=tin[1];

			tout0^=xor0;
			tout1^=xor1;
			l2c(tout0,out);
			l2c(tout1,out);
			xor0=t0;
			xor1=t1;
			}
		if (l != -8)
			{
			c2l(in,tin0);
			c2l(in,tin1);
			
			t0=tin0;
			t1=tin1;

			tin[0]=tin0;
			tin[1]=tin1;
			DES_decrypt3((DES_LONG *)tin,ks1,ks2,ks3);
			tout0=tin[0];
			tout1=tin[1];
		
			tout0^=xor0;
			tout1^=xor1;
			l2cn(tout0,tout1,out,l+8);
			xor0=t0;
			xor1=t1;
			}

		iv = &(*ivec)[0];
		l2c(xor0,iv);
		l2c(xor1,iv);
		}
	tin0=tin1=tout0=tout1=xor0=xor1=0;
	tin[0]=tin[1]=0;
	}

