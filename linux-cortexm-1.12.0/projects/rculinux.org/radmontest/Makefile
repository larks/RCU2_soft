all : radmontest radmondump radmonpoke

# These are flags/tools used to build user-space programs
CFLAGS		:= -Os -mcpu=cortex-m3 -mthumb
LDFLAGS		:= -mcpu=cortex-m3 -mthumb
CC		= $(CROSS_COMPILE_APPS)gcc
CXX		= $(CROSS_COMPILE_APPS)g++
CXXFLAGS	= ${CFLAGS} -std=c++0x

OBJS := main.o Radmon.o TestFixture.o
radmontest : ${OBJS}
	${CXX} ${LDFLAGS} ${OBJS} -o $@

radmondump : radmon_dump.o Radmon.o
	${CXX} ${LDFLAGS} radmon_dump.o Radmon.o -o $@

radmonpoke : radmonpoke.o
	${CC} ${LDFLAGS} $^ -o $@

# Clean-up after user-space programs
clean :
	-rm -f radmontest radmondump *.gdb *.o
