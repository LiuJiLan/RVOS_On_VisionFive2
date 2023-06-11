#include "os.h"

/*
 * Following functions SHOULD be called ONLY ONE time here,
 * so just declared here ONCE and NOT included in file os.h.
 */
extern void uart_init(void);
extern void page_init(void);
extern void sched_init(void);
extern void schedule(void);
extern void os_main(void);
extern void trap_init(void);
extern void plic_init(void);
extern void timer_init(void);

void sbi_putchar(char ch) {

	register reg_t a7 asm("a7") = (reg_t)0x01;
	register reg_t a0 asm("a0") = (reg_t)ch;
	asm volatile("ecall"
				:
				:"r"(a0), "r"(a7)
				: "memory");
}

void sbi_puts(char *s)
{
	while (*s) {
		sbi_putchar(*s++);
	}
}

void start_kernel(reg_t hartid, reg_t dtb_addr)
{	
	sbi_putchar('?');
	sbi_puts("Hello by SBI!\n");
	uart_init();
	
	uart_puts("\n");
	uart_puts("Hello, RVOS!\n");
	uart_puts("\n");
	printf("Hart ID: %d\n", hartid);
	printf("DTB is at %x\n", dtb_addr);

	page_init();
	uart_puts("\n");

	trap_init();

	plic_init();
	uart_puts("\n");

	timer_init();

	sched_init();
	
	os_main();

	schedule();

	uart_puts("Would not go here!\n");
	while (1) {}; // stop here!
}

