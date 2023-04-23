CROSS_COMPILE = riscv64-unknown-elf-
# CFLAGS = -nostdlib -fno-builtin -march=rv32ima -mabi=ilp32 -g -Wall
CFLAGS = -nostdlib -fno-builtin -g -ggdb -Wall
CFLAGS += -mcmodel=medany -mabi=lp64f -march=rv64imafc

# QEMU = qemu-system-riscv32
QEMU = qemu-system-riscv64
QFLAGS = -nographic -smp 1 -machine virt -bios none

GDB = ${CROSS_COMPILE}gdb
CC = ${CROSS_COMPILE}gcc
OBJCOPY = ${CROSS_COMPILE}objcopy
OBJDUMP = ${CROSS_COMPILE}objdump

