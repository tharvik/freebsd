#include "thr_private.h"

/* TLS unsafe stack pointer, used by LLVM's safestack generated code. */
__thread void *__safestack_unsafe_stack_ptr   = NULL;
__thread void *__safestack_unsafe_stack_start = NULL;

void
__safestack_init(void)
{
        /* Nothing to do here, pure compat. */
}

/*
 * The three following function are provided as stack introspection helpers,
 * used by Google chrome's garbage collector for example.
 */
void *
__safestack_get_unsafe_stack_start()
{
        return __safestack_unsafe_stack_start;
}

void *
__safestack_get_unsafe_stack_ptr()
{
	return __safestack_unsafe_stack_ptr;
}

__attribute__((noinline)) /* required for __builtin_frame_address(0) to work */
void *
__safestack_get_safe_stack_ptr()
{
	return ((char *)__builtin_frame_address(0)) + 2 * sizeof (void *);
}
