/*******************************************************************************
	mf16asmlib.h
	interface definition for mf16asmlib.c
 ******************************************************************************/

#ifndef MF16ASMLIB_H
#define MF16ASMLIB_H

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<errno.h>

// some nicknames for contants
#define UNKNOWN	-1
#define YES	1
#define NO	0

// insertInstr() operation modes
#define	INSTR	0
#define	DIREC	1

// some mf16 constants
#define WORDSIZE	16
#define MAXTOKENLEN	512	// maximum length of any token i.e. arg, operation, directive, data, etc.
#define MAXLINELEN	1024	// maximum line length in assembly code
#define LINEFMTTYPES	7	// number of line format types
#define MAXTOKEN	5	// max tokens in a line
#define MAXARGS		3	// max number of args in an instruction
#define OPERCOUNT	30	// total number of operations
#define REGCOUNT	16	// total number of registers
#define MAXLABELLEN	16	// max length of a label
#define OPCODESHFT	12	// opcode shift amount in an instruction
#define HIGHMASK	0xFF00	// upper half of word mask
#define LOWMASK		0x00FF	// lower half of word mask

// ranges of immediate and shamt fields
#define MINIMM16	0	
#define MAXIMM16	65536
#define MINIMM8		0
#define MAXIMM8		255
#define MINSHFT		0
#define MAXSHFT		15

// string formats of different token types
// to be consumed by sscanf
#define DIRECFMT	".%[a-z]"
#define DATAFMT		"%[^\n]"
#define LABELFMT	"%[a-zA-z0-9]:"
#define OPERFMT		"%[a-z]"
#define ARGFMT		"%[a-zA-Z0-9$%]"

typedef enum {UNUSED, IMM, REG} argType;	// argument types
typedef unsigned int uint;

struct argument_t;
struct operation_t;
struct instruction_t;
struct register_t;
struct label_t;
struct lineformat_t;

typedef void (*pseduoFunc)(struct instruction_t*);	// pseudo instruction expansion function

// properties of an argument (immediate or register)
typedef struct argument_t {
	argType	type;		// IMM, REG, UNUSED
	uint	written;	// will it be written to
	uint	lshift;		// how much shift for encoding
	int	max;		// max allowed
	int	min;		// min allowed
} argument_t;

// properties of an operation
typedef struct operation_t {
	char		*symbol;	// ascii symbol
	uint		opcode;		// opcode number
	pseduoFunc	pseudofunc;	// pseudo instruction expansion function
	uint		instrCount;	// number of embedded hardware instructions
	argument_t	arg[MAXARGS];	// array of argument properties
} operation_t;

// properties of a label in label list
typedef struct label_t {
	char		*symbol;	// ascii symbol 
	int		addr;		// equivalent address
	struct label_t	*next;
} label_t;

// properties of an instruction in instruction list
typedef struct instruction_t {
	char			*line;			// original assembly text
	char			*tok[MAXTOKEN];		// list of its tokens
	int			format;			// its format id
	char			*argtok[MAXARGS];	// list of its ascii args
	int			argval[MAXARGS];	// list of its arg values
	operation_t		*operation;		// operation type
	char			*labeltok;		// dereferenced label if any
	uint			linenum;		// assmebly code line number
	int			encoded;		// encoded format
	struct instruction_t	*next;
} instruction_t;

// properties of a register
typedef struct register_t {
	char	*symbol;	// ascii symbol
	uint	num;		// register number
	uint	writable;	// writable or read-only
} register_t;

// properties of a line format
typedef struct lineformat_t {
	uint	id;		// an id
	char	*format;	// format string for sscanf
	int	tokcount;	// number of tokens that must be matched by sscanf
} lineformat_t;

// global variables
lineformat_t	lineformat[LINEFMTTYPES];	// line format library
operation_t 	operlib[OPERCOUNT];		// operations library
register_t 	reglib[REGCOUNT];		// register library
instruction_t	*instrHead, *direcHead;		// instruction list head and tail
instruction_t	*instrTail, *direcTail;		// directive list head and tail
label_t	        *labelHead;			// label list head
int             pc; 				// hardware instruction counter (MIF line counter)
int             linenum;			// current line number
int             pseudo_left;			// any pseudo instructions left to expand?

// mf16asm interface function declarations
char*           stripComment(char*);
label_t*        addLabel(instruction_t*);
void            setTokens(instruction_t*);
void            parseArgs(instruction_t*);
void            parseLabel(instruction_t*);
void            expandPseudo(instruction_t*);
void            encodeInstr(instruction_t*);
void            printMIF();
instruction_t*  insertInstr(instruction_t*, instruction_t**, int);
instruction_t*  initInstr();
void            fatal(int, char*, char*);
void            initParams();
void            expand_li(instruction_t*);
void            expand_addi(instruction_t*);
void            expand_andi(instruction_t*);
void            expand_sub(instruction_t*);
void            expand_lwo(instruction_t*);
void            expand_swo(instruction_t*);
void            expand_push(instruction_t*);
void            expand_pop(instruction_t*);
void            expand_call(instruction_t*);
void            expand_jmp(instruction_t*);
void            expand_jeq(instruction_t*);
void            expand_slli(instruction_t*);
void            expand_srli(instruction_t*);
void            expand_bsri(instruction_t*);
void*           ezmalloc(size_t);
void*           ezrealloc(void*, size_t);
int             is_blank(char*);
char*           trimWhite(char*);

#endif