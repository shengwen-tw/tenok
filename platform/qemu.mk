-include ../config.mk

LD_SCRIPT += platform/stm32f407.ld

CFLAGS += -D STM32F40_41xxx \
	  -D ENABLE_UART1_DMA=0 \
	  -D ENABLE_UART3_DMA=0 \
	  -D BUILD_QEMU

CFLAGS += -D__BOARD_NAME__=\"stm32f407\"

CFLAGS += -I./drivers/boards

SRC += ./drivers/boards/stm32f4disc.c

qemu: all
	$(QEMU) \
	-nographic \
	-cpu cortex-m4 \
	-M netduinoplus2 \
	-serial mon:stdio \
	-serial pty \
	-serial pty \
	-gdb tcp::3333 \
	-kernel ./$(ELF)

.PHONY: qemu
