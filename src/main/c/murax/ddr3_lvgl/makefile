PROJ_NAME=ddr3_lvgl
DEBUG=no
BENCH=no
MULDIV=no

SRCS = 	$(wildcard src/*.c) 	\
		$(wildcard src/*.cpp)  	\
		$(wildcard src/*.S)		\
		$(wildcard src/kernel/*.c)  \
		$(wildcard src/spi/*.c)  \
		$(wildcard src/sd_ctl/*.c)  \
		$(wildcard src/interrupt/*.c)	\
		$(wildcard src/lvgl/examples/porting/*.c) \
		$(wildcard src/lvgl/src/core/*.c) \
		$(wildcard src/lvgl/src/draw/*.c) \
		$(wildcard src/lvgl/src/draw/sw/*.c) \
		$(wildcard src/lvgl/src/extra/*.c) \
		$(wildcard src/lvgl/src/extra/themes/default/*.c) \
		$(wildcard src/lvgl/src/extra/layouts/flex/*.c) \
		$(wildcard src/lvgl/src/extra/layouts/grid/*.c) \
		$(wildcard src/lvgl/src/font/*.c) \
		$(wildcard src/lvgl/src/hal/*.c) \
		$(wildcard src/lvgl/src/misc/*.c) \
		$(wildcard src/lvgl/src/widgets/*.c) \
		$(wildcard src/lvgl/src/extra/widgets/animimg/*.c) \
		$(wildcard src/lvgl/src/extra/widgets/calendar/*.c) \
		$(wildcard src/lvgl/src/extra/widgets/chart/*.c) \
		$(wildcard src/lvgl/src/extra/widgets/colorwheel/*.c) \
		$(wildcard src/lvgl/src/extra/widgets/imgbtn/*.c) \
		$(wildcard src/lvgl/src/extra/widgets/keyboard/*.c) \
		$(wildcard src/lvgl/src/extra/widgets/led/*.c) \
		$(wildcard src/lvgl/src/extra/widgets/list/*.c) \
		$(wildcard src/lvgl/src/extra/widgets/menu/*.c) \
		$(wildcard src/lvgl/src/extra/widgets/meter/*.c) \
		$(wildcard src/lvgl/src/extra/widgets/msgbox/*.c) \
		$(wildcard src/lvgl/src/extra/widgets/span/*.c) \
		$(wildcard src/lvgl/src/extra/widgets/spinbox/*.c) \
		$(wildcard src/lvgl/src/extra/widgets/spinner/*.c) \
		$(wildcard src/lvgl/src/extra/widgets/tabview/*.c) \
		$(wildcard src/lvgl/src/extra/widgets/tileview/*.c) \
		$(wildcard src/lvgl/src/extra/widgets/win/*.c) \
		$(wildcard src/lvgl/demos/benchmark/*.c) \
		$(wildcard src/lvgl/demos/benchmark/assets/*.c)\
		$(wildcard src/lvgl/demos/stress/*.c)

OBJDIR = build

LVGL_ROOT = ./src/lvgl

INC  = -I$(LVGL_ROOT)/examples/porting -I$(LVGL_ROOT)/src -I$(LVGL_ROOT) -I$(LVGL_ROOT)/demos
LIBS =
LIBSINC = -L$(OBJDIR) -L$(LVGL_ROOT)/examples/porting
LDSCRIPT = ./src/linker.ld

#include ../../../resources/gcc.mk
# Set it to yes if you are using the sifive precompiled GCC pack
# SIFIVE_GCC_PACK ?= yes

ifeq ($(SIFIVE_GCC_PACK),yes)
	RISCV_NAME ?= riscv64-unknown-elf
	RISCV_PATH ?= /opt/riscv
else
	RISCV_NAME ?= riscv32-unknown-elf
	ifeq ($(MULDIV),yes)
		RISCV_PATH ?= /opt/rv32im
	else
		RISCV_PATH ?= /opt/rv32i
	endif
endif

MABI=ilp32
MARCH := rv32i
ifeq ($(MULDIV),yes)
	MARCH := $(MARCH)m
endif
ifeq ($(COMPRESSED),yes)
	MARCH := $(MARCH)ac
endif

MARCH := $(MARCH)_zicsr 

CFLAGS += -march=$(MARCH)  -mabi=$(MABI) -DNDEBUG
LDFLAGS += -march=$(MARCH)  -mabi=$(MABI)



#include ../../../resources/subproject.mk


ifeq ($(DEBUG),yes)
	CFLAGS += -g3 -O0
endif

ifeq ($(DEBUG),no)
	CFLAGS += -g -Os
	# CFLAGS += -O0
endif

ifeq ($(BENCH),yes)
	CFLAGS += -fno-inline
endif

ifeq ($(SIFIVE_GCC_PACK),yes)
	RISCV_CLIB=$(RISCV_PATH)/$(RISCV_NAME)/lib/$(MARCH)/$(MABI)/
else
	RISCV_CLIB=$(RISCV_PATH)/$(RISCV_NAME)/lib/
endif





RISCV_OBJCOPY = $(RISCV_PATH)/bin/$(RISCV_NAME)-objcopy
RISCV_OBJDUMP = $(RISCV_PATH)/bin/$(RISCV_NAME)-objdump
RISCV_CC=$(RISCV_PATH)/bin/$(RISCV_NAME)-gcc

# CFLAGS +=  -MD -fstrict-volatile-bitfields -fno-strict-aliasing
CFLAGS +=  -MD
# LDFLAGS +=  -nostdlib -lgcc -mcmodel=medany -nostartfiles -ffreestanding -Wl,-Bstatic,-T,$(LDSCRIPT),-Map,$(OBJDIR)/$(PROJ_NAME).map,--print-memory-usage
# LDFLAGS += -lgcc -mcmodel=medany -nostartfiles -ffreestanding -Wl,-Bstatic,-T,$(LDSCRIPT),-Map,$(OBJDIR)/$(PROJ_NAME).map,--print-memory-usage -Wl,--gc-sections 
LDFLAGS += -lgcc -mcmodel=medany -Wl,-Bstatic,-T,$(LDSCRIPT),-Map,$(OBJDIR)/$(PROJ_NAME).map,--print-memory-usage -Wl,--gc-sections 
#LDFLAGS += -lgcc -lc -lg -nostdlib  -lgcc  -msave-restore    --strip-debug,

OBJS := $(SRCS)
OBJS := $(OBJS:.c=.o)
OBJS := $(OBJS:.cpp=.o)
OBJS := $(OBJS:.S=.o)
OBJS := $(OBJS:..=miaou)
OBJS := $(addprefix $(OBJDIR)/,$(OBJS))


all: $(OBJDIR)/$(PROJ_NAME).elf $(OBJDIR)/$(PROJ_NAME).hex $(OBJDIR)/$(PROJ_NAME).asm $(OBJDIR)/$(PROJ_NAME).v

$(OBJDIR)/%.elf: $(OBJS) | $(OBJDIR)
	$(RISCV_CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBSINC) $(LIBS)

%.hex: %.elf
	$(RISCV_OBJCOPY) -O ihex $^ $@

%.bin: %.elf
	$(RISCV_OBJCOPY) -O binary $^ $@

%.v: %.elf
	$(RISCV_OBJCOPY) -O verilog $^ $@

%.asm: %.elf
	$(RISCV_OBJDUMP) -S -d $^ > $@

$(OBJDIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(RISCV_CC) -c $(CFLAGS)  $(INC) -o $@ $^
	$(RISCV_CC) -S $(CFLAGS)  $(INC) -o $@.disasm $^

$(OBJDIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(RISCV_CC) -c $(CFLAGS)  $(INC) -o $@ $^

$(OBJDIR)/%.o: %.S
	mkdir -p $(dir $@)
	$(RISCV_CC) -c $(CFLAGS) -o $@ $^ -D__ASSEMBLY__=1

$(OBJDIR):
	mkdir -p $@

.PHONY: clean
clean:
	rm -rf $(OBJDIR)/src
	rm -f $(OBJDIR)/$(PROJ_NAME).elf
	rm -f $(OBJDIR)/$(PROJ_NAME).hex
	rm -f $(OBJDIR)/$(PROJ_NAME).map
	rm -f $(OBJDIR)/$(PROJ_NAME).v
	rm -f $(OBJDIR)/$(PROJ_NAME).asm
	find $(OBJDIR) -type f -name '*.o' -print0 | xargs -0 -r rm
	find $(OBJDIR) -type f -name '*.d' -print0 | xargs -0 -r rm

clean-all : clean 

.SECONDARY: $(OBJS)
