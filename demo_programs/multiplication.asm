#-------------------------------------------------------------------------------
#	Integer multiplication program written in mf16 assembly code
#
#	The user is prompted to enter two integers between -32768 and 32767
#	after which their product is calculated and printed to the serial terminal.
#
#	(the question number in front of each function name is related to PS4)
#-------------------------------------------------------------------------------
# MAIN program (question 9)
#
#
#

STR1:	.asciiz Enter 1st number:
STR2:	.asciiz Enter 2nd number:
STR3:	.asciiz The product is:

MAIN:	li $a1, %STR1		# save STR1 address into $a1
		call %WSTR			# call WSTR with $a1

		call %RINT			# read first integer from serial input
		push $rv

		li $a1, %STR2		# save STR2 address into $a1
		call %WSTR			# call WSTR with $a1

		call %RINT			# read second integer from serial input
		push $rv

		pop $a1
		pop $a0
		call %MULT			# perform multiplication
		push $rv

		li $a1, %STR3		# save STR3 label address in $a1
		call %WSTR			# call WSTR with $a1

		pop $a0
		call %WINT			# print the result

		jmp %MAIN			# repeat the program

#-------------------------------------------------------------------------------
# WCHAR subroutine (question 2)
#
#	write a single character from register $a0 to serial output
#	args:	$a0 = single character to write
#	return:	none
#

WCHAR:	li $t0, 0xff00		# save REG_IOCONTROL address into $t0
		lw $t1, $t0			# read REG_IOCONTROL value from memory into $t1
		andi $t1, 0x2, $t1	# test if BIT_SERIAL_OUTPUTREADY high and save into $t1
		jeq $t1, $0, %WCHAR	# branch back to WCHAR if not
		li $t0, 0xff04		# save REG_IOBUFFER1 address into $t0
		sw $a0, $t0			# write subroutine arg in $a0 into REG_IOBUFFER1
		jr $ra				# return from subroutine

#-------------------------------------------------------------------------------
# RCHAR subroutine (question 3)
#
#	read a single character from serial input to register $rv
#	args: 	none
#	return:	$rv = single character read
#

RCHAR:	li $t0, 0xff00			# save REG_IOCONTROL address into $t0
		lw $t1, $t0				# read REG_IOCONTROL value from memory into $t1
		andi $t1, 0x1, $t1		# test if BIT_SERIAL_OUTPUTREADY high and save into $t1
		jeq $t1, $0, %RCHAR		# branch back to RCHAR if not
		li $t0, 0xff04			# save REG_IOBUFFER1 address into $t0
		lw $rv, $t0				# read REG_IOBUFFER1 into $rv
		jr $ra					# return from subroutine

#-------------------------------------------------------------------------------
# WSTR subroutine (question 4)
#
#	write a null terminated string of characters from memory to serial output
#	args:	$a1 = memory address of the beginning of string
#	return:	none
#

WSTR:	lw $a0, $a1				# read a word from memory at $a1 into $a0
		jeq $a0, $0, %WSTR1		# jump out of subroutine if character is null
		call %WCHAR				# call WCHAR and write the char in $a0 to output
		addi $a1, 0x2, $a1		# increment $a1 to next word address
		jmp %WSTR				# jump back for next character
WSTR1:	jr $ra					# return from subroutine

#-------------------------------------------------------------------------------
# RSTR subroutine (question 5)
#
#	read a string of characters from serial input and store in memory
#	args:	$a0 = memory address of the beginning of string
#	return:	none
#

RSTR:	li $t0, 0xa		# save \n value into $t0
		call %RCHAR		# read one char
		sw $rv, $a0		# store the char in memory at $a0
		li $t1, 0x2		# set $t1 to 2
		add $a0, $t1, $a0	# increment $a0 to next word address
		jeq $rv, $t0, %RSTR1	# if $rv is line-feed go to NEXT2
		jmp %RSTR		# otherwise go back to RSTR
RSTR1:	jr $ra			# return from subroutine

#-------------------------------------------------------------------------------
# WINT subroutine (question 6)
#
#	write the signed integer in $a0 to serial output
#	args:	$a0 = signed integer value
#	return:	none
#	Note: currently prints leading zeros
#

WINT:	push $s0				# backup $s0 on stack
		push $0					# put null on stack
		li $t0, 0x8000			# set $t0 to sign bit mask
		and $a0, $t0, $s0		# test if sign bit in $a0 is high and save into $s0
		jeq $s0, $t0, %WINT1	# if sign bit is set go and negate the number
		jmp %WINT2				# otherwise skip negation

WINT1:	nor $a0, $0, $a0		# negate $a0 by inverting
		addi $a0, 0x1, $a0		# and adding 1		

WINT2:	li $a1, 0xa				# set a1=10
		call %MOD				# call modulo and extract the digit
		li $t0, 0x30			# save '0' in $t0
		add $rv, $t0, $rv		# convert int to char by adding '0'
		push $rv				# save current char on stack
		push $a0				# save $a0 on stack
		call %DIV				# get quotient of $a0/10
		pop $a0					# restore $a0
		add $rv, $0, $a0		# and update into $a0
		jeq $a0, $0, %WINT3		# jump out of this loop if no more digits left
		jmp %WINT2

WINT3:	li $t0, 0x8000			# set $t0 to sign bit mask
		jeq $s0, $t0, %WINT4	# if sign bit is set go and add '-' on to the stack
		jmp %WINT5				# otherwise skip it

WINT4:	li $t0, 0x2d			# save '-' in $t0
		push $t0				# and put it on stack

WINT5:	pop $a0					# get a char from stack into $a0
		jeq $a0, $0, %WINT6		# jump out if null
		call %WCHAR				# otherwise print it
		jmp %WINT5				# go to next char

WINT6:	li $a0, 0xa				# put '\n' into $a0
		call %WCHAR				# write a final new line char
		pop $s0					# restore $s0
		jr $ra					# exist subroutine

#-------------------------------------------------------------------------------
# RINT subroutine (question 7)
#
#	read a signed integer from serial input
#	args:	none
#	return:	$rv = signed integer value
#

RINT:	push $s0				# save $s0 onto stack
		add $0, $0, $s0			# set result to 0 (s0=0)
		push $0					# first save a null char on stack
		add $0, $0, $rv			# set $rv to 0
		li $a0, 0x1				# set a0=1 (1's place factor)
RINT1:	call %RCHAR				# read a character from serial input
		li $t0, 0xa				# save '\n' code into $t0
		jeq $rv, $t0, %RINT2	# if it is '\n' skip to RINT2
		push $rv				# save this character onto stack
		jmp %RINT1				# go read next character

RINT2:	pop $a1					# get one from stack into $a1
		jeq $a1, $0, %RINT4		# exit subroutine if it is null
		li $t3, 0x2d			# save '-' in $t3
		jeq $a1, $t3, %RINT3	# go and set sign if '-'
		li $t0, 0x30			# save '0' in $t0
		sub $a1, $t0, $a1		# convert char to int by subtracting '0'
		push $a0				# save factor on stack
		call %MULT				# multiply factor and current digit
		pop $a0					# restore factor from stack
		add $rv, $s0, $s0		# add to final result
		slli 0x1, $a0, $t0		# multiply factor by 10 by doing left shift 1
		slli 0x3, $a0, $a0		# and left shift 3
		add $a0, $t0, $a0		# and then adding them
		jmp %RINT2				# go process the next character

RINT3:	nor $s0, $0, $rv		# negate the result by inverting
		addi $rv, 0x1, $rv		# and adding 1
		pop $s0					# dont forget to pop that initial null
		pop $s0					# restore $s0
		jr $ra					# jump out	

RINT4:	add $s0, $0, $rv		# negate the result by inverting
		pop $s0					# restore $s0
		jr $ra					# jump out	
#-------------------------------------------------------------------------------
# MULT subroutine (question 8)
#
#	perform multiplication (a * b)
#	args:	$a0 = a (multiplicand)
#	$a1 = b (multiplier)
#	return:	$rv = product
#

MULT:	add $0, $0, $rv			# set $rv to 0
		push $s0				# backup $s0
		addi $0, 0x1, $s0		# assume result is negative and set sign in $s0
		li $t3, 0x8000			# set $t3 to sign bit mask
		and $a0, $t3, $t0		# test if sign bit in $a0 is high and save into $t0
		jeq $t0, $t3, %MULT2	# if sign bit set, go and negate 

MULT3:	and $a1, $t3, $t1		# test if sign bit in $a1 is high and save into $t1
		jeq $t1, $t3, %MULT4	# adjust sign if signs of $a0 and $a1 are the same
		jmp %MULT10				# othersiwse skip to rsult sign adjustment

MULT2:	nor $a0, $0, $a0		# negate $a0 by inverting
		addi $a0, 0x1, $a0		# and adding 1
		jmp %MULT3				# jump back and continue

MULT4:	nor $a1, $0, $a1		# negate $a1 by inverting
		addi $a1, 0x1, $a1		# and adding 1

MULT10:	jeq $t0, $t1, %MULT5	# if signs are the same go and flip the sign bit
		jmp %MULT6				# other wise continue

MULT5:	add $0, $0, $s0			# set sign to positive in $s0

MULT6:	jeq $a1, $0, %MULT8		# return if multiplier is 0
		andi $a1, 0x1, $t2		# test lowest bit of b for 1 and set $t2 to 1 
		jeq $t2, $0, %MULT7		# if lowest bit of b is 0 and skip adding to $rv
		add $rv, $a0, $rv		# add $a0 to $rv
MULT7:	slli 0x1, $a0, $a0		# logical shift $a0 left 1 bit
		srli 0x1, $a1, $a1		# logical shift $a1 right 1 bit
		jmp %MULT6				# go to next bit

MULT8:	jeq $s0, $0, %MULT9		# if sign is positive, exit
		nor $rv, $0, $rv		# otherwise negate $rv by inverting
		addi $rv, 0x1, $rv		# and adding 1

MULT9:	pop $s0					# restore $s0
		jr $ra					# exit subroutine

#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
#
# The subroutines below are used as helper functions in WINT and RINT.
#
#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
#	perform division (a / b)
#	args:	$a0 = a
#	$a1 = b
#	return:	$rv = quotient
#

DIV:	add $0, $0, $rv		# set $rv to 0
		addi $0, 0x1, $t1	# set $t1 to 1
DIV1:	slt $a0, $a1, $t0	# if $a0 < $a1 set $t0 to 1
		beq $t0, $t1, $ra	# if $t0 = $t1 return from subroutine
		add $rv, $t1, $rv	# increment $rv by one
		sub $a0, $a1, $a0	# calculate ($a0 - $a1) and store the result in $a0
		jmp %DIV1			# jump back to DIV1

#-------------------------------------------------------------------------------
#	perform modulo (a % b)
#	args:	$a0 = a
#	$a1 = b
#	return:	$rv = modulo
#

MOD:	add $a0, $0, $rv	# copy $a0 to $rv
		slt $rv, $a1, $t0	# if $a0 < $a1 set $t0 to 1
		addi $0, 0x1, $t1	# set $t1 to 1
		beq $t0, $t1, $ra	# if $t0 = $t1 return from subroutine
MOD1:	sub $rv, $a1, $rv	# calculate ($rv - $a1) and store the result in $a0
		slt $rv, $a1, $t2	# if $rv < $a1 set $t2 to 1
		beq $t2, $t1, $ra	# if $t2 = $t1 return from subroutine
		jmp %MOD1			# jump back to MOD1