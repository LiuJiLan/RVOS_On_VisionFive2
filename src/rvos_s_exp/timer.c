#include "os.h"

extern void schedule(void);

/* interval ~= 1s */
#define HZ 1
#define TIMER_INTERVAL CLINT_TIMEBASE_FREQ / HZ

//static uint64_t stime = 0;
static uint32_t _tick = 0;

#define MAX_TIMER 10
static struct timer timer_list[MAX_TIMER];

/* load timer interval(in ticks) for next timer interrupt.*/
void timer_load(uint64_t interval)
{
	// /* each CPU has a separate source of timer interrupts. */
	// int id = r_mhartid();
	
	// 利用SBI的模拟读CSR来读取
	// SBI会通过异常处理运行M态之外访问MTIME
	// 在我的这版openSBI中, 对MTIME的读取不会检查访问它的时候所在的priv
	uint64_t stime_value = r_mtime() + interval;
	//uint64_t stime_value = *(uint64_t*)CLINT_MTIME + interval;
	
	//stime+=interval;
	
	register reg_t a7 asm("a7") = (reg_t)0x00UL;
	register reg_t a0 asm("a0") = (reg_t)stime_value;
	asm volatile("ecall"
				:"+r"(a0)
				:"r"(a7)
				:"memory");
	//reg_t ret = a0;
	//printf("TIMER ECALL RET: 0x%x", ret);
}

void timer_init()
{
	struct timer *t = &(timer_list[0]);
	for (int i = 0; i < MAX_TIMER; i++) {
		t->func = NULL; /* use .func to flag if the item is used */
		t->arg = NULL;
		t++;
	}

	/*
	 * On reset, mtime is cleared to zero, but the mtimecmp registers 
	 * are not reset. So we have to init the mtimecmp manually.
	 */
	timer_load(TIMER_INTERVAL);

	/* enable machine-mode timer interrupts. */
	w_sie(r_sie() | SIE_STIE);
}

struct timer *timer_create(void (*handler)(void *arg), void *arg, uint32_t timeout)
{
	/* TBD: params should be checked more, but now we just simplify this */
	if (NULL == handler || 0 == timeout) {
		return NULL;
	}

	/* use lock to protect the shared timer_list between multiple tasks */
	spin_lock();

	struct timer *t = &(timer_list[0]);
	for (int i = 0; i < MAX_TIMER; i++) {
		if (NULL == t->func) {
			break;
		}
		t++;
	}
	if (NULL != t->func) {
		spin_unlock();
		return NULL;
	}

	t->func = handler;
	t->arg = arg;
	t->timeout_tick = _tick + timeout;

	spin_unlock();

	return t;
}

void timer_delete(struct timer *timer)
{
	spin_lock();

	struct timer *t = &(timer_list[0]);
	for (int i = 0; i < MAX_TIMER; i++) {
		if (t == timer) {
			t->func = NULL;
			t->arg = NULL;
			break;
		}
		t++;
	}

	spin_unlock();
}

/* this routine should be called in interrupt context (interrupt is disabled) */
static inline void timer_check()
{
	struct timer *t = &(timer_list[0]);
	for (int i = 0; i < MAX_TIMER; i++) {
		if (NULL != t->func) {
			if (_tick >= t->timeout_tick) {
				t->func(t->arg);

				/* once time, just delete it after timeout */
				t->func = NULL;
				t->arg = NULL;

				break;
			}
		}
		t++;
	}
}

void timer_handler() 
{
	_tick++;
	//printf("tick: %d\n", _tick);

	timer_check();

	timer_load(TIMER_INTERVAL);

	schedule();
}
