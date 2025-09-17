	lw 0 1 three
	lw 0 2 neg1
loop	add 1 2 1
	beq 0 1 loop
	beq 0 0 done
done	halt
three	.fill 3
neg1	.fill -1