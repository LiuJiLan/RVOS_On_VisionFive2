#include "os.h"

extern void schedule(void);

/* interval ~= 1s */
#define TIMER_INTERVAL CLINT_TIMEBASE_FREQ

static uint64_t stime = 0;
static uint32_t _tick = 0;

#define MAX_TIMER 10
static struct timer timer_list[MAX_TIMER];

/* load timer interval(in ticks) for next timer interrupt.*/
void timer_load(uint64_t interval)
{
	// /* each CPU has a separate source of timer interrupts. */
	// int id = r_mhartid();
	
	// *(uint64_t*)CLINT_MTIMECMP(id) = *(uint64_t*)CLINT_MTIME + interval;
	
	stime+=interval;
	// 我看的这一版openSBI实现中legacy的timer和Timer Extension中的底层实现本质上是同一个函数
	// 另外, 本质上设置的是xtimecmp, 对于没有实现stimecmp的芯片而言, 没法读取当前的xtime
	// 现在的代码会导致短时间内触发多个时间中断, 直到设置的时间点>xtimecmp
	// (注意, SBI手册中写的after interval中的一切时间都是absolute, 是时间点而不是时间段)
	register reg_t a7 asm("a7") = (reg_t)0x00UL;
	//register reg_t a7 asm("a7") = (reg_t)0x54494D45UL;
	//register reg_t a6 asm("a6") = (reg_t)0x00UL;
	register reg_t a0 asm("a0") = (reg_t)stime;
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
	printf("tick: %d\n", _tick);

	timer_check();

	timer_load(TIMER_INTERVAL);

	schedule();
}
