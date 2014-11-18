#ifndef _SAFESTACK_H
# define _SAFESTACK_H

__BEGIN_DECLS

void __safestack_init(void);
void *__safestack_get_unsafe_stack_start(void);
void *__safestack_get_unsafe_stack_ptr(void);
void *__safestack_get_safe_stack_ptr(void);
extern __thread void *__safestack_unsafe_stack_ptr;
extern __thread void *__safestack_unsafe_stack_start;

__END_DECLS

#endif /* !_SAFESTACK_H */
