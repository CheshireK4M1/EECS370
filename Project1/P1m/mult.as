	lw	0	1	mcand
	lw	0	2	mplier
	lw	0	4	one	masking bit to be left shifted
	lw	0	5	ffzehn	counter for left shifting
loop	nor	4	4	4	now reg4 is 0xFFFFFFFE, or say 1
	nor	2	2	6	reg6 is the negation of mplier
	nor	4	6	7	reg7 now has LSB of mcand
	nor	4	4	4	revert reg4 value back into 1
	beq	4	7	incre	
	beq	0	7	shift
incre	add	1	3	3	add the mcand number to reg3
shift	add	1	1	1	shift the mcand left by 1 bit	
	lw	0	7	neg1	reg7 will be used to decrement counter
	add	5	7	5	decrement reg5 by -1
	beq	0	5	end	stop running and return the value if counting ends
	beq	0	0	loop
end	halt
mcand 	.fill	3		
mplier	.fill	5	
one	.fill	1	
ffzehn	.fill	15
neg1	.fill	-1
