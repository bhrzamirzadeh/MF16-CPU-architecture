#include	<stdio.h>
#include	<stdlib.h>
#include	"mf16emulib.h"

/*******************************************************************************
	mf16emu.c
		emulator for mf16 instruction set
	Usage
		./mf16emu <path to MIF file> [-d]
		
		The "-d" option runs the emulator in the debug mode with stepping.

 ******************************************************************************/

int
main(int ac, char *av[])
{
	FILE *fp;
	char *line, *tok[MAXTOKEN];
	int i, format = 0, matchNum;
	unsigned int addr, data;
	char *addr_radix = NULL, *data_radix = NULL;
	
	cpu_t	cpu; // create cpu object

	debug = 0;
	memsize = 0;
	linenum = 0;
	
	// initialize all cpu fields to zero
	memset(&cpu, 0, sizeof(cpu_t));
	
	initParams(); // initialize mf16 parameters
	
	line = (char*)ezmalloc(MAXLINELEN+1); // buffer to hold current line being read from file
	for (i=0; i<MAXTOKEN; i++)
		tok[i] = (char*)ezmalloc(MAXTOKENLEN+1);
	

	// verify command line args
	if (ac == 3) {
		if ( strcmp(av[2], "-d") == 0 )
			debug = 1;
	}
	else if (ac < 2 || ac > 3)
		fatal(0, "Usage: mf16emu <path to MIF file> [-d]", "");
	
	
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
		
		// check the line against all possible formats
		for (i=0; i<LINEFMTTYPES; i++) {
			matchNum = sscanf(line, lineformat[i].format, tok[0], tok[1]);
			// save the format if matched
			if (matchNum == lineformat[i].tokcount) {
				format = lineformat[i].id;
				break;
			}
		}
		
		if (format == 1)
			memsize = str2num(tok[0], 10);
		else if (format == 2)
			addr_radix = tok[0];
		else if (format == 3)
			data_radix = tok[0];
		
		if (memsize && addr_radix && data_radix)
			break;
	}

	initMem(&cpu, memsize); // allocate and inititlize memory
	
	wireUp(&cpu); // connect all components input and outputs according to diagram
	
	// continue reading the file line by line
	while ( (line = fgets(line, MAXLINELEN, fp)) != NULL ) {

		linenum++;
		
		line = stripComment(line); // wipe the comments
		
		if (is_blank(line))	// if line is whitespace
			continue;	// ignore it	
		
		// check the line against MIF data section format
		matchNum = sscanf(line, DATAFMT, &addr, &data);
		
		if (matchNum == 2)			// if matched
			cpu.mem.data[addr] = data;	// put in memory
		else
			continue;
	}
	
	execute(&cpu);	// start execution

	return 0;

}
