#include "os.h"

extern void trap_vector(void);
extern void uart_isr(void);
extern void timer_handler(void);
extern void schedule(void);
extern void do_syscall(struct context *cxt);

void trap_init()
{
	/*
	 * set the trap-vector base-address for machine-mode
	 */
	w_stvec((reg_t)trap_vector);
}

void external_interrupt_handler()
{
	int irq = plic_claim();

	if (irq == UART0_IRQ){
      		uart_isr();
	} else if (irq) {
		printf("unexpected interrupt irq = %d\n", irq);
	}
	
	if (irq) {
		plic_complete(irq);
	}
}

reg_t trap_handler(reg_t epc, reg_t cause, struct context *cxt)
{
	reg_t return_pc = epc;
	reg_t cause_code = cause & 0xfff;
	
	reg_t mask = 0x1;
	mask = mask << 63;
	
	if (cause & mask) {
		/* Asynchronous trap - interrupt */
		switch (cause_code) {
		case 1:
			uart_puts("software interruption!\n");
			/*
			 * acknowledge the software interrupt by clearing
    			 * the MSIP bit in mip.
			 */
			// int id = r_mhartid();
    		// 	*(uint32_t*)CLINT_MSIP(id) = 0;
			
			//	S态不能用这种方式触发SIP
			//	我们直接用写SIP的方式触发
			w_sip(r_sip() & ~SIP_SSIP);

			schedule();

			break;
		case 5:
			uart_puts("timer interruption!\n");
			timer_handler();
			break;
		case 9:
			uart_puts("external interruption!\n");
			external_interrupt_handler();
			break;
		default:
			uart_puts("unknown async exception!\n");
			break;
		}
	} else {
		/* Synchronous trap - exception */
		printf("Sync exceptions!, code = %d\n", cause_code);
		switch (cause_code) {
		case 8:
			uart_puts("System call from U-mode!\n");
			do_syscall(cxt);
			return_pc += 4;
			break;
		default:
			panic("OOPS! What can I do!");
			//return_pc += 4;
		}
	}

	return return_pc;
}

void trap_test()
{
	/*
	 * Synchronous exception code = 7
	 * Store/AMO access fault
	 */
	*(int *)0x00000000 = 100;

	/*
	 * Synchronous exception code = 5
	 * Load access fault
	 */
	//int a = *(int *)0x00000000;

	uart_puts("Yeah! I'm return back from trap!\n");
}

