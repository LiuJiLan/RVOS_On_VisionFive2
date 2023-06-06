include ../common.mk

SYSCALL = y

ifeq (${SYSCALL}, y)
CFLAGS += -D CONFIG_SYSCALL
endif

SRCS_ASM = \
	start.S \
	mem.S \
	entry.S \
	usys.S

SRCS_C = \
	kernel.c \
	uart.c \
	printf.c \
	page.c \
	sched.c \
	user.c \
	trap.c \
	plic.c \
	timer.c \
	lock.c \
	syscall.c

OBJS = $(SRCS_ASM:.S=.o)
OBJS += $(SRCS_C:.c=.o)

.DEFAULT_GOAL := all
all: rvos_image.bin

# start.o must be the first in dependency!
os.elf: ${OBJS}
	${CC} ${CFLAGS} -T os.ld -o os.elf $^

rvos_image.bin: os.elf
	${OBJCOPY} -O binary os.elf os.bin
	#用uboot工具中的制作可引导的镜像
	mkimage -A riscv -O linux -T kernel -C none -a 0x40200000 -e 0x40200000 -d os.bin rvos_image.bin

%.o : %.c
	${CC} ${CFLAGS} -c -o $@ $<

%.o : %.S
	${CC} ${CFLAGS} -c -o $@ $<

QFLAGS = -nographic -smp 1 -machine virt -bios default

run: all
	@${QEMU} -M ? | grep virt >/dev/null || exit
	@echo "Press Ctrl-A and then X to exit QEMU"
	@echo "------------------------------------"
	@${QEMU} ${QFLAGS} -kernel os.elf

.PHONY : debug
debug: os.elf
	@echo "Press Ctrl-C and then input 'quit' to exit GDB and QEMU"
	@echo "-------------------------------------------------------"
	@${QEMU} ${QFLAGS} -kernel os.elf -s -S &
	@${GDB} os.elf -q -x ./gdbinit

.PHONY : code
code: all
	@${OBJDUMP} -S os.elf | less

.PHONY : clean
clean:
	rm -rf *.o *.bin *.elf *.out

