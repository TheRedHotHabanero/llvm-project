	.text
	.file	"simple_module"
	.globl	empty_function                  ; -- Begin function empty_function
	.type	empty_function,@function
empty_function:                         ; @empty_function
; %bb.0:
	BR r0
.Lfunc_end0:
	.size	empty_function, .Lfunc_end0-empty_function
                                        ; -- End function
	.section	".note.GNU-stack","",@progbits
