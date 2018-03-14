/*******************************************************************************
	mf16emulib.h
	interface definition for mf16emulib.c
 ******************************************************************************/

#ifndef MF16EMULIB_H
#define MF16EMULIB_H

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<errno.h>

// register write control signal
#define RS		1
#define RT		2
#define RD		3
#define RA		4

// some values for different control signals
#define NO		0
#define YES		1
#define REG		2
#define PC		3
#define MEM		4
#define ALU		5
#define IMM		6
#define PCP1	7

// instructions
#define add		0
#define and		1
#define beq		2
#define jr		3
#define jrl		4
#define lui		5
#define lw		6
#define bsr		7
#define nor		8
#define or		9
#define ori		10
#define sll		11
#define slt		12
#define xor		13
#define srl		14
#define sw		15


#define REG_IOCONTROL	0xFF00
#define REG_IOBUFFER1	0xFF04
#define SIGNMASK	0x8000
#define SIGNKILL	0x7FFF
#define ALIGNMASK	0xFFFE
#define MASK16		0xFFFF

// some mf16 constants
#define WORDSIZE	16	// word size in bits
#define IROUTPUTS	4	// number of instruction register outputs
#define MAXTOKENLEN	512	// maximum length of any token i.e. arg, operation, directive, data, etc.
#define MAXLINELEN	1024	// maximum line length in assembly code
#define LINEFMTTYPES	7	// number of line format types
#define MAXTOKEN	2	// max tokens in a line
#define REGCOUNT	16	// total number of registers
#define OPERCOUNT	16	// total number of operations

// MIF file data section format
// for sscanf consumtpion
#define DATAFMT		" %x : %x "

// colored register print formats
#define REDHEX		"\e[41m%-5X\e[0m"
#define REDSTR		"\e[41m%-5s\e[0m"

// PC
typedef struct {
	int		value;		// current stored value
	int		out;
	int		*in;
	int		*enableWrite;	
} pc_t;

// Instruction register
typedef struct {
	int		value;
	int		out[IROUTPUTS];
	int		*in;
	int		*enableWrite;
} ir_t;

// Memory
typedef struct {
	int		*data;
	int		out;
	int		*writeData;
	int		*addr;
	int		*enableWrite;
} memory_t;

// register file
typedef struct {
	int		data[REGCOUNT];
	int		rsData;
	int		rtData;
	int		rdData;
	int		*rs;
	int		*rt;
	int		*rd;
	int		*writeData;
	int		*writeReg;
} regfile_t;

// ALU
typedef struct {
	int		result;
	int		jump_flag;
	int		*in[2];
	int		*aluOp;
} alu_t;

// generic device with 2 input and 1 output
typedef struct {
	int		out;
	int		*in[2];
} in2out1_t;

// 2:1 mux
typedef struct {
	int		out;
	int		*in[2];
	int		*sel;
} mux21_t;

// 4:1 mux
typedef struct {
	int		out;
	int		*in[4];
	int		*sel;
} mux41_t;

// sequencer
typedef struct {
	int		writePC;
	int		memAddrSrc;
	int		writeMem;
	int		writeIr;
	int		doBranch;
	int		regFileWriteSrc;
	int		writeReg;
	int		aluIn1;
	int		aluOp;
	int		*opcode;
} sequencer_t;

// CPU made of all components in the diagram
typedef struct {
	pc_t		pc;
	ir_t		ir;
	memory_t	mem;
	regfile_t	regFile;
	alu_t		alu;
	in2out1_t	branchAnd;
	in2out1_t	pcAdder;
	in2out1_t	zeroExt;
	mux21_t		memAddrMux;
	mux21_t		pcMux;
	mux21_t		aluMux1;
	mux41_t		regWriteDataMux;
	sequencer_t	seq;	
} cpu_t;

typedef void (*instrFunc)(cpu_t*);	// hardware instruction execution function

// properties of an operation
typedef struct operation_t {
	char		*symbol;	// ascii symbol
	int		opcode;		// opcode number
	instrFunc	execfunc;	// pointer to instruction execution function
} operation_t;

// properties of a register
typedef struct register_t {
	char	*symbol;	// ascii symbol
	int	num;		// register number
} register_t;

// properties of a line format
typedef struct lineformat_t {
	int	id;		// an id
	char	*format;	// format string for sscanf
	int	tokcount;	// number of tokens that must be matched by sscanf
} lineformat_t;

lineformat_t	lineformat[LINEFMTTYPES];	// line format library
operation_t 	operlib[OPERCOUNT];		// operations library
register_t 	reglib[REGCOUNT];		// register library
int		linenum, wordbytes, alignmask, memsize, debug;

// mf16 emulator interface function declarations
char*           stripComment(char*);
void            fatal(int, char*, char*);
void            initParams();
void            wireUp(cpu_t*);
void            initMem(cpu_t*, int);
void            execute(cpu_t*);
void*           ezmalloc(size_t);
void*           ezrealloc(void*, size_t);
int		is_blank(char*);
char*           trimWhite(char*);
int 		str2num(char*, int);

#endif