#ifndef __USER_API_H__
#define __USER_API_H__

#include "types.h"

/* user mode syscall APIs */
extern int gethid(unsigned int *hid);
int generalized_syscall(uint64_t func, uint64_t in_nr, uint64_t * in_stack, uint64_t * out_stack);

#endif /* __USER_API_H__ */
