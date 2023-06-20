#include "os.h"

int spin_lock()
{
	w_sstatus(r_sstatus() & ~SSTATUS_SIE);
	return 0;
}

int spin_unlock()
{
	w_sstatus(r_sstatus() | SSTATUS_SIE);
	return 0;
}
