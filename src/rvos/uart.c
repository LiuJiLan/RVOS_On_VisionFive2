#include "os.h"

#define UART0_BASE_ADDR 0x10000000UL

#define RBR 0x00
#define THR 0x00
#define IER 0x01
#define IIR 0x02
#define LCR 0x03
#define MCR 0x04
#define LSR 0x05
#define MSR 0x06
#define SCR 0x07
#define DLL 0x00
#define DLM 0x01

#define LSR_RX_READY 0x01
#define LSR_TX_IDLE 0x20
#define REG_WIDTH 4
#define REG_SHIFT 2

static inline uint8_t uart_read_reg(uint8_t reg)
{
    volatile uint32_t *addr = (uint32_t *)(UART0_BASE_ADDR + (reg << REG_SHIFT));
    return *addr & 0xff;
}

static inline void uart_write_reg(uint8_t reg, uint8_t value)
{
    volatile uint32_t *addr = (uint32_t *)(UART0_BASE_ADDR + (reg << REG_SHIFT));
    *addr = value;
}

void uart_init()
{
    /*
    * enable receive interrupts.
    */
    uint8_t ier = uart_read_reg(IER);
    uart_write_reg(IER, ier | (1 << 0));
}


int uart_putc(char ch)
{
// 为了minicom的换行，得多加一个
//if (ch == '\n') {
//        uart_putc('\r');
//    }
//可以在minicom改一改

    while ((uart_read_reg(LSR) & LSR_TX_IDLE) == 0);
    uart_write_reg(THR, ch);
    return (unsigned char)ch;
}

void uart_puts(char *s)
{
	while (*s) {
		uart_putc(*s++);
	}
}

int uart_getc(void)
{
    if (uart_read_reg(LSR) & LSR_RX_READY) {
        return uart_read_reg(RBR);
    } else {
        return -1;
    }
}

/*
 * handle a uart interrupt, raised because input has arrived, called from trap.c.
 */
void uart_isr(void)
{
	while (1) {
		int c = uart_getc();
		if (c == -1) {
			break;
		} else {
			uart_putc((char)c);
			uart_putc('\n');
		}
	}
}
