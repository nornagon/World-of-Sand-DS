@ stolen from tonclib

@ void memset32(void *dst, u32 val, u32 n)
  .section .itcm,"ax", %progbits
  .align 2
  .code 32
  .global memset32
memset32:
  and   r12, r2, #7
  movs  r2, r2, lsr #3
  beq  .Lres_set32
  stmfd sp!, {r4-r10}
  mov   r3, r1 @ r3 = r1
  mov   r4, r1
  mov   r5, r1
  mov   r6, r1
  mov   r7, r1
  mov   r8, r1
  mov   r9, r1
  mov   r10, r1
.Lmain_set32:
    stmia r0!, {r3-r10}
    subs r2, r2, #1
    bhi .Lmain_set32
  ldmfd sp!, {r4-r10}
.Lres_set32:
    subs    r12, r12, #1
    stmcsia r0!, {r1}
    bcs     .Lres_set32
  bx    lr

@ void memset16(void *dst, u16 val, u32 n)
	.text
	.align 2
	.code 16
	.global memset16
	.thumb_func
memset16:
	push	{r4, lr}
	@ under 6 hwords -> std set
	cmp		r2, #5
	bls		.Ltail_set16
	@ dst not word aligned: copy 1 hword and align
	lsl		r3, r0, #31
	bcc		.Lmain_set16
		strh	r1, [r0]
		add		r0, #2
		sub		r2, r2, #1
	@ Again, memset32 does the real work
.Lmain_set16:
	lsl		r4, r1, #16
	orr		r1, r4
	lsl		r4, r2, #31
	lsr		r2, r2, #1
	ldr		r3, .Lpool_set16
	bl		_call_via_r3
	@ NOTE: r0 is altered by memset32, but in exactly the right 
	@ way, so we can use is as is. r1 is now doubled though.
	lsr		r2, r4, #31
	beq		.Lend_set16
	lsr		r1, #16
.Ltail_set16:
	sub		r2, #1
	bcc		.Lend_set16		@ r2 was 0, bug out
	lsl		r2, r2, #1
.Lres_set16:
		strh	r1, [r0, r2]
		sub		r2, r2, #2
		bcs		.Lres_set16
.Lend_set16:
	pop		{r4}
	pop		{r3}
	bx	r3
	.align 2
.Lpool_set16:
	.word memset32


@ void memcpy32(void *dst, const void *src, u32 n)
	.section .itcm,"ax", %progbits
	.align	2
	.code	32
	.global	memcpy32
memcpy32:
	and		r12, r2, #7
	movs	r2, r2, lsr #3
	beq		.Lres_cpy32
	stmfd	sp!, {r4-r10}
	@ copy 32byte chunks with 8fold xxmia
.Lmain_cpy32:
		ldmia	r1!, {r3-r10}	
		stmia	r0!, {r3-r10}
		subs	r2, r2, #1
		bhi		.Lmain_cpy32
	ldmfd	sp!, {r4-r10}
	@ and the residual 0-7 words
.Lres_cpy32:
		subs	r12, r12, #1
		ldmcsia	r1!, {r3}
		stmcsia	r0!, {r3}
		bcs		.Lres_cpy32
	bx	lr


@void memcpy16(void *dst, const void *src, u32 n)
	.text
	.align	2
	.code	16
	.global memcpy16
	.thumb_func
memcpy16:
	push	{r4, lr}
	@ under 5 hwords -> std cpy
	cmp		r2, #5
	bls		.Ltail_cpy16
	@ unreconcilable alignment -> std cpy
	@ if (dst^src)&2 -> alignment impossible
	mov		r3, r0
	eor		r3, r1
	lsl		r3, r3, #31		@ (dst^src), bit 1 into carry
	bcs		.Ltail_cpy16	@ (dst^src)&2 : must copy by halfword
	@ src and dst have same alignment -> word align
	lsl		r3, r0, #31
	bcc		.Lmain_cpy16	@ ~src&2 : already word aligned
	@ aligning is necessary: copy 1 hword and align
		ldrh	r3, [r1]
		strh	r3, [r0]
		add		r0, #2
		add		r1, #2
		sub		r2, r2, #1
	@ right, and for the REAL work, we're gonna use memcpy32
.Lmain_cpy16:
	lsl		r4, r2, #31
	lsr		r2, r2, #1
	ldr		r3, .Lpool_cpy16
	bl		_call_via_r3
	@ NOTE: r0,r1 are altered by memcpy32, but in exactly the right 
	@ way, so we can use them as is.
	lsr		r2, r4, #31
	beq		.Lend_cpy16
.Ltail_cpy16:
	sub		r2, #1
	bcc		.Lend_cpy16		@ r2 was 0, bug out
	lsl		r2, r2, #1
.Lres_cpy16:
		ldrh	r3, [r1, r2]
		strh	r3, [r0, r2]
		sub		r2, r2, #2
		bcs		.Lres_cpy16
.Lend_cpy16:
	pop		{r4}
	pop		{r3}
	bx	r3
	.align 2
.Lpool_cpy16:
	.word memcpy32

