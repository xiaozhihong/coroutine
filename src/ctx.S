// ==============================================================================
.global AsmSwapRegister
.type AsmSwapRegister, @function

AsmSwapRegister:
	movq  (%rdi), %rdx
	movq  %rbp   , 8(%rdx)
	movq  %rsp   , 16(%rdx)
	movq  %rbx   , 24(%rdx)
	movq  (%rsp) , %rax
    movq  %rax   , 32(%rdx)
	movq  %r12   , 40(%rdx)
	movq  %r13   , 48(%rdx)
	movq  %r14   , 56(%rdx)
	movq  %r15   , 64(%rdx)
	movq  (%rsi), %rdx
	movq  8(%rdx)  , %rbp
	movq  16(%rdx) , %rsp
    leaq 8(%rsp), %rsp
	movq  24(%rdx) , %rbx
	movq  40(%rdx) , %r12
	movq  48(%rdx) , %r13
	movq  56(%rdx) , %r14
	movq  64(%rdx) , %r15
	movq  %rsi   , %rdi
	jmpq *32(%rdx)

// ==============================================================================
.global AsmLoadRegister
.type AsmLoadRegister, @function

AsmLoadRegister:
	movq  (%rdi), %rdx
	movq  8(%rdx)  , %rbp
	movq  16(%rdx) , %rsp
    leaq 8(%rsp), %rsp
	movq  24(%rdx) , %rbx
	movq  40(%rdx) , %r12
	movq  48(%rdx) , %r13
	movq  56(%rdx) , %r14
	movq  64(%rdx) , %r15
	movq  %rdi   , %rdi
	jmpq *32(%rdx)
