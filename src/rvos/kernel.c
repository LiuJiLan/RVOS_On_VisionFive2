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

void start_kernel(void)
{	
	// 由于和PLL相关，所以就用BOOTROM中初始化的UART
	uart_init();
	uart_puts("\n");
	uart_puts("Hello, RVOS!\n");
	uart_puts("\n");
	

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

