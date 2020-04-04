
#ifdef _ASAMBLE_

#define oRAX 	0
#define oRBX 	8
#define oRCX 	16
#define oRDX	24
#define oRSP 	32
#define oRBP 	40
#define oRSI 	48
#define oRDI 	56
#define oR8 	64
#define oR9	72
#define oR10 	80
#define oR11 	88
#define oR12 	96
#define oR13 	104
#define oR14 	112
#define oR15 	120
#define oRIP 	128

#else

#include <sys/types.h>

#ifndef _CONTEXT_H_
#define _CONTEXT_H_

struct stack {
	void *ss_sp;
	size_t ss_size;
};

typedef struct lcontext {
	unsigned long rax;
	unsigned long rbx;
	unsigned long rcx;
	unsigned long rdx;
	unsigned long rsp;
	unsigned long rbp;
	unsigned long rsi;
	unsigned long rdi;
	unsigned long r8;
	unsigned long r9;
	unsigned long r10;
	unsigned long r11;
	unsigned long r12;
	unsigned long r13;
	unsigned long r14;
	unsigned long r15;
	unsigned long rip;

	struct stack uc_stack;
} lcontext_t;

extern int lwt_getcontext(lcontext_t *ucp);
extern int lwt_setcontext(const lcontext_t *ucp);
extern void lwt_makecontext(lcontext_t *ucp, void (*func)(), int argc, ...);
extern int lwt_swapcontext(lcontext_t *oucp, const lcontext_t *ucp);

#endif /* _CONTEXT_H_ */

#endif /* _ASAMBLE_ */
