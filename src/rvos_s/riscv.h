#ifndef __RISCV_H__
#define __RISCV_H__

#include "types.h"

/*
 * ref: https://github.com/mit-pdos/xv6-riscv/blob/riscv/kernel/riscv.h
 */

static inline reg_t r_tp()
{
	reg_t x;
	asm volatile("mv %0, tp" : "=r" (x) );
	return x;
}

/* which hart (core) is this? */
static inline reg_t r_mhartid()
{
	reg_t x;
	asm volatile("csrr %0, mhartid" : "=r" (x) );
	return x;
}

/* Machine Status Register, mstatus */
#define MSTATUS_MPP (3 << 11)
#define MSTATUS_SPP (1 << 8)

#define MSTATUS_MPIE (1 << 7)
#define MSTATUS_SPIE (1 << 5)
#define MSTATUS_UPIE (1 << 4)

#define MSTATUS_MIE (1 << 3)
#define MSTATUS_SIE (1 << 1)
#define MSTATUS_UIE (1 << 0)

#define SSTATUS_SIE (1 << 1)

static inline reg_t r_sstatus()
{
	reg_t x;
	asm volatile("csrr %0, sstatus" : "=r" (x) );
	return x;
}

static inline void w_sstatus(reg_t x)
{
	asm volatile("csrw sstatus, %0" : : "r" (x));
}

/*
 * machine exception program counter, holds the
 * instruction address to which a return from
 * exception will go.
 */
static inline void w_mepc(reg_t x)
{
	asm volatile("csrw mepc, %0" : : "r" (x));
}

static inline reg_t r_mepc()
{
	reg_t x;
	asm volatile("csrr %0, mepc" : "=r" (x));
	return x;
}

static inline void w_sepc(reg_t x)
{
	asm volatile("csrw sepc, %0" : : "r" (x));
}

static inline reg_t r_sepc()
{
	reg_t x;
	asm volatile("csrr %0, sepc" : "=r" (x));
	return x;
}

/* Machine Scratch register, for early trap handler */
static inline void w_sscratch(reg_t x)
{
	asm volatile("csrw sscratch, %0" : : "r" (x));
}

/* Machine-mode interrupt vector */
static inline void w_stvec(reg_t x)
{
	asm volatile("csrw stvec, %0" : : "r" (x));
}

/* Machine-mode Interrupt Enable */
#define MIE_MEIE (1 << 11) // external
#define MIE_MTIE (1 << 7)  // timer
#define MIE_MSIE (1 << 3)  // software

#define SIE_SEIE (1 << 9) // external
#define SIE_STIE (1 << 5)  // timer
#define SIE_SSIE (1 << 1)  // software

#define SIP_SEIP (1 << 9) // external
#define SIP_STIP (1 << 5)  // timer
#define SIP_SSIP (1 << 1)  // software

static inline reg_t r_sie()
{
	reg_t x;
	asm volatile("csrr %0, sie" : "=r" (x) );
	return x;
}

static inline void w_sie(reg_t x)
{
	asm volatile("csrw sie, %0" : : "r" (x));
}

static inline reg_t r_sip()
{
	reg_t x;
	asm volatile("csrr %0, sip" : "=r" (x) );
	return x;
}

static inline void w_sip(reg_t x)
{
	asm volatile("csrw sip, %0" : : "r" (x));
}

static inline reg_t r_scause()
{
	reg_t x;
	asm volatile("csrr %0, scause" : "=r" (x) );
	return x;
}

static inline reg_t r_stval()
{
	reg_t x;
	asm volatile("csrr %0, stval" : "=r" (x) );
	return x;
}

// 此MTIME非CLINT的那个MMIO的MTIME, 是Unprivileged and User-Level CSR
// 供SBI使用
// 在https://github.com/riscv-software-src/opensbi/blob/v1.2/include/sbi/riscv_encoding.h#L247
// SBI版本更新可能会不在这一行
static inline reg_t r_mtime()
{
	reg_t x;
	asm volatile("csrr %0, 0x0C01" : "=r" (x) );
	return x;
}

#endif /* __RISCV_H__ */
