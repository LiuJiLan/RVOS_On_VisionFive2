#ifndef __TYPES_H__
#define __TYPES_H__

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int  uint32_t;
typedef unsigned long long uint64_t;

/*
 * RISCV32: register is 32bits width
 */ 
typedef uint64_t reg_t;

#define PGSIZE 4096
#define PGSHIFT 12

#endif /* __TYPES_H__ */
