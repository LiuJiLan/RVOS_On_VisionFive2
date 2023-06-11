#include "os.h"

void plic_init(void)
{
	int hart = r_tp();

	/* 
	 * Set priority for UART0.
	 *
	 * Each PLIC interrupt source can be assigned a priority by writing 
	 * to its 32-bit memory-mapped priority register.
	 * The QEMU-virt (the same as FU540-C000) supports 7 levels of priority. 
	 * A priority value of 0 is reserved to mean "never interrupt" and 
	 * effectively disables the interrupt. 
	 * Priority 1 is the lowest active priority, and priority 7 is the highest. 
	 * Ties between global interrupts of the same priority are broken by 
	 * the Interrupt ID; interrupts with the lowest ID have the highest 
	 * effective priority.
	 */
	*(uint32_t*)PLIC_PRIORITY(UART0_IRQ) = 7;
	
	//清空所有的中断enable
	
	//第一个中断enable寄存器, 有几个中断enable寄存器得看手册
	//但我觉得将此hart所有的enable空间(包括Reserved空间)一并清零也没事
	//因为大多数这种Reserved空间都是Write Ignore Read Ignore的
	uint32_t * p = (uint32_t*)PLIC_SENABLE(hart, 0);
	
	printf("The following interrupts are already enabled before we set them:\n");
	
	for (int i = 0; i < 5; i++) {
		uint32_t enable_reg = *(p+i);
		printf("PLIC Enable Register %d: ", i);
		if (enable_reg){
			for (int j = 0; j < 32; j++){
				uint32_t mask = 0x01;
				if (enable_reg & (mask << j)) {
					printf("%d; ", i * 32 + j);
				}
			}
			printf("\n");
		} else {
			printf("None!\n");
		}
	}
	
	printf("We will disable them all. And then, only enable UART0.\n");
	for (int i = 0; i < 5; i++) {
		*(p+i) = 0x0;
	}
 
	/*
	 * Enable UART0
	 *
	 * Each global interrupt can be enabled by setting the corresponding 
	 * bit in the enables registers.
	 */
	 
	*(uint32_t*)PLIC_SENABLE(hart, UART0_IRQ)= 1;

	/* 
	 * Set priority threshold for UART0.
	 *
	 * PLIC will mask all interrupts of a priority less than or equal to threshold.
	 * Maximum threshold is 7.
	 * For example, a threshold value of zero permits all interrupts with
	 * non-zero priority, whereas a value of 7 masks all interrupts.
	 * Notice, the threshold is global for PLIC, not for each interrupt source.
	 */
	*(uint32_t*)PLIC_STHRESHOLD(hart) = 0;
	

	/* enable machine-mode external interrupts. */
	w_sie(r_sie() | SIE_SEIE);
}

/* 
 * DESCRIPTION:
 *	Query the PLIC what interrupt we should serve.
 *	Perform an interrupt claim by reading the claim register, which
 *	returns the ID of the highest-priority pending interrupt or zero if there 
 *	is no pending interrupt. 
 *	A successful claim also atomically clears the corresponding pending bit
 *	on the interrupt source.
 * RETURN VALUE:
 *	the ID of the highest-priority pending interrupt or zero if there 
 *	is no pending interrupt.
 */
int plic_claim(void)
{
	int hart = r_tp();
	printf("Check plic hart:%d\n", hart);
	int irq = *(uint32_t*)PLIC_SCLAIM(hart);
	return irq;
}

/* 
 * DESCRIPTION:
  *	Writing the interrupt ID it received from the claim (irq) to the 
 *	complete register would signal the PLIC we've served this IRQ. 
 *	The PLIC does not check whether the completion ID is the same as the 
 *	last claim ID for that target. If the completion ID does not match an 
 *	interrupt source that is currently enabled for the target, the completion
 *	is silently ignored.
 * RETURN VALUE: none
 */
void plic_complete(int irq)
{
	int hart = r_tp();
	*(uint32_t*)PLIC_SCOMPLETE(hart) = irq;
}
