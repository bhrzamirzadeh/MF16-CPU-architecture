#include 	<math.h>
#include 	<unistd.h>
#include 	<termios.h>
#include	"mf16emulib.h"


// internal function declarations

// all functions named "run_<component>" update 
// the component's outputs based on it current inputs
void run_pc(cpu_t*);
void run_memAddrMux(cpu_t*);
void run_mem(cpu_t*);
void run_ir(cpu_t*);
void reset_control_signals(cpu_t*);
void run_pcAdder(cpu_t*);
void run_zeroExt(cpu_t*);
void run_regFile(cpu_t*);
void run_aluMux1(cpu_t*);
void run_alu(cpu_t*);
void run_regWriteDataMux(cpu_t*);
void run_branchAnd(cpu_t*);
void run_pcMux(cpu_t*);

// all functions named "exec_<something>" activate different circuit components 
// in the proper order based on the current opcode
void exec_alu(cpu_t*);
void exec_branch(cpu_t*);
void exec_lui(cpu_t*);
void exec_lw(cpu_t*);
void exec_sw(cpu_t*);

int do_srl(int, int);
char* getRegName(int);
char* getOperName(int);
void printDebug(cpu_t*);
instrFunc getInstrCtrlFunc(int);

/*******************************************************************************
	mf16emulib.c
		library of functions for mf16asm emulator
	
 ******************************************************************************/

 /*------------------------------------------------------------------------------
	initParams
		initilize global parameters and libraries for mf16emu
	args
		none
	return
		none
------------------------------------------------------------------------------*/
void
initParams()
{
	// different possible formats of a line in MIF file
	lineformat[0] = (lineformat_t){	1,	"DEPTH = %s ;",		1};
	lineformat[1] = (lineformat_t){	2,	"ADDRESS_RADIX = %s ;",	1};
	lineformat[2] = (lineformat_t){	3,	"DATA_RADIX = %s ;",	1};
	lineformat[3] = (lineformat_t){	4,	" %X : %X; ",		2};


	// list of all hardware instructions	
	operlib[0] = (operation_t){	"add",	0,	exec_alu};
	operlib[1] = (operation_t){	"and",	1,	exec_alu};
	operlib[2] = (operation_t){	"beq",	2,	exec_branch};
	operlib[3] = (operation_t){	"jr",	3,	exec_branch};
	operlib[4] = (operation_t){	"jrl",	4,	exec_branch};
	operlib[5] = (operation_t){	"lui",	5,	exec_alu};
	operlib[6] = (operation_t){	"lw",	6,	exec_lw};
	operlib[7] = (operation_t){	"bsr",	7,	exec_alu};
	operlib[8] = (operation_t){	"nor",	8,	exec_alu};
	operlib[9] = (operation_t){	"or",	9,	exec_alu};
	operlib[10] = (operation_t){	"ori",	10,	exec_alu};
	operlib[11] = (operation_t){	"sll",	11,	exec_alu};
	operlib[12] = (operation_t){	"slt",	12,	exec_alu};
	operlib[13] = (operation_t){	"xor",	13,	exec_alu};
	operlib[14] = (operation_t){	"srl",	14,	exec_alu};
	operlib[15] = (operation_t){	"sw",	15,	exec_sw};

	// list of all registers
	reglib[0] = (register_t){	"$0",	0,	};
	reglib[1] = (register_t){	"$s0",	1,	};
	reglib[2] = (register_t){	"$s1",	2,	};
	reglib[3] = (register_t){	"$s2",	3,	};
	reglib[4] = (register_t){	"$s3",	4,	};
	reglib[5] = (register_t){	"$t0",	5,	};
	reglib[6] = (register_t){	"$t1",	6,	};
	reglib[7] = (register_t){	"$t2",	7,	};
	reglib[8] = (register_t){	"$t3",	8,	};
	reglib[9] = (register_t){	"$a0",	9,	};
	reglib[10] = (register_t){	"$a1",	10,	};
	reglib[11] = (register_t){	"$la",	11,	};
	reglib[12] = (register_t){	"$sp",	12,	};
	reglib[13] = (register_t){	"$rv",	13,	};
	reglib[14] = (register_t){	"$at",	14,	};
	reglib[15] = (register_t){	"$ra",	15,	};
	
	wordbytes = WORDSIZE/8;
	
	alignmask = ALIGNMASK;
}

 /*------------------------------------------------------------------------------
	wireUp
		connect all CPU circuit components together
	args
		pointer to cpu_t struct
	return
		none
------------------------------------------------------------------------------*/
void
wireUp(cpu_t *cpu)
{
	pc_t		*pc			= &(cpu->pc);
	ir_t		*ir			= &(cpu->ir);
	memory_t	*mem			= &(cpu->mem);
	regfile_t	*regFile		= &(cpu->regFile);
	alu_t		*alu			= &(cpu->alu);
	in2out1_t	*branchAnd		= &(cpu->branchAnd);
	in2out1_t	*pcAdder		= &(cpu->pcAdder);
	in2out1_t	*zeroExt		= &(cpu->zeroExt);
	mux21_t		*memAddrMux		= &(cpu->memAddrMux);
	mux21_t		*pcMux			= &(cpu->pcMux);
	mux21_t		*aluMux1		= &(cpu->aluMux1);
	mux41_t		*regWriteDataMux	= &(cpu->regWriteDataMux);
	sequencer_t	*seq			= &(cpu->seq);
	
	pc->in = &(pcMux->out);
	pc->enableWrite = &(seq->writePC);
	
	memAddrMux->in[0] = &(pc->out);
	memAddrMux->in[1] = &(regFile->rtData);
	memAddrMux->sel = &(seq->memAddrSrc);
	
	mem->writeData = &(regFile->rsData);
	mem->addr = &(memAddrMux->out);
	mem->enableWrite = &(seq->writeMem);
	
	pcAdder->in[0] = &wordbytes;
	pcAdder->in[1] = &(pc->out);
	
	ir->in = &(mem->out);
	ir->enableWrite = &(seq->writeIr);
	
	zeroExt->in[0] = &(ir->out[0]);
	zeroExt->in[1] = &(ir->out[1]);
	
	pcMux->in[0] = &(pcAdder->out);
	pcMux->in[1] = &(regFile->rdData);
	pcMux->sel = &(branchAnd->out);
	
	regWriteDataMux->in[0] = &(zeroExt->out);
	regWriteDataMux->in[1] = &(mem->out);
	regWriteDataMux->in[2] = &(pcAdder->out);
	regWriteDataMux->in[3] = &(alu->result);
	regWriteDataMux->sel = &(seq->regFileWriteSrc);
	
	regFile->rs = &(ir->out[2]);
	regFile->rt = &(ir->out[1]);
	regFile->rd = &(ir->out[0]);
	regFile->writeData = &(regWriteDataMux->out);
	regFile->writeReg = &(seq->writeReg);
	regFile->data[12] = (memsize-1)*wordbytes; // initialize stack pointer value
	
	aluMux1->in[0] = &(zeroExt->out);
	aluMux1->in[1] = &(regFile->rtData);
	aluMux1->sel = &(seq->aluIn1);
	
	alu->in[0] = &(aluMux1->out);
	alu->in[1] = &(regFile->rsData);
	alu->aluOp = &(seq->aluOp);
	
	branchAnd->in[0] = &(seq->doBranch);
	branchAnd->in[1] = &(alu->jump_flag);
	
	seq->opcode = &(ir->out[3]);
	
}
/*------------------------------------------------------------------------------
	initMem
		allocate and initilize the memory array
	args
		pointer to cpu_t struct
		memory size in words
	returns
		none
------------------------------------------------------------------------------*/
void
initMem(cpu_t *cpu, int memsize)
{
	int i;
	
	cpu->mem.data = (int*)ezmalloc(memsize*sizeof(int));
	for (i=0; i<memsize; i++)
		cpu->mem.data[i] = 0;
}
/*------------------------------------------------------------------------------
	execute
		runs the execution cycle
	args
		pointer to cpu_t struct
	returns
		none
------------------------------------------------------------------------------*/
void
execute(cpu_t *cpu)
{
	instrFunc	this;
	
	if (debug)
		printf("Keep hitting Enter to go step by step...");
	
	while (1) {
		reset_control_signals(cpu);
		
		run_pc(cpu);
		
		run_pcAdder(cpu);
		
		run_branchAnd(cpu);
		
		run_pcMux(cpu);
		
		run_memAddrMux(cpu);
		
		run_mem(cpu);
		
		run_ir(cpu);
		
		this = getInstrCtrlFunc( *(cpu->seq.opcode) );
		
		cpu->seq.aluOp = *(cpu->seq.opcode);

		if (debug)
			printDebug(cpu);
		
		(*this)(cpu);
		
	}
	
}
/*------------------------------------------------------------------------------
	What follows is the definition of the functions that are responsible for 
	the execution path of different instruction groups.
	All of them are named in the "exec_<something>" format.
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
	exec_alu
		execution path for some ALU instructions
	args
		pointer to cpu_t struct
	returns
		none
------------------------------------------------------------------------------*/
void
exec_alu(cpu_t *cpu)
{
	sequencer_t	*seq	= &(cpu->seq);
	
	run_regFile(cpu);
	
	seq->aluIn1		= REG;
	seq->regFileWriteSrc	= ALU;
	seq->writeReg		= RD;	
			
	switch ( *(seq->opcode) ) {
		case ori:
		case lui:
			seq->aluIn1		= IMM;
			seq->writeReg		= RS;
			run_zeroExt(cpu);
			break;
	}
	
	run_aluMux1(cpu);
	run_alu(cpu);
	run_regWriteDataMux(cpu);
	run_regFile(cpu);
}
/*------------------------------------------------------------------------------
	exec_branch
		execution path for branch instructions
	args
		pointer to cpu_t struct
	returns
		none
------------------------------------------------------------------------------*/
void
exec_branch(cpu_t *cpu)
{
	sequencer_t	*seq	= &(cpu->seq);
	
	run_regFile(cpu);
	
	seq->aluIn1		= REG;
	seq->doBranch		= YES;
	seq->writePC		= YES;

	run_aluMux1(cpu);
	run_alu(cpu);
	run_branchAnd(cpu);
	run_pcMux(cpu);
	run_pc(cpu);
	
	if ( *(seq->opcode) == jrl ) {		// if jrl
		seq->regFileWriteSrc	= PCP1;	// save PC+1 in $ra
		seq->writeReg		= RA;
		run_regWriteDataMux(cpu);
		run_regFile(cpu);		
	}
}
/*------------------------------------------------------------------------------
	exec_lw
		execution path for instruction "lw"
	args
		pointer to cpu_t struct
	returns
		none
------------------------------------------------------------------------------*/
void
exec_lw(cpu_t *cpu)
{
	sequencer_t	*seq	= &(cpu->seq);
	
	run_regFile(cpu);
	
	seq->memAddrSrc		= REG;
	seq->writeReg		= RS;
	seq->regFileWriteSrc	= MEM;
				
	run_memAddrMux(cpu);
	run_mem(cpu);
	run_regWriteDataMux(cpu);
	run_regFile(cpu);
}
/*------------------------------------------------------------------------------
	exec_sw
		execution path for instruction "sw"
	args
		pointer to cpu_t struct
	returns
		none
------------------------------------------------------------------------------*/
void
exec_sw(cpu_t *cpu)
{
	sequencer_t	*seq	= &(cpu->seq);
	
	run_regFile(cpu);
	
	seq->memAddrSrc		= REG;
	seq->writeMem		= YES;
	
	run_memAddrMux(cpu);
	run_mem(cpu);
}
/*------------------------------------------------------------------------------
	reset_control_signals
		reset all control signal values to their default values
	args
		pointer to cpu_t struct
	returns
		none
------------------------------------------------------------------------------*/
void
reset_control_signals(cpu_t *cpu)
{
	sequencer_t	*seq	= &(cpu->seq);
	
	seq->writePC		= YES;
	seq->memAddrSrc		= PC;
	seq->writeMem		= NO;
	seq->writeIr		= YES;
	seq->doBranch		= NO;
	seq->regFileWriteSrc	= ALU;
	seq->writeReg		= NO;
	seq->aluIn1		= REG;
}
/*------------------------------------------------------------------------------
	What follows is the definition of the functions that are responsible for 
	data processing within each component and presenting the data on the output
	ports of the component.
	All of them are named in the "run_<device name>" format.
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
	run_pc
------------------------------------------------------------------------------*/
void
run_pc(cpu_t *cpu)
{
	pc_t *pc = &(cpu->pc);
	
	if ( *(pc->enableWrite) == YES )
		pc->value = *(pc->in);
	
	pc->out = pc->value;
}
/*------------------------------------------------------------------------------
	run_memAddrMux
------------------------------------------------------------------------------*/
void
run_memAddrMux(cpu_t *cpu)
{
	mux21_t *memAddrMux = &(cpu->memAddrMux);
	
	switch ( *(memAddrMux->sel) ) {
		case PC:
			memAddrMux->out = *(memAddrMux->in[0]);
			break;
		case REG:
			memAddrMux->out = *(memAddrMux->in[1]);
			break;
	}
	
}
/*------------------------------------------------------------------------------
	run_mem
------------------------------------------------------------------------------*/
void
run_mem(cpu_t *cpu)
{
	memory_t *mem = &(cpu->mem);
	struct termios orig, new;
	int addr = *(mem->addr) / 2;
	
	// we'll need to force terminal to accept keypress immediately when reading
	tcgetattr(0, &orig);
	new = orig;
	new.c_lflag &= ~ICANON;
	
	if ( *(mem->enableWrite) == YES ) {
		if ( *(mem->addr) == REG_IOBUFFER1 ) {
			printf("\e[7m%c\e[27m", *(mem->writeData));
			fflush(stdout);
		}
		else
			mem->data[addr] = *(mem->writeData);
	
	}
	else {
		if ( *(mem->addr) == REG_IOCONTROL )
			mem->out = 3; // first and second bit is always 1
		else if ( *(mem->addr) == REG_IOBUFFER1 ) {
			tcsetattr(0, TCSANOW, &new);
			mem->out = getchar();
			tcsetattr(0, TCSANOW, &orig);
		}
		else
			mem->out = mem->data[addr];
	}
	
}
/*------------------------------------------------------------------------------
	run_ir
------------------------------------------------------------------------------*/
void
run_ir(cpu_t *cpu)
{
	int i;
	int nybshft[IROUTPUTS] = {0, 4, 8, 12};
	int nybmask[IROUTPUTS] = {0x000F, 0x00F0, 0x0F00, 0xF000};
	ir_t *ir = &(cpu->ir);
	
	if ( *(ir->enableWrite) == YES )
		ir->value = *(ir->in);
	
	for (i=0; i<IROUTPUTS; i++)
		ir->out[i] = do_srl( (ir->value & nybmask[i]), nybshft[i] );
}
/*------------------------------------------------------------------------------
	run_pcAdder
------------------------------------------------------------------------------*/
void
run_pcAdder(cpu_t *cpu)
{
	in2out1_t *pcAdder = &(cpu->pcAdder);
	
	pcAdder->out = *(pcAdder->in[0]) + *(pcAdder->in[1]);
}
/*------------------------------------------------------------------------------
	run_zeroExt (concatenates two nybbles)
------------------------------------------------------------------------------*/
void
run_zeroExt(cpu_t *cpu)
{
	in2out1_t *zeroExt = &(cpu->zeroExt);
	
	zeroExt->out = *(zeroExt->in[0]) | ( *(zeroExt->in[1]) << 4 );
}
/*------------------------------------------------------------------------------
	run_regFile
------------------------------------------------------------------------------*/
void
run_regFile(cpu_t *cpu)
{
	regfile_t *regFile = &(cpu->regFile);
	
	switch ( *(regFile->writeReg) ) {
		case RS:
			regFile->data[*(regFile->rs)] = *(regFile->writeData);
			break;		
		case RT:
			regFile->data[*(regFile->rt)] = *(regFile->writeData);
			break;		
		case RD:
			regFile->data[*(regFile->rd)] = *(regFile->writeData);
			break;
		case RA:
			regFile->data[15] = *(regFile->writeData);
			break;
	}
	
	regFile->rsData = regFile->data[*(regFile->rs)];
	regFile->rtData = regFile->data[*(regFile->rt)];
	regFile->rdData = regFile->data[*(regFile->rd)];
}
/*------------------------------------------------------------------------------
	run_aluMux1
------------------------------------------------------------------------------*/
void
run_aluMux1(cpu_t *cpu)
{
	mux21_t *aluMux1 = &(cpu->aluMux1);
	
	switch ( *(aluMux1->sel) ) {
		case IMM:
			aluMux1->out = *(aluMux1->in[0]);
			break;
		case REG:
			aluMux1->out = *(aluMux1->in[1]);
			break;
	}
}
/*------------------------------------------------------------------------------
	run_alu
------------------------------------------------------------------------------*/
void
run_alu(cpu_t *cpu)
{
	alu_t *alu = &(cpu->alu);
	int shamt;
	
	switch ( *(alu->aluOp) ) {
		case add:
			alu->result = ( (short)*(alu->in[0]) + (short)*(alu->in[1]) ) & MASK16;
			break;
		case and:
			alu->result = *(alu->in[0]) & *(alu->in[1]);
			break;
		case beq:
			alu->jump_flag = *(alu->in[0]) == *(alu->in[1]) ? 1 : 0;
			break;
		case jr:
		case jrl:
			alu->jump_flag = 1;
			break;
		case lui:
			alu->result = *(alu->in[0]) << (WORDSIZE/2);
			break;
		case bsr:
			shamt = *(alu->in[1]) & 0x000F;
			alu->result = ( do_srl(*(alu->in[0]), shamt) ) | ( *(alu->in[0]) << (WORDSIZE-shamt) );
			break;
		case nor:
			alu->result = (~( *(alu->in[0]) | *(alu->in[1]) ) ) & MASK16;
			break;
		case or:
		case ori:
			alu->result = *(alu->in[0]) | *(alu->in[1]);
			break;
		case sll:
			shamt = *(alu->in[1]) & 0x000F;
			alu->result = *(alu->in[0]) << shamt;
			break;
		case slt:
			alu->result = (short)*(alu->in[1]) < (short)*(alu->in[0]) ? 1 : 0;
			break;
		case xor:
			alu->result = *(alu->in[1]) ^ *(alu->in[0]);
			break;	
		case srl:
			shamt = *(alu->in[1]) & 0x000F;
			alu->result = do_srl( *(alu->in[0]), shamt );
			break;						
	}
	
}

/*------------------------------------------------------------------------------
	run_regWriteDataMux
------------------------------------------------------------------------------*/
void
run_regWriteDataMux(cpu_t *cpu)
{
	mux41_t *regWriteDataMux = &(cpu->regWriteDataMux);
	
	switch ( *(regWriteDataMux->sel) ) {
		case IMM:
			regWriteDataMux->out = *(regWriteDataMux->in[0]);
			break;
		case MEM:
			regWriteDataMux->out = *(regWriteDataMux->in[1]);
			break;
		case PCP1:
			regWriteDataMux->out = *(regWriteDataMux->in[2]);
			break;
		case ALU:
			regWriteDataMux->out = *(regWriteDataMux->in[3]);
			break;
	}
}
/*------------------------------------------------------------------------------
	run_branchAnd
------------------------------------------------------------------------------*/
void
run_branchAnd(cpu_t *cpu)
{
	in2out1_t *branchAnd = &(cpu->branchAnd);
	
	branchAnd->out = *(branchAnd->in[0]) & *(branchAnd->in[1]);
}
/*------------------------------------------------------------------------------
	run_pcMux
------------------------------------------------------------------------------*/
void
run_pcMux(cpu_t *cpu)
{
	mux21_t *pcMux = &(cpu->pcMux);
	
	if ( *(pcMux->sel) )
		pcMux->out = *(pcMux->in[1]);
	else
		pcMux->out = *(pcMux->in[0]);
}
 
/*------------------------------------------------------------------------------
	printDebug
		print debug info when requested by [-d] flag
		shows modified register in red
		shows disassembled hardware instruction
		shows MIF line
		shows stack contents
	args:
		pointer to cpu_t struct
	returns
		none
------------------------------------------------------------------------------*/
void
printDebug(cpu_t *cpu)
{
	int i, sp, imm;
	char *name, *oper, *arg1, *arg2, *arg3 ;
	regfile_t *regFile = &(cpu->regFile);
	pc_t *pc = &(cpu->pc);
	ir_t *ir = &(cpu->ir);
	memory_t *mem = &(cpu->mem);
	static regfile_t prev_regfile;
	
	getchar(); // pause for enter to step through instructions	
	
	printf("============================================================================================================================");
	printf("\n%-10s","Register");
	for (i=0; i<REGCOUNT; i++) {
		name = getRegName(i);
		if (prev_regfile.data[i] != regFile->data[i])
			printf("|"REDSTR, name);
		else
			printf("|%-5s", name);
	}
	printf("\n%-10s","Value");
	for (i=0; i<REGCOUNT; i++) {
		if (prev_regfile.data[i] != regFile->data[i])
			printf("|"REDHEX, regFile->data[i]);
		else
			printf("|%-5X", regFile->data[i]);
	}
	printf("\n----------------------");
	printf("\n%-10s%-5X\n----------------------", "PC = ", pc->out/2);
	
	oper = getOperName(ir->out[3]);
	switch (ir->out[3]) {
		case add:
		case and:
		case beq:
		case bsr:
		case nor:
		case or:
		case sll:
		case slt:
		case xor:
		case srl:
			arg1 = getRegName(ir->out[2]);
			arg2 = getRegName(ir->out[1]);
			arg3 = getRegName(ir->out[0]);
			printf("\n%-10s%-5s %-5s %-5s %-5s\n", "Instr = ", oper, arg1, arg2, arg3);
			break;
		case jr:
		case jrl:
			arg1 = getRegName(ir->out[0]);
			printf("\n%-10s%-5s %-5s\n", "Instr = ", oper, arg1);
			break;
		case lui:
		case ori:
			arg1 = getRegName(ir->out[2]);
			imm = (ir->out[1] << 4) | ir->out[0];
			printf("\n%-10s%-5s %-5s 0x%-5X\n", "Instr = ", oper, arg1, imm);
			break;
		case sw:
		case lw:
			arg1 = getRegName(ir->out[2]);
			arg2 = getRegName(ir->out[1]);
			printf("\n%-10s%-5s %-5s %-5s\n", "Instr = ", oper, arg1, arg2);
			break;
	}
	
	printf("----------------------\n");
	printf("%-10s","Stack = ");
	sp = regFile->data[12]/wordbytes;
	while (sp < memsize - 1) {
		printf("|%-5X ", mem->data[sp]);
		sp += 1;
	};
	printf("\n");
	
	prev_regfile = *regFile;
}
/*------------------------------------------------------------------------------
	getRegName
		get register name from its number
	args:
		none
	returns
		none
------------------------------------------------------------------------------*/
char*
getRegName(int num)
{
	int i;
	char *name = NULL;
	
	for (i=0; i<REGCOUNT; i++) {
		if ( reglib[i].num == num ) {
			name = reglib[i].symbol;
			break;
		}
	}
	
	return name;
}
/*------------------------------------------------------------------------------
	getOperName
		get operation name from its opcode
	args:
		none
	returns
		none
------------------------------------------------------------------------------*/
char*
getOperName(int num)
{
	int i;
	char *name = NULL;
	
	for (i=0; i<OPERCOUNT; i++) {
		if ( operlib[i].opcode == num ) {
			name = operlib[i].symbol;
			break;
		}
	}
	
	return name;
}
 /*------------------------------------------------------------------------------
	do_srl
		shift right logical to make sure C doesnt do arithmetic shift
	args
		number to be shifted
		shift amount
	return
		shifted number
------------------------------------------------------------------------------*/
int
do_srl(int num, int shamt)
{	
	int result;
	
	if (shamt == 0)
		result = num;
	else
		result = ( (num>>1) & SIGNKILL ) >> (shamt-1);
	
	return result;
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
str2num(char *str, int base)
{
	long int	result;
	char	**endptr = &str;

	errno = 0;
	result = strtol(str, endptr, base);
	//if ( errno != 0 || **endptr != '\0')
	//	success = NULL;
	
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
/*------------------------------------------------------------------------------
	stripComment
		remove comments from line (anything after the first occurence of "--")
	args
		pointer to line 
	returns
		pointer to commentless line
------------------------------------------------------------------------------*/
char*
stripComment(char* str)
{
	unsigned int i, end = strlen(str);

	for (i=0; i<strlen(str); i++) {
		if ( strncmp(str+i, "--", 2) == 0 ) {
			end = i;
			break;
		}
	}

	*(str + end) = '\0';
	
	return str;
}
/*------------------------------------------------------------------------------
	getInstrCtrlFunc
		fetch instruction execution function from its opcode
	args
		instruction opcode
	returns
		pointer to instrFunc
------------------------------------------------------------------------------*/
instrFunc
getInstrCtrlFunc(int opcode)
{
	int i;
	instrFunc result;
	
	for (i=0; i<OPERCOUNT; i++) {
		if (opcode == operlib[i].opcode) {
			result = operlib[i].execfunc;
			break;
		}
	}
	
	return result;
}

