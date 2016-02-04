# Makefile

all		: rcu-sh
clean		: clean_apps

# Edit the line below to modify a set of user-space programs
# you need to build 

DEPS = cmdInterpreter.h
#rcu_sh_SRC	= RCU_shell.c cmdInterpreter.c
rcu_sh_OBJ	= RCU_shell.o cmdInterpreter.o
RCU_shell.o	: RCU_shell.c $(DEPS)
		  $(CC) -c RCU_shell.c $(CFLAGS)
cmdInterpreter.o : cmdInterpreter.c $(DEPS)
		  $(CC) -c cmdInterpreter.c $(CFLAGS)
rcu-sh		: $(rcu_sh_OBJ)
		  $(CC) $(LDFLAGS) -o rcu-sh $(rcu_sh_OBJ)

jtagtest    : jtagtest.c
		  $(CC) jtagtest.c -o jtagtest $(CFLAGS)

clocktest	: clocktest.c
		  $(CC) clocktest.c -o clocktest $(CFLAGS)

# tests
#regmap.o	:	regmap.h
#		   $(CC) -c regmap.h $(CFLAGS)
comblk.o	: comblk.c comblk.h regmap.h
		  $(CC) -c comblk.c $(CFLAGS)
sys_services.o : sys_services.c sys_services.h comblk.h regmap.h
		  $(CC) -c sys_services.c $(CFLAGS)
read.o		: read.c sys_services.h comblk.h
		   $(CC) -c read.c $(CFLAGS)
rd		: read.o comblk.o sys_services.o
		  $(CC) $(LDFLAGS) -o rd read.o comblk.o sys_services.o $(CFLAGS)

# system controller stuff
mss_sys_services.o : sc_drivers/mss_sys_services/mss_sys_services.c sc_drivers/mss_sys_services/mss_sys_services.h sc_drivers/mss_sys_services/mss_comblk.h sc_drivers/CMSIS/mss_assert.h
		  $(CC) -c sc_drivers/mss_sys_services/mss_sys_services.c $(CFLAGS)
		  
mss_comblk.o : sc_drivers/mss_sys_services/mss_comblk.c sc_drivers/mss_sys_services/mss_comblk.h sc_drivers/CMSIS/mss_assert.h
		  $(CC) -c sc_drivers/mss_sys_services/mss_comblk.c $(CFLAGS)
		  
systemctrltest.o	: systemctrltest.c sc_drivers/mss_sys_services/mss_sys_services.h
		  $(CC) -c systemctrltest.c $(CFLAGS)
		  
readVer	: systemctrltest.o mss_sys_services.o mss_comblk.o 
		  $(CC) $(LDFLAGS) -o readVer systemctrltest.o mss_sys_services.o mss_comblk.o $(CFLAGS)

# These are flags/tools used to build user-space programs
CFLAGS		:= -Os -mcpu=cortex-m3 -mthumb -Wall -lm -I/home/lars/work/linux-cortexm-1.11.0 -O0 -g
LDFLAGS		:= -mcpu=cortex-m3 -mthumb -Wall -I/home/lars/work/linux-cortexm-1.11.0
CC		= $(CROSS_COMPILE_APPS)gcc

# Clean-up after user-space programs
clean_apps	:
	-rm -f $(APPS) *.gdb *.o

# Edit the line below to modify a set of loadable modules
# you need to build 
#obj-m		+= sample.o

# Define dependencies for a particular module
# sample-y	:= some.o

# Path to the kernel modules directory in context of which
# these loadable modules are built
KERNELDIR	:=  $(INSTALL_ROOT)/linux

# Build the modules in this directory
modules		:
	make -C $(KERNELDIR) M=`pwd` modules CFLAGS="" LDFLAGS=""

# Clean-up after modules
clean_modules:
	make -C $(KERNELDIR) M=`pwd` clean
	rm -f modules.order

