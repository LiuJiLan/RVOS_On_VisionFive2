#include "os.h"
#include "syscall.h"

int sys_gethid(unsigned int *ptr_hid)
{
	printf("--> sys_gethid, arg0 = 0x%x\n", ptr_hid);
	if (ptr_hid == NULL) {
		return -1;
	} else {
		//*ptr_hid = r_mhartid();
		*ptr_hid = r_tp();
		return 0;
	}
}

void do_syscall(struct context *cxt)
{
	uint32_t syscall_num = cxt->a7;

	reg_t	func;
	reg_t	in_nr;
	reg_t * in_stack;
	reg_t * out_stack;

	switch (syscall_num) {
	case SYS_gethid:
		cxt->a0 = sys_gethid((unsigned int *)(cxt->a0));
		break;
	case 0x2:
		func		= cxt->a0;
		in_nr		= cxt->a1;
		in_stack	= cxt->a2;
		out_stack	= cxt->a3;

		switch (func)
		{
		case 0UL:
			task_yield();
			cxt->a0 = 0;
			break;
		
		default:
			cxt->a0 = -1;
			break;
		}

		break;

	default:
		printf("Unknown syscall no: %d\n", syscall_num);
		cxt->a0 = -1;
	}

	return;
}
