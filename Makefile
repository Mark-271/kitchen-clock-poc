# Check if path to libopencm3 directory is specified.
ifeq ($(strip $(OPENCM3_DIR)),)
  $(error Please specify libopencm3 library dir through OPENCM3_DIR variable!)
endif

# Show libopencm3 dir path.
ifeq ($(V),1)
  $(info Using $(OPENCM3_DIR) path to library)
endif

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
  Q		:= @
endif
endif

APP		:= thermometer
LIB_DIR		= $(OPENCM3_DIR)/lib
INCLUDE_DIR	= $(OPENCM3_DIR)/include
LDSCRIPT	= stm32vl-discovery.ld

LIBNAME		= opencm3_stm32f1
DEFS		+= -DSTM32F1
DEFS		+= -Iinclude
DEFS		+= -I$(INCLUDE_DIR)

FP_FLAGS	?= -msoft-float
ARCH_FLAGS	= -mthumb -mcpu=cortex-m3 $(FP_FLAGS) -mfix-cortex-m3-ldrd

CC		:= $(CROSS_COMPILE)gcc
CXX		:= $(CROSS_COMPILE)g++
LD		:= $(CROSS_COMPILE)gcc
AR		:= $(CROSS_COMPILE)ar
AS		:= $(CROSS_COMPILE)as
OBJCOPY		:= $(CROSS_COMPILE)objcopy
OBJDUMP		:= $(CROSS_COMPILE)objdump
GDB		:= $(CROSS_COMPILE)gdb
STFLASH		= $(shell which st-flash)
OPT		:= -O2
DEBUG		:= -ggdb3
CSTD		?= -std=gnu89

OBJS		+=			\
		   src/board.o		\
		   src/common.o		\
		   src/ds18b20.o	\
		   src/kbd.o		\
		   src/main.o		\
		   src/one_wire.o	\
		   src/sched.o		\
		   src/serial.o		\
		   src/tools.o		\
		   src/wh1602.o

# C flags

CFLAGS	+= $(OPT) $(CSTD) $(DEBUG)
CFLAGS	+= $(ARCH_FLAGS)
CFLAGS	+= -Wextra -Wshadow -Wimplicit-function-declaration
CFLAGS	+= -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes
CFLAGS	+= -fno-common -ffunction-sections -fdata-sections

# C preprocessor common flags

CPPFLAGS	+= -MD
CPPFLAGS	+= -Wall -Wundef
CPPFLAGS	+= $(DEFS)

# Linker flags

LDFLAGS		+= -L$(LIB_DIR)
LDFLAGS		+= --static -nostartfiles
LDFLAGS		+= -T$(LDSCRIPT)
LDFLAGS		+= $(ARCH_FLAGS) $(DEBUG)
LDFLAGS		+= -Wl,--gc-sections

# Used libraries

LDLIBS		+= -l$(LIBNAME)
LDLIBS		+= -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

# --------------------------------------------------------------------

# Workaround for Make regression: don't invoke .SECONDARY for completion
ifeq ($(filter npq%,$(firstword $(MAKEFLAGS))),)
.SECONDARY:
endif

all: elf
elf: $(APP).elf
bin: $(APP).bin

%.bin: %.elf
	@printf "  OBJCOPY $(*).bin\n"
	$(Q)$(OBJCOPY) -Obinary $(*).elf $(*).bin

%.elf: $(OBJS) $(LDSCRIPT) $(LIB_DIR)/lib$(LIBNAME).a
	@printf "  LD      $(*).elf\n"
	$(Q)$(LD) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(*).elf

%.o: %.c
	@printf "  CC      $(*).c\n"
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -o $(*).o -c $(*).c

clean:
	@printf "  CLEAN\n"
	$(Q)$(RM) $(APP).elf $(APP).bin $(OBJS) $(OBJS:%.o=%.d)

distclean: clean
	@printf "  DISTCLEAN\n"
	$(Q)$(RM) cscope* tags

flash: $(APP).bin
	@printf "  FLASH  $<\n"
	$(STFLASH) write $(APP).bin 0x8000000

.PHONY: clean distclean flash elf bin

-include $(OBJS:.o=.d)
