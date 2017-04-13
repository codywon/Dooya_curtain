
                AREA    |.text|, CODE, READONLY
	 EXPORT  bb_enter_prog			
	 ALIGN	
bb_enter_prog PROC
	ldr	r1, [r0]
	mov	sp, r1
	ldr	r2, [r0, #4]
	push	{r2}
	mov.w	r1, #0
	mov	r0, r1
	mov	r2, r1
	mov	r3, r1
	mov	r4, r1
	mov	r5, r1
	mov	r6, r1
	mov	r7, r1
	mov	r8, r1
	mov	r9, r1
	mov	r10, r1
	mov	r11, r1
	mov	r12, r1
	mvn	lr, r1
	pop	{pc}
     ENDP
	 END