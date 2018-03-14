#include	"mf16asmlib.h"

// internal function declarations
int getLabelAddr(char*);
operation_t* getOperation(char*);
int getRegNum(char*);
label_t* initLabel();
void insertLabel(label_t*);
int str2num(char*, int, int**);

/*******************************************************************************
	mf16asmlib.c
		library of functions for MF16 assembler
	
 ******************************************************************************/

 /*------------------------------------------------------------------------------
	initParams
		initilize global parameters and libraries for mf16asm
	args
		none
	return
		none
------------------------------------------------------------------------------*/
void
initParams()
{
	// different possible formats of a line. This format is used in sscanf calls.
	// each line has an example above it.
	
	// Label1: .asciiz this is some data
	lineformat[0] = (lineformat_t){	1,	" "LABELFMT" "DIRECFMT" "DATAFMT" ",				3};
	// Label1: add arg1, arg2, arg3
	lineformat[1] = (lineformat_t){	2,	" "LABELFMT" "OPERFMT" "ARGFMT" , "ARGFMT" , "ARGFMT" ",	5};
	// Label1: lui arg1, arg2
	lineformat[2] = (lineformat_t){	3,	" "LABELFMT" "OPERFMT" "ARGFMT" , "ARGFMT" ",			4};
	// Label1: jr arg1
	lineformat[3] = (lineformat_t){	4,	" "LABELFMT" "OPERFMT" "ARGFMT" ",				3};
	// add arg1, arg2, arg3
	lineformat[4] = (lineformat_t){	5,	" "OPERFMT" "ARGFMT" , "ARGFMT" , "ARGFMT" ",			4};
	// lui arg1, arg2
	lineformat[5] = (lineformat_t){	6,	" "OPERFMT" "ARGFMT" , "ARGFMT" ",				3};
	// jr arg1
	lineformat[6] = (lineformat_t){	7,	" "OPERFMT" "ARGFMT" ",						2};


	// list of all hardware and pseudo instructions
	// see mf16asmlib.h for details of each field
	operlib[0] = (operation_t){	"add",	0,	NULL,	1,
	{
	(argument_t){REG,	NO,	8,	0,	0},
	(argument_t){REG,	NO,	4,	0,	0},
	(argument_t){REG,	YES,	0,	0,	0},
	}
	};
	operlib[1] = (operation_t){	"and",	1,	NULL,	1,
	{
	(argument_t){REG,	NO,	8,	0,	0},
	(argument_t){REG,	NO,	4,	0,	0},
	(argument_t){REG,	YES,	0,	0,	0},
	}
	};
	operlib[2] = (operation_t){	"beq",	2,	NULL,	1,
	{
	(argument_t){REG,	NO,	8,	0,	0},
	(argument_t){REG,	NO,	4,	0,	0},
	(argument_t){REG,	NO,	0,	0,	0},
	}
	};
	operlib[3] = (operation_t){	"jr",	3,	NULL,	1,
	{
	(argument_t){REG,	NO,	0,	0,	0},
	(argument_t){UNUSED,	NO,	0,	0,	0},
	(argument_t){UNUSED,	NO,	0,	0,	0},
	}
	};
	operlib[4] = (operation_t){	"jrl",	4,	NULL,	1,
	{
	(argument_t){REG,	NO,	0,	0,	0},
	(argument_t){UNUSED,	NO,	0,	0,	0},
	(argument_t){UNUSED,	NO,	0,	0,	0},
	}
	};
	operlib[5] = (operation_t){	"lui",	5,	NULL,	1,
	{
	(argument_t){REG,	YES,	8,	0,	0},
	(argument_t){IMM,	NO,	0,	MAXIMM8,	MINIMM8},
	(argument_t){UNUSED,	NO,	0,	0,	0},
	}
	};
	operlib[6] = (operation_t){	"lw",	6,	NULL,	1,
	{
	(argument_t){REG,	YES,	8,	0,	0},
	(argument_t){REG,	NO,	4,	0,	0},
	(argument_t){UNUSED,	NO,	0,	0,	0},
	}
	};
	operlib[7] = (operation_t){	"bsr",	7,	NULL,	1,
	{
	(argument_t){REG,	NO,	8,	0,	0},
	(argument_t){REG,	NO,	4,	0,	0},
	(argument_t){REG,	YES,	0,	0,	0},
	}
	};
	operlib[8] = (operation_t){	"nor",	8,	NULL,	1,
	{
	(argument_t){REG,	NO,	8,	0,	0},
	(argument_t){REG,	NO,	4,	0,	0},
	(argument_t){REG,	YES,	0,	0,	0},
	}
	};
	operlib[9] = (operation_t){	"or",	9,	NULL,	1,
	{
	(argument_t){REG,	NO,	8,	0,	0},
	(argument_t){REG,	NO,	4,	0,	0},
	(argument_t){REG,	YES,	0,	0,	0},
	}
	};
	operlib[10] = (operation_t){	"ori",	10,	NULL,	1,
	{
	(argument_t){REG,	YES,	8,	0,	0},
	(argument_t){IMM,	NO,	0,	MAXIMM8,	MINIMM8},
	(argument_t){UNUSED,	NO,	0,	0,	0},
	}
	};
	operlib[11] = (operation_t){	"sll",	11,	NULL,	1,
	{
	(argument_t){REG,	NO,	8,	0,	0},
	(argument_t){REG,	NO,	4,	0,	0},
	(argument_t){REG,	YES,	0,	0,	0},
	}
	};
	operlib[12] = (operation_t){	"slt",	12,	NULL,	1,
	{
	(argument_t){REG,	NO,	8,	0,	0},
	(argument_t){REG,	NO,	4,	0,	0},
	(argument_t){REG,	YES,	0,	0,	0},
	}
	};
	operlib[13] = (operation_t){	"xor",	13,	NULL,	1,
	{
	(argument_t){REG,	NO,	8,	0,	0},
	(argument_t){REG,	NO,	4,	0,	0},
	(argument_t){REG,	YES,	0,	0,	0},
	}
	};
	operlib[14] = (operation_t){	"srl",	14,	NULL,	1,
	{
	(argument_t){REG,	NO,	8,	0,	0},
	(argument_t){REG,	NO,	4,	0,	0},
	(argument_t){REG,	YES,	0,	0,	0},
	}
	};
	operlib[15] = (operation_t){	"sw",	15,	NULL,	1,
	{
	(argument_t){REG,	NO,	8,	0,	0},
	(argument_t){REG,	NO,	4,	0,	0},
	(argument_t){UNUSED,	NO,	0,	0,	0},
	}
	};
	
	operlib[16] = (operation_t){	"li",	16,	expand_li,	2,
	{
	(argument_t){REG,	YES,	0,	0,	0},
	(argument_t){IMM,	NO,	0,	MAXIMM16,	MINIMM16},
	(argument_t){UNUSED,	NO,	0,	0,	0},
	}
	};
	operlib[17] = (operation_t){	"addi",	17,	expand_addi,	3,
	{
	(argument_t){REG,	NO,	0,	0,	0},
	(argument_t){IMM,	NO,	0,	MAXIMM16,	MINIMM16},
	(argument_t){REG,	YES,	0,	0,	0},
	}
	};
	operlib[18] = (operation_t){	"andi",	18,	expand_andi,	3,
	{
	(argument_t){REG,	NO,	0,	0,	0},
	(argument_t){IMM,	NO,	0,	MAXIMM16,	MINIMM16},
	(argument_t){REG,	YES,	0,	0,	0},
	}
	};
	operlib[19] = (operation_t){	"sub",	19,	expand_sub,	13,
	{
	(argument_t){REG,	NO,	0,	0,	0},
	(argument_t){REG,	NO,	0,	0,	0},
	(argument_t){REG,	YES,	0,	0,	0},
	}
	};
	operlib[20] = (operation_t){	"lwo",	20,	expand_lwo,	4,
	{
	(argument_t){REG,	YES,	0,	0,	0},
	(argument_t){REG,	NO,	0,	0,	0},
	(argument_t){IMM,	NO,	0,	MAXIMM16,	MINIMM16},
	}
	};
	operlib[21] = (operation_t){	"swo",	21,	expand_swo,	4,
	{
	(argument_t){REG,	NO,	0,	0,	0},
	(argument_t){REG,	NO,	0,	0,	0},
	(argument_t){IMM,	NO,	0,	MAXIMM16,	MINIMM16},
	}
	};
	operlib[22] = (operation_t){	"push",	22,	expand_push,	4,
	{
	(argument_t){REG,	NO,	0,	0,	0},
	(argument_t){UNUSED,	NO,	0,	0,	0},
	(argument_t){UNUSED,	NO,	0,	0,	0},
	}
	};
	operlib[23] = (operation_t){	"pop",	23,	expand_pop,	4,
	{
	(argument_t){REG,	YES,	0,	0,	0},
	(argument_t){UNUSED,	NO,	0,	0,	0},
	(argument_t){UNUSED,	NO,	0,	0,	0},
	}
	};
	operlib[24] = (operation_t){	"call",	24,	expand_call,	11,
	{
	(argument_t){IMM,	NO,	0,	MAXIMM16,	MINIMM16},
	(argument_t){UNUSED,	NO,	0,	0,	0},
	(argument_t){UNUSED,	NO,	0,	0,	0},
	}
	};
	operlib[25] = (operation_t){	"jmp",	25,	expand_jmp,	3,
	{
	(argument_t){IMM,	NO,	0,	MAXIMM16,	MINIMM16},
	(argument_t){UNUSED,	NO,	0,	0,	0},
	(argument_t){UNUSED,	NO,	0,	0,	0},
	}
	};
	operlib[26] = (operation_t){	"jeq",	26,	expand_jeq,	3,
	{
	(argument_t){REG,	NO,	0,	0,	0},
	(argument_t){REG,	NO,	0,	0,	0},
	(argument_t){IMM,	NO,	0,	MAXIMM16,	MINIMM16},
	}
	};
	operlib[27] = (operation_t){	"slli",	27,	expand_slli,	3,
	{
	(argument_t){IMM,	NO,	0,	MAXSHFT,	MINSHFT},
	(argument_t){REG,	NO,	0,	0,	0},
	(argument_t){REG,	NO,	0,	0,	0},
	}
	};
	operlib[28] = (operation_t){	"srli",	28,	expand_srli,	3,
	{
	(argument_t){IMM,	NO,	0,	MAXSHFT,	MINSHFT},
	(argument_t){REG,	NO,	0,	0,	0},
	(argument_t){REG,	NO,	0,	0,	0},
	}
	};
	operlib[29] = (operation_t){	"bsri",	29,	expand_bsri,	3,
	{
	(argument_t){IMM,	NO,	0,	MAXSHFT,	MINSHFT},
	(argument_t){REG,	NO,	0,	0,	0},
	(argument_t){REG,	NO,	0,	0,	0},
	}
	};
	// list of all registers
	// see mf16data.h for details of each field
	reglib[0] = (register_t){	"$0",	0,	NO};
	reglib[1] = (register_t){	"$s0",	1,	YES};
	reglib[2] = (register_t){	"$s1",	2,	YES};
	reglib[3] = (register_t){	"$s2",	3,	YES};
	reglib[4] = (register_t){	"$s3",	4,	YES};
	reglib[5] = (register_t){	"$t0",	5,	YES};
	reglib[6] = (register_t){	"$t1",	6,	YES};
	reglib[7] = (register_t){	"$t2",	7,	YES};
	reglib[8] = (register_t){	"$t3",	8,	YES};
	reglib[9] = (register_t){	"$a0",	9,	YES};
	reglib[10] = (register_t){	"$a1",	10,	YES};
	reglib[11] = (register_t){	"$la",	11,	NO};
	reglib[12] = (register_t){	"$sp",	12,	YES};
	reglib[13] = (register_t){	"$rv",	13,	YES};
	reglib[14] = (register_t){	"$at",	14,	NO};
	reglib[15] = (register_t){	"$ra",	15,	YES};

	instrHead = NULL;
	instrTail = NULL;
	direcHead = NULL;
	direcTail = NULL;
	labelHead = NULL;
	pc = 0;

}


/*------------------------------------------------------------------------------
	stripComment
		remove comments from line (anything after and including the first # sign)
	args
		pointer to line 
	returns
		pointer to commentless line
------------------------------------------------------------------------------*/
char*
stripComment(char* str)
{
	uint i, end = strlen(str);

	for (i=0; i<strlen(str); i++) {
		if (*(str+i) == '#') {
			end = i;
			break;
		}
	}

	*(str + end) = '\0';
	
	return str;
}
/*------------------------------------------------------------------------------
	addLabel
		add label definition from instruction to label table
	args
		pointer to instruction
	returns
		pointer to label
------------------------------------------------------------------------------*/
label_t*
addLabel(instruction_t *this)
{
	label_t *p;

	p = initLabel();
	p->symbol = this->tok[0];
	p->addr = pc * WORDSIZE/8;
	
	insertLabel(p);
	
	return p;
}

/*------------------------------------------------------------------------------
	getLabelAddr
		translate label text to its address
	args
		label text
	returns
		label address
------------------------------------------------------------------------------*/
int
getLabelAddr(char* symbol)
{
	int addr = UNKNOWN;
	label_t *p = labelHead;

	do {
		if (strcmp(symbol, p->symbol) == 0) {
			addr = p->addr;
			break;
		}
	} while ( (p = p->next) );
	
	return addr;
}

 /*------------------------------------------------------------------------------
	getOperation
		translate operation text to its object
	args
		operation text
	return
		operation_t object
------------------------------------------------------------------------------*/
operation_t*
getOperation(char* symbol)
{
	operation_t *oper = NULL;
	int i;

	for (i=0; i<OPERCOUNT; i++) {
		if (strcmp(symbol, operlib[i].symbol) == 0) {
			oper = &(operlib[i]);
			break;
		}
	}
	
	return oper;
}

 /*------------------------------------------------------------------------------
	setTokens
		assign operation object and argument tokens for instruction
	args
		pointer to instruction
	return
		none
------------------------------------------------------------------------------*/
void
setTokens(instruction_t *this)
{
	char	*opertok;
	int i, argoffset;
	
	if (this->format >= 2 && this->format <= 4) {	// if labeled format
		addLabel(this);
		opertok = this->tok[1];
		argoffset = 2;
	}	
	else if (this->format >= 5) {			// if unlabaled format
		opertok = this->tok[0];
		argoffset = 1;
	}
	else
		return;
	
	this->operation = getOperation(opertok);
	
	if (!this->operation)
		fatal(this->linenum, "unknown operation symbol", opertok);
	
	for (i=0; i<MAXTOKEN; i++)
		if (is_blank(this->tok[i]))
			this->tok[i] = NULL;
	
	for (i=0; i<MAXARGS; i++)
		this->argtok[i] = this->tok[i+argoffset];
}
/*------------------------------------------------------------------------------
	parseArgs:
		validate and evaluate arguments in instruction
	args
		pointer to current instruction
	returns
		none
------------------------------------------------------------------------------*/
void
parseArgs(instruction_t *this)
{
	int i, val, *success;
	argument_t argprops;
	char* arg;
	
	for (i=0; i<MAXARGS; i++) {
	
	arg = this->argtok[i];
	argprops = this->operation->arg[i];
	
	if (arg && argprops.type != UNUSED) {	// if arg is given and arg is expected
		
		if (*arg == '$') {	// if arg is register
		
			if (argprops.type != REG)
				fatal(this->linenum, "register specifier not allowed in this position", arg);
			
			if ( (val = getRegNum(arg)) == -1 )
				fatal(this->linenum, "unknown register.", arg);
			
			if (reglib[val].writable == NO && argprops.written == YES)
				fatal(this->linenum, "register cannot be written to", arg);
		}
		else if (strncmp(arg, "0x", 2) == 0) {	// if arg is hex number
		
			if (argprops.type != IMM)
				fatal(this->linenum, "immediate not allowed in this position", arg);
			
			val = str2num(arg+2, 16, &success);
			if (!*success)
				fatal(this->linenum, "invalid hex number", arg);
			
			if (val > argprops.max || val < argprops.min)
				fatal(this->linenum, "hex number out of range", arg);
		}
		else if (*arg == '%')	// if arg is dereferenced label
			this->labeltok = arg;
		
		this->argval[i] = val;
	}
	else if (arg && argprops.type == UNUSED)		// if arg is given and arg is not expected
		fatal(this->linenum, "extra argument", arg);
	else if (!arg && argprops.type != UNUSED)		// if arg is not given but arg expected or vice versa
		fatal(this->linenum, "missing argument", "");
	}
	
}

/*------------------------------------------------------------------------------
	parseLabel:
		validate and evaluate dereferenced labels in instructions
	args
		pointer to current instruction
	returns
		none
------------------------------------------------------------------------------*/
void
parseLabel(instruction_t *this)
{
	int i, val;
	argument_t argprops;
	char* arg;
	
	for (i=0; i<MAXARGS; i++) {
	
		arg = this->argtok[i];
		argprops = this->operation->arg[i];
		
		if (arg && *arg == '%') {
		
			if (argprops.type != IMM)
				fatal(this->linenum, "labels are not allowed in a register position", arg);
			
			if ( (val = getLabelAddr(arg+1)) == -1 )
				fatal(this->linenum, "label undefined", arg);
			
			if (val > argprops.max)
				fatal(this->linenum, "label definition is too far", arg);
			
			this->argval[i] = val;
		}
	}
	
}
 /*------------------------------------------------------------------------------
	expandPseudo
		expand pseudo instruction in the current instruction
	args
		pointer to current instruction
	return
		none
------------------------------------------------------------------------------*/
void
expandPseudo(instruction_t *this)
{
	void (*expndfunc)(instruction_t*) = this->operation->pseudofunc;
	
	if (expndfunc)	// if any pseudo instruction expansion function exists
		(*expndfunc)(this);

}

/*------------------------------------------------------------------------------
	The following functions are named with the "expand_<pseudo instruction name>"
	format. Each function is responsible for expanding a pseudo instruction and
	inserting it into the chain of instructions.
	args
		pointer to current instruction
	returns
		none
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
	expand_li:
------------------------------------------------------------------------------*/
void
expand_li(instruction_t *this)
{
	instruction_t *thisP1 = insertInstr(initInstr(), &this, INSTR);

	thisP1->operation = getOperation("ori");
	thisP1->argval[0] = this->argval[0];
	thisP1->argval[1] = this->argval[1] & LOWMASK;
	
	this->operation = getOperation("lui");
	this->argval[1] = (this->argval[1] & HIGHMASK) >> (WORDSIZE/2);
	this->argval[2] = UNKNOWN;
}

/*------------------------------------------------------------------------------
	expand_addi:
------------------------------------------------------------------------------*/
void
expand_addi(instruction_t *this)
{
	instruction_t *thisP1;
	int at = getRegNum("$at");
	
	pseudo_left = YES;
	
	thisP1 = insertInstr(initInstr(), &this, INSTR);

	thisP1->operation = getOperation("add");
	thisP1->argval[0] = this->argval[0];
	thisP1->argval[1] = at;
	thisP1->argval[2] = this->argval[2];
	
	this->operation = getOperation("li");
	this->argval[0] = at;
	this->argval[1] = this->argval[1];
}
/*------------------------------------------------------------------------------
	expand_andi:
------------------------------------------------------------------------------*/
void
expand_andi(instruction_t *this)
{
	instruction_t *thisP1;
	int at = getRegNum("$at");
	
	pseudo_left = YES;
	
	thisP1 = insertInstr(initInstr(), &this, INSTR);

	thisP1->operation = getOperation("and");
	thisP1->argval[0] = this->argval[0];
	thisP1->argval[1] = at;
	thisP1->argval[2] = this->argval[2];
	
	this->operation = getOperation("li");
	this->argval[0] = at;
	this->argval[1] = this->argval[1];
}

/*------------------------------------------------------------------------------
	expand_sub:
------------------------------------------------------------------------------*/
void
expand_sub(instruction_t *this)
{
	instruction_t *thisP1, *thisP2, *thisP3, *thisP4;
	int zero = getRegNum("$0");
	
	thisP4 = insertInstr(initInstr(), &this, INSTR);
	thisP3 = insertInstr(initInstr(), &this, INSTR);
	thisP2 = insertInstr(initInstr(), &this, INSTR);
	thisP1 = insertInstr(initInstr(), &this, INSTR);

	pseudo_left = YES;
	
	thisP4->operation = getOperation("pop");
	thisP4->argval[0] = this->argval[1];	
	
	thisP3->operation = getOperation("add");
	thisP3->argval[0] = this->argval[0];
	thisP3->argval[1] = this->argval[1];
	thisP3->argval[2] = this->argval[2];
	
	thisP2->operation = getOperation("addi");
	thisP2->argval[0] = this->argval[1];
	thisP2->argval[1] = 1;
	thisP2->argval[2] = this->argval[1];
	
	thisP1->operation = getOperation("nor");
	thisP1->argval[0] = this->argval[1];
	thisP1->argval[1] = zero;
	thisP1->argval[2] = this->argval[1];
	
	this->operation = getOperation("push");
	this->argval[0] = this->argval[1];
}

/*------------------------------------------------------------------------------
	expand_lwo:
------------------------------------------------------------------------------*/
void
expand_lwo(instruction_t *this)
{
	instruction_t *thisP1;
	int at = getRegNum("$at");
	
	pseudo_left = YES;
	
	thisP1 = insertInstr(initInstr(), &this, INSTR);
	
	thisP1->operation = getOperation("lw");
	thisP1->argval[0] = this->argval[0];
	thisP1->argval[1] = at;
	
	this->operation = getOperation("addi");
	this->argval[0] = this->argval[1];
	this->argval[1] = this->argval[2];
	this->argval[2] = at;
}

/*------------------------------------------------------------------------------
	expand_swo:
------------------------------------------------------------------------------*/
void
expand_swo(instruction_t *this)
{
	instruction_t *thisP1;
	int at = getRegNum("$at");
	
	pseudo_left = YES;
	
	thisP1 = insertInstr(initInstr(), &this, INSTR);
	
	thisP1->operation = getOperation("sw");
	thisP1->argval[0] = this->argval[0];
	thisP1->argval[1] = at;
	
	this->operation = getOperation("addi");
	this->argval[0] = this->argval[1];
	this->argval[1] = this->argval[2];
	this->argval[2] = at;
}
/*------------------------------------------------------------------------------
	expand_push:
------------------------------------------------------------------------------*/
void
expand_push(instruction_t *this)
{
	instruction_t *thisP1;
	int sp = getRegNum("$sp");
	
	pseudo_left = YES;
	
	thisP1 = insertInstr(initInstr(), &this, INSTR);
	
	thisP1->operation = getOperation("sw");
	thisP1->argval[0] = this->argval[0];
	thisP1->argval[1] = sp;
	
	this->operation = getOperation("addi");
	this->argval[0] = sp;
	this->argval[1] = -2;
	this->argval[2] = sp;
}
/*------------------------------------------------------------------------------
	expand_pop:
------------------------------------------------------------------------------*/
void
expand_pop(instruction_t *this)
{
	instruction_t *thisP1;
	int sp = getRegNum("$sp");
	
	pseudo_left = YES;
	
	thisP1 = insertInstr(initInstr(), &this, INSTR);
	
	thisP1->operation = getOperation("addi");
	thisP1->argval[0] = sp;
	thisP1->argval[1] = 2;
	thisP1->argval[2] = sp;
	
	this->operation = getOperation("lw");
	this->argval[0] = this->argval[0];
	this->argval[1] = sp;
}
/*------------------------------------------------------------------------------
	expand_call:
		expand call pseudo instruction
	args
		pointer to current instruction
	returns
		none
------------------------------------------------------------------------------*/
void
expand_call(instruction_t *this)
{
	instruction_t *thisP1, *thisP2, *thisP3;
	int la = getRegNum("$la"), ra = getRegNum("$ra");
	
	pseudo_left = YES;
	
	thisP3 = insertInstr(initInstr(), &this, INSTR);
	thisP2 = insertInstr(initInstr(), &this, INSTR);
	thisP1 = insertInstr(initInstr(), &this, INSTR);
	
	thisP3->operation = getOperation("pop");
	thisP3->argval[0] = ra;
	
	thisP2->operation = getOperation("jrl");
	thisP2->argval[0] = la;
	
	thisP1->operation = getOperation("push");
	thisP1->argval[0] = ra;
	
	this->operation = getOperation("li");
	this->argval[1] = this->argval[0];
	this->argval[0] = la;
	
}
/*------------------------------------------------------------------------------
	expand_jmp:
------------------------------------------------------------------------------*/
void
expand_jmp(instruction_t *this)
{
	instruction_t *thisP1;
	int la = getRegNum("$la");
	
	pseudo_left = YES;
	
	thisP1 = insertInstr(initInstr(), &this, INSTR);
	
	thisP1->operation = getOperation("jr");
	thisP1->argval[0] = la;
	
	this->operation = getOperation("li");
	this->argval[0] = la;
	this->argval[1] = getLabelAddr(this->labeltok + 1);
	
}
/*------------------------------------------------------------------------------
	expand_jeq:
------------------------------------------------------------------------------*/
void
expand_jeq(instruction_t *this)
{
	instruction_t *thisP1;
	int la = getRegNum("$la");
	
	pseudo_left = YES;
	
	thisP1 = insertInstr(initInstr(), &this, INSTR);
	
	thisP1->operation = getOperation("beq");
	thisP1->argval[0] = this->argval[0];
	thisP1->argval[1] = this->argval[1];
	thisP1->argval[2] = la;
	
	this->operation = getOperation("li");
	this->argval[0] = la;
	this->argval[1] = getLabelAddr(this->labeltok + 1);
	
}
/*------------------------------------------------------------------------------
	expand_slli:
------------------------------------------------------------------------------*/
void
expand_slli(instruction_t *this)
{
	instruction_t *thisP1;
	int at = getRegNum("$at");
	
	pseudo_left = YES;
	
	thisP1 = insertInstr(initInstr(), &this, INSTR);

	thisP1->operation = getOperation("sll");
	thisP1->argval[0] = at;
	thisP1->argval[1] = this->argval[1];
	thisP1->argval[2] = this->argval[2];
	
	this->operation = getOperation("li");
	this->argval[1] = this->argval[0];
	this->argval[0] = at;
}
/*------------------------------------------------------------------------------
	expand_srli:
------------------------------------------------------------------------------*/
void
expand_srli(instruction_t *this)
{
	instruction_t *thisP1;
	int at = getRegNum("$at");
	
	pseudo_left = YES;
	
	thisP1 = insertInstr(initInstr(), &this, INSTR);

	thisP1->operation = getOperation("srl");
	thisP1->argval[0] = at;
	thisP1->argval[1] = this->argval[1];
	thisP1->argval[2] = this->argval[2];
	
	this->operation = getOperation("li");
	this->argval[1] = this->argval[0];
	this->argval[0] = at;
}
/*------------------------------------------------------------------------------
	expand_bsri:
------------------------------------------------------------------------------*/
void
expand_bsri(instruction_t *this)
{
	instruction_t *thisP1;
	int at = getRegNum("$at");
	
	pseudo_left = YES;
	
	thisP1 = insertInstr(initInstr(), &this, INSTR);

	thisP1->operation = getOperation("bsr");
	thisP1->argval[0] = at;
	thisP1->argval[1] = this->argval[1];
	thisP1->argval[2] = this->argval[2];
	
	this->operation = getOperation("li");
	this->argval[1] = this->argval[0];
	this->argval[0] = at;
}
/*------------------------------------------------------------------------------
	encodeInstr
		encode instruction
	args
		pointer to current instruction
	returns
		none
------------------------------------------------------------------------------*/
void
encodeInstr(instruction_t *this)
{
	int i;
	
	this->encoded = 0 | (this->operation->opcode << OPCODESHFT);
	
	for (i=0; i<MAXARGS; i++)
		if (this->argval[i] != UNKNOWN)
			this->encoded = this->encoded | (this->argval[i] << this->operation->arg[i].lshift);
}

/*------------------------------------------------------------------------------
	printMIF
		print encoded instructions in MIF file format.
		This also prints the original assembly code and
		all expanded pseudo instructions.
	args
		none
	returns
		none
------------------------------------------------------------------------------*/
void
printMIF()
{
	instruction_t *this;
	char *str;
	int addr = 0, i;
	

	printf("DEPTH = 32768;                -- The size of memory in words\n");
	printf("WIDTH = 16;                   -- The size of data in bits\n");
	printf("ADDRESS_RADIX = HEX;          -- The radix for address values\n");
	printf("DATA_RADIX = HEX;             -- The radix for data values\n");
	printf("CONTENT                       -- start of (address : data pairs)\n");
	printf("\nBEGIN\n\n");
	
	this = instrHead;
	do {
		printf("%04X : %04X; -- %-6s", addr, this->encoded, this->operation->symbol);
		for (i=0; i<MAXARGS; i++) {
			if (this->argval[i] != UNKNOWN)
				printf("0x%04X ", this->argval[i]);
			else
				printf("       ");
		}
		addr++;
		if (this->line)
			printf(" ----------- %-50s ", this->line);
		printf("\n");
	
	} while ( (this = this->next) );
	
	this = direcHead;
	while (this) {
		str = this->tok[2];
		while (*str) {
			printf("%04X : %04X; -- \"%c\"\n", addr++, *str, *str);
			str++;
		}
		printf("%04X : %04X; -- \"%s\"\n", addr++, 10, "\\n"); // print new line at the end of string
		printf("%04X : %04X; -- \"%s\"\n", addr++, 0, "\\0"); // print null at the end of string
		this = this->next;
	}
	
	printf("\nEND;\n");
}
/*------------------------------------------------------------------------------
	getRegNum
		translate register name to its number
	args
		register name
	returns
		register number
------------------------------------------------------------------------------*/
int
getRegNum(char *symbol)
{
	int i, reg = -1;

	for (i=0; i<REGCOUNT; i++) {
		if (strcmp(symbol, reglib[i].symbol) == 0)
			reg = reglib[i].num;
	}
	
	return reg;
}

/*------------------------------------------------------------------------------
	ezmalloc
		memory allocation with error checking
	args:
		allocation size
	returns
		pointer to allocated memory
------------------------------------------------------------------------------*/
void*
ezmalloc(size_t sz)
{
	void *p = malloc(sz);

	if (p == NULL) {
		perror("malloc");
		fatal(linenum, "could not allocate memory", "");
	}

	return p;
}

/*------------------------------------------------------------------------------
	ezrealloc
		memory reallocation with error checking
	args
		pointer to originally allocated memory
		allocation size
	returns
		pointer to allocated memory
------------------------------------------------------------------------------*/
void*
ezrealloc(void *p, size_t sz)
{
	p = realloc(p, sz);
	if (p == NULL) {
		perror("realloc");
		fatal(linenum, "could not reallocate memory", "");
	}

	return p;
}

/*------------------------------------------------------------------------------
	insertInstr
		insert an intruction_t object into list
	args
		pointer to instruction_t to be added
		pointer to instruction_t to add after
		which list to insert into (INSTR, DIREC)
	returns
		none
------------------------------------------------------------------------------*/
instruction_t*
insertInstr(instruction_t *data, instruction_t **where, int mode)
{
	if (*where) {
		data->next = (*where)->next;
		(*where)->next = data;
	}
	else
		data->next = *where;
	
	if (mode == DIREC) {
		if (*where == direcTail)
			direcTail = data;
		}
	else {
		if (*where == instrTail)
			instrTail = data;
	}

	return data;
}

/*------------------------------------------------------------------------------
	initInstr:
		create and initilize and instruction_t object
	args
		none
	returns
		pointer to current instruction
------------------------------------------------------------------------------*/
instruction_t*
initInstr()
{
	instruction_t *p;
	int i;
	
	p = (instruction_t*)ezmalloc(sizeof(instruction_t));
	p->line = NULL;
	p->format = UNKNOWN;
	p->operation = NULL;
	p->labeltok = NULL;
	p->encoded = UNKNOWN;
	p->next = NULL;
	p->linenum = 0;
	
	for (i=0; i<MAXARGS; i++) {
		p->argtok[i] = NULL;
		p->argval[i] = UNKNOWN;
	}
	
	for (i=0; i<MAXTOKEN; i++)
		p->tok[i] = (char*)ezmalloc(MAXTOKENLEN+1);
	
	return p;
}

/*------------------------------------------------------------------------------
	initLabel
		create and initilize a label_t object
	
	args
		none
	returns
		pointer to label_t object
------------------------------------------------------------------------------*/
label_t*
initLabel()
{
	label_t *p;
	
	p = (label_t*)ezmalloc(sizeof(label_t));
	p->symbol = (char*)ezmalloc(MAXLABELLEN+1);
	p->addr = UNKNOWN;
	
	return p;
}

/*------------------------------------------------------------------------------
	insertLabel
		insert a label_t object into list
	args
		pointer to label_t to be added
	returns
		none
------------------------------------------------------------------------------*/
void
insertLabel(label_t *data)
{
	data->next = labelHead;
	labelHead = data;
}

/*------------------------------------------------------------------------------
	fatal
		prints error message and line number and exits the program
	args
		line number
		message1
		message2
	returns
		none
------------------------------------------------------------------------------*/
void
fatal(int linenum, char *s1, char *s2)
{
	fprintf(stderr, "\nError: line (%d): %s: %s\n\n",  linenum, s1, s2);
	exit(EXIT_FAILURE);
}

/*------------------------------------------------------------------------------
	is_blank
		check if a string is all whitespace
	args
		string
	returns
		YES or NO
------------------------------------------------------------------------------*/
int
is_blank(char *str)
{
	int result = YES;
	
	if (*str != '\0') {
		while (*str++) {
			if (*str != ' ' && *str != '\t' && *str != '\n' && *str != '\0') {
				result = NO;
				break;
			}
		}
	}
	
	return result;
}

/*------------------------------------------------------------------------------
	str2num
		convert string to integer (wrapper for strtol)
	args
		string
		number base
		success or fail
	returns
		converted number
	note
		success is indicated by modifying success argument
------------------------------------------------------------------------------*/
int
str2num(char *str, int base, int **success)
{
	long int	result;
	char	**endptr = &str;

	errno = 0;
	result = strtol(str, endptr, base);
	if ( errno != 0 || **endptr != '\0')
		*success = NULL;
	
	return result;
}
/*------------------------------------------------------------------------------
	trimWhite
		trim edge white spaces from string
	args
		string
	returns
		trimmed string
------------------------------------------------------------------------------*/
char*
trimWhite(char *str)
{
	int i;
	
	do {
		if (*str != ' ' && *str != '\t')
			break;
	} while ( (*str++) );
	
	for (i=strlen(str)-1; i>=0; i--)
		if (*(str+i) == ' ' || *(str+i) == '\t' || *(str+i) == '\n')
			*(str+i) = '\0';
	else
		break;
	
	return str;
}

