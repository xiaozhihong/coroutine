// ==============================================================================
.global AsmSwapRegister
.type AsmSwapRegister, @function

AsmSwapRegister:
	movq  %rbp   , 8(%rdi)
	movq  %rsp   , 16(%rdi)
	movq  %rbx   , 24(%rdi)
	movq  (%rsp) , %rax
    movq  %rax   , 32(%rdi)
	movq  %r12   , 40(%rdi)
	movq  %r13   , 48(%rdi)
	movq  %r14   , 56(%rdi)
	movq  %r15   , 64(%rdi)
    movq  %rax,    72(%rdi)
	movq  8(%rsi)  , %rbp
	movq  16(%rsi) , %rsp
    leaq 8(%rsp), %rsp
	movq  24(%rsi) , %rbx
	movq  40(%rsi) , %r12
	movq  48(%rsi) , %r13
	movq  56(%rsi) , %r14
	movq  64(%rsi) , %r15
    movq  72(%rsi), %rax
	movq  %rsi   , %rdi
	jmpq *32(%rsi)

// ==============================================================================
.global AsmLoadRegister
.type AsmLoadRegister, @function

AsmLoadRegister:
	movq  8(%rdi)  , %rbp
	movq  16(%rdi) , %rsp
    leaq 8(%rsp), %rsp
	movq  24(%rdi) , %rbx
	movq  40(%rdi) , %r12
	movq  48(%rdi) , %r13
	movq  56(%rdi) , %r14
	movq  64(%rdi) , %r15
    movq  72(%rdi), %rax
	movq  %rdi   , %rdi
	jmpq *32(%rdi)

// ==============================================================================
.global AsmInitRegister
.type AsmInitRegister, @function

AsmInitRegister:
	ret
