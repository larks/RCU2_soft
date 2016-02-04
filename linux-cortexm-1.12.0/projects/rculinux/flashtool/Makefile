all : flashtool spidev_flash spidev_bootcmd spidev_isp_prog spidev_chenv spidev_ip spidev_getvar spidev_uboot cptf

# These are flags/tools used to build user-space programs
CFLAGS		:= -Os -mcpu=cortex-m3 -mthumb -std=c99 -I$(INSTALL_ROOT)/A2F/root/usr/include -L${INSTALL_ROOT}/A2F/root/usr/lib -pthread 
LDFLAGS		:= -mcpu=cortex-m3 -mthumb -L${INSTALL_ROOT}/A2F/root/usr/lib -pthread
CC		= $(CROSS_COMPILE_APPS)gcc

OBJS := flashtool.o 
OBJS2 := spidev_flash.o 
OBJS3 := spidev_bootcmd.o 
OBJS4 := spidev_isp_prog.o 
OBJS5 := spidev_chenv.o 
OBJS6 := spidev_ip.o 
OBJS7 := spidev_getvar.o 
OBJS8 := spidev_uboot.o 
OBJS9 := cptf.o 

flashtool : ${OBJS}
	${CC} ${LDFLAGS} ${OBJS} -o $@

spidev_flash: ${OBJS2}
	$(CC) $(LDFLAGS) ${OBJS2} -o $@


spidev_bootcmd: ${OBJS3}
	$(CC) $(LDFLAGS) ${OBJS3} -o $@


spidev_isp_prog: ${OBJS4}
	$(CC) $(LDFLAGS) ${OBJS4} -o $@

spidev_chenv: ${OBJS5}
	$(CC) $(LDFLAGS) ${OBJS5} -o $@

spidev_ip: ${OBJS6}
	$(CC) $(LDFLAGS) ${OBJS6} -o $@

spidev_getvar: ${OBJS7}
	$(CC) $(LDFLAGS) ${OBJS7} -o $@


spidev_uboot: ${OBJS8}
	$(CC) $(LDFLAGS) ${OBJS8} -o $@


cptf: ${OBJS9}
	$(CC) $(LDFLAGS) ${OBJS9} -o $@


# Clean-up after user-space programs
clean :
	-rm -f flashtool spidev_flash spidev_bootcmd spidev_isp_prog spidev_chenv spidev_ip spidev_getvar spidev_uboot *.gdb *.o

