// ==============================================================================
.globl AsmStoreRegister
.type  AsmStoreRegister, @function

AsmStoreRegister:
	movq (%rdi), %rbx
	movq %rbp, (%rbx)
	movq %rsp, 8(%rbx)
	movq %rax, 16(%rbx)
	movq %rcx, 24(%rbx)
	movq %rdx, 32(%rbx)
	movq %rsi, 40(%rbx)
	movq $0x12ABCD, 48(%rbx)
	ret

// ==============================================================================

.global AsmLoadRegister
.type AsmLoadRegister, @function

AsmLoadRegister:
	ret

// ==============================================================================
