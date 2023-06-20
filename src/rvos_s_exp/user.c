#include "os.h"

#include "user_api.h"

#define DELAY 4000

void user_task0(void)
{
	uart_puts("Task 0: Created!\n");

	unsigned int hid = -1;

	/*
	 * if syscall is supported, this will trigger exception, 
	 * code = 2 (Illegal instruction)
	 */
	//hid = r_mhartid();
	//printf("hart id is %d\n", hid);

#ifdef CONFIG_SYSCALL
	int ret = -1;
	ret = gethid(&hid);
	//ret = gethid(NULL);
	if (!ret) {
		printf("system call returned!, hart id is %d\n", hid);
	} else {
		printf("gethid() failed, return: %d\n", ret);
	}
#endif
	uart_puts("Task 0: Running... \n");
	task_delay(DELAY);
	while (1)
	{
		generalized_syscall(0x0, 0x0, 0x0, 0x0);
	}
	
	// while (1){
	// 	uart_puts("Task 0: Running... \n");
	// 	task_delay(DELAY);
	// }
}

void user_task1(void)
{
	uart_puts("Task 1: Created!\n");
	while (1) {
		uart_puts("Task 1: Running... \n");
		task_delay(DELAY);
	}
}

/* NOTICE: DON'T LOOP INFINITELY IN main() */
void os_main(void)
{
	task_create(user_task0);
	task_create(user_task1);
}

int generalized_syscall(uint64_t func, uint64_t in_nr, uint64_t * in_stack, uint64_t * out_stack) {
	// ret ONLY transfers ERRNO
	register reg_t a7 asm("a7") = (reg_t)0x2;
	register reg_t a0 asm("a0") = (reg_t)func;
	register reg_t a1 asm("a1") = (reg_t)in_nr;
	register reg_t a2 asm("a2") = (reg_t)in_stack;
	register reg_t a3 asm("a3") = (reg_t)out_stack;
	asm volatile("ecall"
				:"+r"(a0)
				:"r"(a7), "r"(a1), "r"(a2),  "r"(a3)
				:"memory");
	return (int)a0;
}

