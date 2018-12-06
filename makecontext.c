#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

#include "context.h"

void dummy(void)
{
	abort();
}

void
makecontext (ucontext_t *ucp, void (*func) (void), int argc, ...)
{
  uintptr_t *sp;
  unsigned int rbp;
  va_list ap;
  int i;

  /* Generate room on stack for parameter if needed and uc_link.  */
  sp = (uintptr_t *)((uintptr_t) ucp->uc_stack.ss_sp
		   + ucp->uc_stack.ss_size);
  sp -= (argc > 6 ? argc - 6 : 0) + 1;
  /* Align stack and make space for trampoline address.  */
  sp = (uintptr_t *) ((((uintptr_t) sp) & -16L) - 8);

  rbp = (argc > 6 ? argc - 6 : 0) + 1;

  /* Setup context ucp.  */
  /* Address to jump to.  */
  ucp->rip = (unsigned long) func;
  /* Setup rbx.*/
  ucp->rbp = (unsigned long) &(sp[rbp]);
  ucp->rsp = (unsigned long) sp;

  /* Setup stack.  */
    sp[0] = (uintptr_t) &dummy;

  va_start (ap, argc);
  /* Handle arguments.

     The standard says the parameters must all be int values.  This is
     an historic accident and would be done differently today.  For
     x86-64 all integer values are passed as 64-bit values and
     therefore extending the API to copy 64-bit values instead of
     32-bit ints makes sense.  It does not break existing
     functionality and it does not violate the standard which says
     that passing non-int values means undefined behavior.  */
  for (i = 0; i < argc; ++i)
    switch (i)
      {
      case 0:
	ucp->rdi = va_arg (ap, unsigned long);
	break;
      case 1:
	ucp->rsi = va_arg (ap, unsigned long);
	break;
      case 2:
	ucp->rdx = va_arg (ap, unsigned long);
	break;
      case 3:
	ucp->rcx = va_arg (ap, unsigned long);
	break;
      case 4:
	ucp->r8 = va_arg (ap, unsigned long);
	break;
      case 5:
	ucp->r9 = va_arg (ap, unsigned long);
	break;
      default:
	/* Put value on stack.  */
	sp[i - 5] = va_arg (ap, unsigned long);
	break;
      }
  va_end (ap);

}
