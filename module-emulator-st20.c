// taken from: https://github.com/falsovsky/kaffeinesc/blob/master/src/mgcam
/*
 * Softcam plugin to VDR (C++)
 *
 * This code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * Or, point your browser to http://www.gnu.org/copyleft/gpl.html
 */

#include "globals.h"
#include "module-emulator-st20.h"

#define ST20_SAVE_DEBUG 0

#define IPTR 0
#define WPTR 1
#define AREG 2
#define BREG 3
#define CREG 4


//#define FLASHS	0x7FE00000
//#define FLASHE	0x7FFFFFFF
#define FLASHS	0x80000000
#define FLASHE	0x8FFFFFFF

#define RAMS	0x40000000
#define RAME	0x401FFFFF

//#define IRAMS	0x80000000
//#define IRAME	0x800017FF
#define IRAMS	0x90000000
#define IRAME	0x900017FF


#define ERR_ILL_OP -1
#define ERR_CNT    -2

#define STACKMAX  16
#define STACKMASK (STACKMAX-1)

#define INVALID_VALUE 	0xCCCCCCCC
#define ERRORVAL	0xDEADBEEF

#define MININT		0x7FFFFFFF
#define MOSTPOS		0x7FFFFFFF
#define MOSTNEG		0x80000000

#ifndef ST20_SAVE_DEBUG
#define POP() stack[(sptr++)&STACKMASK]
#else
#define POP() ({ int __t=stack[sptr&STACKMASK]; stack[(sptr++)&STACKMASK]=INVALID_VALUE; __t; })
#endif
#define PUSH(v) do { int __v=(v); stack[(--sptr)&STACKMASK]=__v; } while(0)
#define DROP(n) sptr+=n

#define AAA stack[sptr&STACKMASK]
#define BBB stack[(sptr+1)&STACKMASK]
#define CCC stack[(sptr+2)&STACKMASK]

#define GET_OP()        operand|=op1&0x0F
#define CLEAR_OP()      operand=0
#define JUMP(x)         Iptr+=(x)
//#define POP64()         ({ unsigned int __b=POP(); ((unsigned long long)POP()<<32)|__b; })
#define PUSHPOP(op,val) do { int __a=val; AAA op##= (__a); } while(0)

#ifndef ST20_SAVE_DEBUG
#define RB(off) UINT8_LE(st20_Addr(off))
#define RW(off) UINT32_LE(st20_Addr(off))
#define WW(off,val) BYTE4_LE(st20_Addr(off),val)
#else
#define RB(off) st20_ReadByte(off)
#define RW(off) st20_ReadWord(off)
#define WW(off,val) st20_WriteWord(off,val)
#endif

#ifdef ST20_SAVE_DEBUG
#define LOG_OP(op) st20_LogOp(op)
#else
#define LOG_OP(op)
#endif

//
static unsigned int Iptr = 0, Wptr = 0;
static unsigned char *flash = NULL, *ram = NULL;
static unsigned int flashSize = 0, ramSize = 0;
static int sptr = 0, stack[STACKMAX];
static unsigned char iram[0x1800];

#ifndef ST20_SAVE_DEBUG
static int invalid = 0;
#endif
//

static unsigned int UINT32_LE(unsigned char *addr)
{
	return (addr[3]<<24) | (addr[2]<<16) | (addr[1]<<8) | addr[0];
}
static unsigned int UINT16_LE(unsigned char *addr)
{
	return (addr[1]<<8) | addr[0];
}
static unsigned int UINT8_LE(unsigned char *addr)
{
	return addr[0];
}
static void BYTE4_LE(unsigned char *addr, int value)
{
	addr[0] = value;
	addr[1] = value>>8;
	addr[2] = value>>16;
	addr[3] = value>>24;
}
/*static void BYTE2_LE(unsigned char *addr, int value)
{
	addr[0] = value;
	addr[1] = value>>8;
}*/
static void BYTE1_LE(unsigned char *addr, int value)
{
	addr[0] = value;
}

// -- cST20 --------------------------------------------------------------------

static void st20_SetFlash(unsigned char *m, int len)
{
	if(flash)
	{
		free(flash);
	}

	flash = (unsigned char*)malloc(len);
	if(!flash)
	{
		return;
	}

	if(m)
	{
		memcpy(flash,m,len);
	}
	else
	{
		memset(flash,0,len);
	}

	flashSize=len;
}

static void st20_SetRam(unsigned char *m, int len)
{
	if(ram)
	{
		free(ram);
	}

	ram = (unsigned char*)malloc(len);
	if(!ram)
	{
		return;
	}

	if(m)
	{
		memcpy(ram,m,len);
	}
	else
	{
		memset(ram,0,len);
	}

	ramSize=len;
}

static void st20_Init(unsigned int IPtr, unsigned int WPtr)
{
	Wptr=WPtr;
	Iptr=IPtr;
	memset(stack,INVALID_VALUE,sizeof(stack));
	sptr=STACKMAX-3;
	memset(iram,0,sizeof(iram));
}

static void st20_SetReg(int reg, unsigned int val)
{
	switch(reg) {
	case IPTR:
		Iptr=val;
		return;
	case WPTR:
		Wptr=val;
		return;
	case AREG:
		AAA=val;
		return;
	case BREG:
		BBB=val;
		return;
	case CREG:
		CCC=val;
		return;
	default:
		return;
	}
}

/*static unsigned int st20_GetReg(int reg)
{
	switch(reg) {
	case IPTR:
		return Iptr;
	case WPTR:
		return Wptr;
	case AREG:
		return AAA;
	case BREG:
		return BBB;
	case CREG:
		return CCC;
	default:
		return ERRORVAL;
	}
}*/

static unsigned char *st20_Addr(unsigned int off)
{
	switch(off) {
	case FLASHS ... FLASHE:
#ifndef ST20_SAVE_DEBUG
		return &flash[off-FLASHS];
#else
		off-=FLASHS;
		if(off<flashSize && flash) { return &flash[off]; }
		break;
#endif
	case RAMS ... RAME:
#ifndef ST20_SAVE_DEBUG
		return &ram[off-RAMS];
#else
		off-=RAMS;
		if(off<ramSize && ram) { return &ram[off]; }
		break;
#endif
	case IRAMS ... IRAME:
		return &iram[off-IRAMS];
	}
#ifndef ST20_SAVE_DEBUG
	invalid=ERRORVAL;
	return (unsigned char *)&invalid;
#else
	return 0;
#endif
}

#ifdef ST20_SAVE_DEBUG
static unsigned int st20_ReadWord(unsigned int off)
{
#ifndef ST20_SAVE_DEBUG
	return UINT32_LE(st20_Addr(off));
#else
	unsigned char *addr=st20_Addr(off);
	return addr ? UINT32_LE(addr) : ERRORVAL;
#endif
}
#endif

static unsigned short st20_ReadShort(unsigned int off)
{
#ifndef ST20_SAVE_DEBUG
	return UINT16_LE(st20_Addr(off));
#else
	unsigned char *addr=st20_Addr(off);
	return addr ? UINT16_LE(addr) : ERRORVAL>>16;
#endif
}

static unsigned char st20_ReadByte(unsigned int off)
{
#ifndef ST20_SAVE_DEBUG
	return UINT8_LE(st20_Addr(off));
#else
	unsigned char *addr=st20_Addr(off);
	return addr ? UINT8_LE(addr) : ERRORVAL>>24;
#endif
}

static void st20_WriteWord(unsigned int off, unsigned int val)
{
#ifndef ST20_SAVE_DEBUG
	BYTE4_LE(st20_Addr(off),val);
#else
	unsigned char *addr=st20_Addr(off);
	if(addr) { BYTE4_LE(addr,val); }
#endif
}

/*static void st20_WriteShort(unsigned int off, unsigned short val)
{
#ifndef ST20_SAVE_DEBUG
	BYTE2_LE(st20_Addr(off),val);
#else
	unsigned char *addr=st20_Addr(off);
	if(addr) { BYTE2_LE(addr,val); }
#endif
}*/

static void st20_WriteByte(unsigned int off, unsigned char val)
{
#ifndef ST20_SAVE_DEBUG
	BYTE1_LE(st20_Addr(off),val);
#else
	unsigned char *addr=st20_Addr(off);
	if(addr) { BYTE1_LE(addr,val); }
#endif
}

static void st20_SetCallFrame(unsigned int raddr, int p1, int p2, int p3)
{
	Wptr-=16;
	st20_WriteWord(Wptr,raddr); // RET
	st20_WriteWord(Wptr+4,0);
	st20_WriteWord(Wptr+8,0);
	st20_WriteWord(Wptr+12,p1);
	st20_WriteWord(Wptr+16,p2);
	st20_WriteWord(Wptr+20,p3);
	st20_SetReg(AREG,raddr);    // RET
}

#define OP_COL 20

#ifdef ST20_SAVE_DEBUG
static void st20_LogOpOper(int op, int oper)
{
	static const char *cmds[] = { "j","ldlp",0,"ldnl","ldc","ldnlp",0,"ldl","adc","call","cj","ajw","eqc","stl","stnl",0 };
	static const char flags[] = {  3,  0,    0, 0,     1,    0,     0, 0,    1,    3,     3,   0,    1,    0,    0,    0 };
	oper|=op&0xF;
	op>>=4;
	if(!cmds[op]) { return; }
	if(flags[op]&2) { oper+=Iptr; }

	cs_log("[Emu] %s %X", cmds[op], oper);
}

static void st20_LogOp(char *op)
{
	cs_log("[Emu] %s", op);
}
#endif

static unsigned int st20_bitrevnbits(unsigned int data, unsigned int nbBits)
{
	unsigned int i;
	unsigned int tmp;

	tmp = 0;
	for(i=0; i<nbBits; i++)
	{
		tmp <<= 1;
		tmp |= data & 1;
		data >>= 1;
	}

	return tmp;
}

static int st20_Decode(int count)
{
	int operand;
	CLEAR_OP();
	while(Iptr!=0) {
		int a, b, c, op1=RB(Iptr++);
		unsigned int tmp;
#ifdef ST20_SAVE_DEBUG
		st20_LogOpOper(op1,operand);
#endif
		GET_OP();
		switch(op1>>4) {
		case 0x0: // j / jump
#ifdef ST20_SAVE_DEBUG
			POP();
			POP();
			POP();
#endif
			JUMP(operand);
			CLEAR_OP();
			break;
		case 0x1: // ldlp
			PUSH(Wptr+(operand*4));
			CLEAR_OP();
			break;
		case 0x2: // positive prefix
			operand<<=4;
			break;
		case 0x3: // ldnl
			AAA=RW(AAA+(operand*4));
			CLEAR_OP();
			break;
		case 0x4: // ldc
			PUSH(operand);
			CLEAR_OP();
			break;
		case 0x5: // ldnlp
			PUSHPOP(+,operand*4);
			CLEAR_OP();
			break;
		case 0x6: // negative prefix
			operand=(~operand)<<4;
			break;
		case 0x7: // ldl
			PUSH(RW(Wptr+(operand*4)));
			CLEAR_OP();
			break;
		case 0x8: // adc
			PUSHPOP(+,operand);
			CLEAR_OP();
			break;
		case 0x9: // call
			Wptr-=16;
			WW(Wptr,Iptr);
			WW(Wptr+4,POP());
			WW(Wptr+8,POP());
			WW(Wptr+12,POP());
			PUSH(Iptr);
			JUMP(operand);
			CLEAR_OP();
			break;
		case 0xA: // cj / conditional jump
			if(AAA) { DROP(1); }
			else { JUMP(operand); }
			CLEAR_OP();
			break;
		case 0xB: // ajw / adjust workspace
			Wptr+=operand*4;
			CLEAR_OP();
			break;
		case 0xC: // eqc / equals constant
			AAA=(operand==AAA ? 1 : 0);
			CLEAR_OP();
			break;
		case 0xD: // stl
			WW(Wptr+(operand*4),POP());
			CLEAR_OP();
			break;
		case 0xE: // stnl
			a=POP();
			WW(a+(operand*4),POP());
			CLEAR_OP();
			break;
		case 0xF: // opr (secondary ins)
			switch(operand) {
			case  0x00:
				LOG_OP("rev");
				a=AAA;
				AAA=BBB;
				BBB=a;
				break;
			case  0x01:
				LOG_OP("lb");
				AAA=RB(AAA);
				break;
			case  0x02:
				LOG_OP("bsub");
				PUSHPOP(+,POP());
				break;
			case  0x04:
				LOG_OP("diff");
				PUSHPOP(-,POP());
				break;
			case  0x05:
				LOG_OP("add");
				PUSHPOP(+,POP());
				break;
			case  0x06:
				LOG_OP("gcall");
				a=AAA;
				AAA=Iptr;
				Iptr=a;
				break;
			case  0x08:
				LOG_OP("prod");
				PUSHPOP(*,POP());
				break;
			case  0x09:
				LOG_OP("gt");
				a=POP();
				AAA=(AAA>a);
				break;
			case  0x0A:
				LOG_OP("wsub");
				a=POP();
				AAA=a+(AAA*4);
				break;
			case  0x0C:
				LOG_OP("sub");
				PUSHPOP(-,POP());
				break;

			case  0x1B:
				LOG_OP("ldpi");
				PUSHPOP(+,Iptr);
				break;
			case  0x20:
				LOG_OP("ret");
				Iptr=RW(Wptr);
				Wptr=Wptr+16;
				break;
			case  0x2C:
				LOG_OP("div");
				PUSHPOP(/,POP());
				break;
			case  0x32:
				LOG_OP("not");
				AAA=~AAA;
				break;
			case  0x33:
				LOG_OP("xor");
				PUSHPOP(^,POP());
				break;
			case  0x34:
				LOG_OP("bcnt");
				PUSHPOP(*,4);
				break;
			case  0x3B:
				LOG_OP("sb");
				a=POP();
				st20_WriteByte(a,POP());
				break;
			case  0x3F:
				LOG_OP("wcnt");
				a=POP();
				PUSH(a&3);
				PUSH((unsigned int)a>>2);
				break;
			case  0x40:
				LOG_OP("shr");
				a=POP();
				AAA=(unsigned int)AAA>>a;
				break;
			case  0x41:
				LOG_OP("shl");
				a=POP();
				AAA=(unsigned int)AAA<<a;
				break;
			case  0x42:
				LOG_OP("mint");
				PUSH(MOSTNEG);
				break;
			case  0x46:
				LOG_OP("and");
				PUSHPOP(&,POP());
				break;
			case  0x4A:
				LOG_OP("move");
				a=POP();
				b=POP();
				c=POP();
				while(a--) { st20_WriteByte(b++,st20_ReadByte(c++)); }
				break;
			case  0x4B:
				LOG_OP("or");
				PUSHPOP(|,POP());
				break;
			case  0x53:
				LOG_OP("mul");
				PUSHPOP(*,POP());
				break;
			case  0x5A:
				LOG_OP("dup");
				PUSH(AAA);
				break;
			case  0x5F:
				LOG_OP("gtu");
				a=POP();
				AAA=((unsigned int)AAA>(unsigned int)a);
				break;
			case  0x1D:
				LOG_OP("xdble");
				CCC=BBB;
				BBB=(AAA>=0 ? 0:-1);
				break;
			case  0x1A:
				LOG_OP("ldiv");
				a=POP();
				b=POP();
				c=POP();
				tmp=b;
				PUSH(tmp%((unsigned int)a));
				PUSH(tmp/((unsigned int)a));
				break;
			case  0x1F:
				LOG_OP("rem");
				PUSHPOP(%,POP());
				break;
			case  0x35:
				LOG_OP("lshr");
				a=POP();
				b=POP();
				c=POP();
				if (a>32) { PUSH(0); PUSH(c>>(32-a));}
				else { PUSH(c>>a); PUSH((b>>a) + (c<<(32-a))); }
				break;
			case  0x78:
				LOG_OP("bitrevnbits");
				a=POP();
				AAA=st20_bitrevnbits(AAA,a);
				break;
			case  0xCA:
				LOG_OP("ls");
				AAA=st20_ReadShort(AAA);
				break;
			case  0xCD:
				LOG_OP("gintdis");
				break;
			case  0xCE:
				LOG_OP("gintenb");
				break;

			case -0x40:
				LOG_OP("nop");
				break;

			default:
				cs_log("[Emu] warning: unknown st20 opcode");
				return ERR_ILL_OP;
			}
			CLEAR_OP();
			break;
		}
		if(--count<=0 && operand==0) {
			return ERR_CNT;
		}
	}
	return 0;
}

int8_t st20_run(uint8_t* codeData, uint32_t codeSize, uint8_t* instructionPointer, uint8_t* data, uint32_t dataSize)
{
	static uint8_t init = 0;
	int32_t error;
	uint32_t vInstructionPointer, vDataPointer;

	if(!init)
	{		
		st20_SetRam(NULL, 0x2000);
		init = 1;
	}

	if(instructionPointer)
	{
#define MEMORY_GAP_SIZE 0x4000

		st20_SetFlash(NULL, MEMORY_GAP_SIZE+codeSize+0x10+dataSize);
		memcpy(flash+MEMORY_GAP_SIZE, codeData, codeSize);
		memcpy(flash+MEMORY_GAP_SIZE+codeSize+0x10, data, dataSize);
		
		vInstructionPointer = FLASHS + MEMORY_GAP_SIZE + (uint32_t)((instructionPointer - codeData));
		vDataPointer = FLASHS + MEMORY_GAP_SIZE + codeSize + 0x10;
		
		st20_Init(vInstructionPointer, RAMS+0x1000);
		st20_SetCallFrame(0, vDataPointer, 0, 0);

		error = st20_Decode(40000);

		if(error < 0) {
			cs_log("[Emu] st20 processing failed with error %d", error);
			return 0;
		}

		return 1;
	}

	return 0;
}
