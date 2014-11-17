#include "thr_private.h"

/* TLS unsafe stack pointer, used by LLVM's safestack generated code. */
__thread void *__safestack_unsafe_stack_ptr = NULL;

void
__safestack_init(void)
{
        /* nothing to do here, pure compat */
}

/* The three following function are provided has helpers, like for
 * Google chrome's garbage collector.
 */
void *__safestack_get_unsafe_stack_start()
{
        struct pthread *curthread;

        curthread = _get_curthread();
        return ((char *)curthread->attr.unsafe_stackaddr_attr) +
                curthread->attr.stacksize_attr;
}

void *__safestack_get_unsafe_stack_ptr()
{
        return __safestack_unsafe_stack_ptr;
}

__attribute__((noinline)) /* required for __builtin_frame_address(0) to work */
void *__safestack_get_safe_stack_ptr()
{
        return ((char *)__builtin_frame_address(0)) + 2 * sizeof (void *);
}
