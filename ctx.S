// ==============================================================================
.globl AsmStoreRegister
.type  AsmStoreRegister, @function

AsmStoreRegister:
	movq %rdi      , %rbx
	movq %rbp      , 8(%rbx)
	movq %rsp      , 16(%rbx)
	movq %rax      , 24(%rbx)
	movq %rcx      , 32(%rbx)
	movq %rdx      , 40(%rbx)
	movq %rsi      , 48(%rbx)
	movq (%rsp)   , %rax
	//movq $0x666666, %rcx
    movq %rax, 56(%rbx)
	movq $0x12ABCD , 64(%rbx)
	ret

// ==============================================================================

.global AsmLoadRegister
.type AsmLoadRegister, @function

AsmLoadRegister:
	movq  %rdi    , %rbx
	movq  8(%rbx)   , %rbp
	movq  16(%rbx)  , %rsp
	movq  24(%rbx)  , %rax
	movq  32(%rbx)  , %rcx
	movq  40(%rbx)  , %rdx
	movq  48(%rbx)  , %rsi
	pushq 56(%rbx)
	ret

// ==============================================================================
.global AsmSwapRegister
.type AsmSwapRegister, @function

AsmSwapRegister:
	movq  %rdi     , %rbx
	movq  %rbp     , 8(%rbx)
	movq  %rsp     , 16(%rbx)
	movq  %rax     , 24(%rbx)
	movq  %rcx     , 32(%rbx)
	movq  %rdx     , 40(%rbx)
	movq  %rsi     , 48(%rbx)
	movq  (%rsp)   , %rax
    movq  %rax     , 56(%rbx)
	movq  %r12     , 64(%rbx)
	movq  %r13     , 72(%rbx)
	movq  %r14     , 80(%rbx)
	movq  %r15     , 88(%rbx)
	movq  %rsi     , %rbx
	movq  8(%rbx)  , %rbp
	movq  16(%rbx) , %rsp
	movq  24(%rbx) , %rax
	movq  32(%rbx) , %rcx
	movq  40(%rbx) , %rdx
	movq  48(%rbx) , %rsi
	movq  64(%rbx) , %r12
	movq  72(%rbx) , %r13
	movq  80(%rbx) , %r14
	movq  88(%rbx) , %r15
	movq  (%rbx)   , %rdi
	pushq 56(%rbx)
	ret

// ==============================================================================
.global AsmSwapRegister
