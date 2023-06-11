#include "platform.h"

	# size of each hart's stack is 1024 bytes
	.equ	STACK_SIZE, 1024

	.global	entry

.section  .text.entry
entry:
	# park harts with id != 0
	# csrr	t0, mhartid		# read current hart id
	# mv	tp, t0			# keep CPU's hartid in its tp for later usage.
	# bnez	t0, park		# if we're not on the hart 0
					# we park the hart

	mv tp, a0 #从SBI接入, hartid
	mv s1, a1 #从SBI接入, 设备树地址, 为什么不存s0因为参考uboot源代码
			  #(s0同时还是fp)
	
	# Set all bytes in the BSS section to zero.
	la	a0, _bss_start
	la	a1, _bss_end
	bgeu	a0, a1, 2f
1:
	sw	zero, (a0)
	addi	a0, a0, 4
	bltu	a0, a1, 1b
2:
	# Setup stacks, the stack grows from bottom to top, so we put the
	# stack pointer to the very end of the stack range.
	
	# 注意， 我们改用tp了， 而不是t0， 因为t0不再是hartid
	slli	t0, tp, 10		# shift left the hart id by 1024
	la	sp, stacks + STACK_SIZE	# set the initial stack pointer
					# to the end of the first stack space
	add	sp, sp, t0		# move the current hart stack pointer
					# to its place in the stack space

#ifdef CONFIG_SYSCALL
	# https://lore.kernel.org/qemu-devel/20201223192553.332508-1-atish.patra@wdc.com/
	# For qemu version >= 6.0, exception would be raised if no PMP enty is
	# configured. So just configure one entny, which allows all the whole
	# 32-bits physical address range is R/W/X.
	# FIXME: I say it is a temporary workaroud due to I think the patch
	# above contains bug, and I have raised new issue to qemu but it has not
	# been rootcaused till now. Details please refer to
	# https://gitlab.com/qemu-project/qemu/-/issues/585 or
	# https://gitee.com/unicornx/riscv-operating-system-mooc/issues/I441IC (in chinese)
	# So it's just a temporary workaround till now to not block people who
	# want to try newer qemu (>= 6.0).
	# not t0, x0
	#li      t0, 0xffffffff
	#csrw    pmpaddr0, t0
	#li      t0, 0xf
	#csrw    pmpcfg0, t0
#endif

	# At the end of start_kernel, schedule() will call MRET to switch
	# to the first task, so we parepare the mstatus here.
	# Notice: default mstatus is 0
#ifdef CONFIG_SYSCALL
	# Keep mstatus.MPP as 0, so we will run in User mode after MRET.
	# Set mstatus.MPIE to 1, so MRET will enable the interrupt.

	# SPIE
	mv	t0, zero
#else
	# Set mstatus.MPP to 3, so we still run in Machine mode after MRET.
	# Set mstatus.MPIE to 1, so MRET will enable the interrupt.

	# SPP & SPIE
	li	t0, 1 << 8 | 1 << 5
#endif
	csrr	a1, sstatus
	or	t0, t0, a1
	csrw	sstatus, t0
	
	la t0, temp_vector
	csrw stvec, t0
	
	mv a0, tp
	mv a1, s1

	j	start_kernel		# hart 0 jump to c

park:
	wfi
	j	park
	
.align 2
temp_vector:
	csrr t0, scause
	li t1, 63
	srl t1, t0, t1
	beqz t1, 1f
	
	li a0, 0x69
	li   a7, 0x01
	ecall
	
1:	addi a0, t0, 0x30
	li   a7, 0x01
	ecall
	li a0, 0x0A
	li   a7, 0x01
	ecall
	
	csrr t0, stval
	li t1, 0x0F
	li t2, 16
	
2:	beqz t2, 3f
	and t3, t0, t1
	addi a0, t3, 0x41
	li   a7, 0x01
	ecall
	srli t0, t0, 4
	addi t2, t2, -1
	j 2b
3:	li a0, 0x0A
	li   a7, 0x01
	ecall

	mv t0, sp
	li t1, 0x0F
	li t2, 16

2:	beqz t2, 3f
	and t3, t0, t1
	addi a0, t3, 0x41
	li   a7, 0x01
	ecall
	srli t0, t0, 4
	addi t2, t2, -1
	j 2b
3:	li a0, 0x0A
	li   a7, 0x01
	ecall

	la t0, entry
	li t1, 0x0F
	li t2, 16

2:	beqz t2, 3f
	and t3, t0, t1
	addi a0, t3, 0x41
	li   a7, 0x01
	ecall
	srli t0, t0, 4
	addi t2, t2, -1
	j 2b
3:	li a0, 0x0A
	li   a7, 0x01
	ecall
	
	csrr t0, sepc
	li t1, 0x0F
	li t2, 16

2:	beqz t2, 3f
	and t3, t0, t1
	addi a0, t3, 0x41
	li   a7, 0x01
	ecall
	srli t0, t0, 4
	addi t2, t2, -1
	j 2b
3:	li a0, 0x0A
	li   a7, 0x01
	ecall
	
	#mv a0, t0
	li   a7, 0x08
	ecall
	

	

.align 3
stacks:
	.skip	STACK_SIZE * MAXNUM_CPU # allocate space for all the harts stacks

	.end				# End of file
