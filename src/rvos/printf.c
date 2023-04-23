#include "os.h"

static int _vsnprintf(char * out, size_t n, const char* s, va_list vl)
{
    int format = 0;
    int longarg = 0;
    size_t pos = 0;
    for (; *s; s++) {
        if (format) {
            switch(*s) {
            case 'l': {
                longarg = 1;
                break;
            }
            case 'p': {
                longarg = 1;
                if (out && pos < n) {
                    out[pos] = '0';
                }
                pos++;
                if (out && pos < n) {
                    out[pos] = 'x';
                }
                pos++;
            }
            case 'x': {
                long long num = longarg ? va_arg(vl, long long) : va_arg(vl, long);
                int hexdigits = 2*(longarg ? sizeof(long long) : sizeof(long))-1;
                for(int i = hexdigits; i >= 0; i--) {
                    int d = (num >> (4*i)) & 0xF;
                    if (out && pos < n) {
                        out[pos] = (d < 10 ? '0'+d : 'a'+d-10);
                    }
                    pos++;
                }
                longarg = 0;
                format = 0;
                break;
            }
            case 'd': {
                long long num = longarg ? va_arg(vl, long long) : va_arg(vl, long);
                if (num < 0) {
                    num = -num;
                    if (out && pos < n) {
                        out[pos] = '-';
                    }
                    pos++;
                }
                long long digits = 1;
                for (long long nn = num; nn /= 10; digits++);
                for (int i = digits-1; i >= 0; i--) {
                    if (out && pos + i < n) {
                        out[pos + i] = '0' + (num % 10);
                    }
                    num /= 10;
                }
                pos += digits;
                longarg = 0;
                format = 0;
                break;
            }
            case 's': {
                const char* s2 = va_arg(vl, const char*);
                while (*s2) {
                    if (out && pos < n) {
                        out[pos] = *s2;
                    }
                    pos++;
                    s2++;
                }
                longarg = 0;
                format = 0;
                break;
            }
            case 'c': {
                if (out && pos < n) {
                    out[pos] = (char)va_arg(vl,long);
                }
                pos++;
                longarg = 0;
                format = 0;
                break;
            }
            default:
                break;
            }
        } else if (*s == '%') {
            format = 1;
        } else {
            if (out && pos < n) {
                out[pos] = *s;
            }
            pos++;
        }
    }
    if (out && pos < n) {
        out[pos] = 0;
    } else if (out && n) {
        out[n-1] = 0;
    }
    return pos;
}


static char out_buf[1000]; // buffer for _vprintf()

static int _vprintf(const char* s, va_list vl)
{
	int res = _vsnprintf(NULL, -1, s, vl);
	if (res+1 >= sizeof(out_buf)) {
		uart_puts("error: output string size overflow\n");
		while(1) {}
	}
	_vsnprintf(out_buf, res + 1, s, vl);
	uart_puts(out_buf);
	return res;
}

int printf(const char* s, ...)
{
	int res = 0;
	va_list vl;
	va_start(vl, s);
	res = _vprintf(s, vl);
	va_end(vl);
	return res;
}

void panic(char *s)
{
	printf("panic: ");
	printf(s);
	printf("\n");
	while(1){};
}

void see(reg_t a){
	printf("0x%x\n", a);
}

void asm_test(void) {
	uart_puts("Sound Good!\n");
}

void see_all(reg_t * a) {
	for (int i = 0; i < 32; i++) {
		printf("%d: 0x%x\n", i, *(a + i));
	}
}