#-------------------------------------------------------------------------------
#	Red Pike encryption/decryption program written in mf16 assembly code.
#
#	The user is prompted to enter two pin numbers between -32768 and 32767
#	after which they are used to encrypt in-place a portion of the memory labeled with
#	"SECRET". The user is then prompted to enter two pins again continuosly.
#	Every time the pins are correct, the message will be decryppted and printed to
#	the terminal otherwise some gibberish is printed.
#
#	For details on the Red Pike algorithm, please see the link below:
#		https://en.wikipedia.org/wiki/Red_Pike_(cipher)
#-------------------------------------------------------------------------------
#
#
#
SECRET:	.asciiz THIS MESSAGE WAS ENCRYPTED

PIN1:	.asciiz Enter an integer as the 1st pin:
PIN2:	.asciiz Enter an integer as the 2nd pin:
MSG1:	.asciiz Encrypting . . .
MSG2:	.asciiz Decrypting . . .

#-----------------------
# Prompting the user for pins to encrypt
#
MAIN:	li $a1, %MSG1		# save MSG1 address into $a1
		call %WSTR			# call WSTR with $a1

		li $a1, %PIN1		# save PIN1 prompt address into $a1
		call %WSTR			# call WSTR with $a1

		call %RINT			# read first pin
		push $rv

		li $a1, %PIN2		# save PIN2 prompt address into $a1
		call %WSTR			# call WSTR with $a1

		call %RINT			# read second pin
		push $rv

		pop $s3				# restore pin2
		pop $s2				# restore pin1

		li $s0, %SECRET		# load address of secret message

MAIN1:	lw $a0, $s0				# while secret message is not finished
		jeq $a0, $0, %MAIN2
		addi $s0, 0x2, $s1
		lw $a1, $s1
		jeq $a1, $0, %MAIN2
		call %RPENC				# call encryption function
		sw $a0, $s0
		sw $a1, $s1
		addi $s0, 0x4, $s0		# move to the next two memory locations
		jmp %MAIN1

#-----------------------
# Decryption stage
#
MAIN2:	li $a1, %MSG2		# save MSG2 address into $a1
		call %WSTR			# call WSTR with $a1

		li $a1, %PIN1		# save PIN1 prompt address into $a1
		call %WSTR			# call WSTR with $a1

		call %RINT			# read first pin
		push $rv

		li $a1, %PIN2		# save PIN2 prompt address into $a1
		call %WSTR			# call WSTR with $a1

		call %RINT			# read second pin
		push $rv

		pop $s3				# restore pin2
		pop $s2				# restore pin1

		li $s0, %SECRET

MAIN3:	lw $a0, $s0				# while secret message is not finished
		jeq $a0, $0, %MAIN4
		addi $s0, 0x2, $s1
		lw $a1, $s1
		jeq $a1, $0, %MAIN4
		call %RPDEC				# call decryption function
		call %WCHAR
		add $a1, $0, $a0
		call %WCHAR
		addi $s0, 0x4, $s0		# move to the next two memory locations
		jmp %MAIN3

MAIN4:	li $a0, 0xa
		call %WCHAR
		jmp %MAIN2

#-----------------------------------------
# Red pike encrypt
#	a0 = first word of data block
#	a1 = second word of data block
#	s2 = first word of key
#	s3 = second word of key
#
#	updates a0 and a1 with encrypted values in place

RPENC:	push $s2
		push $s3
		li $t3, 0x10
RPENC1:	li $t2, 0x37		# a constant
		add $s2, $t2, $s2
		sub $s3, $t2, $s3
		xor $a0, $s2, $a0
		add $a0, $a1, $a0
		push $a1
		nor $a1, $0, $a1
		addi $a1, 0x1, $a1
		bsr $a1, $a0, $a0
		pop $a1
		bsr $a0, $a1, $a1
		sub $a1, $a0, $a1
		xor $a1, $s3, $a1
		addi $t3, 0xffff, $t3
		jeq $t3, $0, %RPENC2
		jmp %RPENC1

RPENC2:	add $a0, $0, $s2
		add $a1, $0, $a0
		add $s2, $0, $a1
		pop $s3
		pop $s2
		jr $ra

#-----------------------------------------
# Red pike decrypt
#	a0 = first word of encrypted block
#	a1 = second word of encrypted block
#	s2 = first word of key
#	s3 = second word of key
#
#	updates a0 and a1 with decrypted values in place

RPDEC:	push $s2
		push $s3
		li $t2, 0x3a7		# a constant
		push $s3
		add $s2, $t2, $s3
		pop $t3
		li $t2, 0xfc59		# a constant
		add $t3, $t2, $s2
		call %RPENC
		pop $s3
		pop $s2
		jr $ra

#-------------------------------------------------------------------------------
# WCHAR subroutine
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
# RCHAR subroutine
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
# WSTR subroutine
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
# RINT subroutine
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
# MULT subroutine
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