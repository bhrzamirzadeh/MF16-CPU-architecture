#include	<stdio.h>
#include	<stdlib.h>
#include	"mf16asmlib.h"

/*******************************************************************************
	mf16asm.c
		assembler for mf16 instruction set
	args
		path to a single file containing mf16 assembly code
	returns
		MIF file printed to stdout
	Notes
		see MF16 instruction set user guide for syntax and feature details
 ******************************************************************************/

int
main(int ac, char *av[])
{
	FILE *fp;
	char *line;
	int i, matchNum, linenum = 0;
	instruction_t  *this;

	initParams(); // initialize mf16 parameters
	
	line = (char*)ezmalloc(MAXLINELEN+1); // buffer to hold current line being read from file

	// verify command line args
	if (ac != 2)
		fatal(0, "You must provide one and only one file name.", "");
	
	// open input file
	if ( (fp = fopen(av[1], "r")) == NULL) {
		perror("fopen");
		fatal(0, "could not open file ", av[1]);
	}

	// read file line by line
	while ( (line = fgets(line, MAXLINELEN, fp)) != NULL ) {

		linenum++;
		
		line = stripComment(line); // wipe the comments
		
		if (is_blank(line))	// if line is whitespace
			continue;	// ignore it	
		
		this = initInstr();	// initialize an instruction
		
		this->line = (char*)ezmalloc(MAXLINELEN+1);
		strcpy(this->line, trimWhite(line));	// save line text
		
		// check the line against all possible formats
		for (i=0; i<LINEFMTTYPES; i++) {
			matchNum = sscanf(line, lineformat[i].format, this->tok[0], this->tok[1], this->tok[2], this->tok[3], this->tok[4]);
			// save the format if matched
			if (matchNum == lineformat[i].tokcount) {
				this->format = lineformat[i].id;
				break;
			}
		}
		
		this->linenum = linenum; // save line number
		
		if (this->format == UNKNOWN)					// if no format was matched
			fatal(linenum, "invalid instruction format", line);	// error
		else if (this->format > 1) {					// if non directive
			this = insertInstr(this, &instrTail, INSTR);		// insert instr into list
			if (!instrHead)						// assign list head
				instrHead = this;
			setTokens(this);					// set operation type and arg tokens
			parseArgs(this);					// validate and parse args
			pc += this->operation->instrCount;			// adjust pc for pseudo instructions
		}
		else {								// if directive format
			this = insertInstr(this, &direcTail, DIREC);		// insert into directive list
			if (!direcHead)						// assign list head
				direcHead = this;
		}

	}

	this = direcHead;
	while (this) {				// for each directive in directive list
		addLabel(this);			// add its label to label list
		pc += strlen(this->tok[2])+2;	// adjust pc based on directive data length + (null and \n)
		this = this->next;
	}
	
	this = instrHead;
	do {					// for each instruction in instruction list
		parseLabel(this);		// translate label usage (%label) into its address
	} while ( (this = this->next) );

	do {					// start expanding pseudo instructions
		pseudo_left = NO;		
		this = instrHead;
		do {				// for each instruction in instruction list
			expandPseudo(this);	// expand it if pseudo instruction
		} while ( (this = this->next) );
	} while (pseudo_left == YES);		// until no pseudo instruction left in list

	this = instrHead;
	do {					// for each instruction in instruction list
		encodeInstr(this);		// encode it
	} while ( (this = this->next) );

	printMIF();				// print out put in MIF format to stdout

	return 0;

}
