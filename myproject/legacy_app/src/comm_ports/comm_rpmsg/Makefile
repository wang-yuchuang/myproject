
include $(NCCU_SOURCE_DIR)/include.mk

#the target output file
EXEC = libcomm_rpmsg.so

#the head files the object files depend on
INCS = commrpmsg.h

#the object files to generate the target
OBJS = commrpmsg.o

#for self test
#TEST_EXEC = test_comm_rpmsg
#TEST_OBJS = test_comm_rpmsg.o
#TEST_LIBS = $(EXEC) -lpthread
CFLAGS += -I$(NCCU_SOURCE_DIR)/include
CFLAGS += -I$(CURDIR)/rpmsg_bsp/inc
CFLAGS += -fPIC

SRCS = commrpmsg.c \
       rpmsg_bsp/src/iccmsg_hal.c \
       rpmsg_bsp/src/iccmsg_dpr.c \
       rpmsg_bsp/src/iccmsg_ll.c \
       rpmsg_bsp/src/iccmsg_queue.c \
       rpmsg_bsp/src/iccmsg.c \
       rpmsg_bsp/src/rpch_common.c \
       rpmsg_bsp/src/sd_crc32.c

OBJS = $(SRCS:.c=.o)

#private shared libraries
LIBS = -dl -lpub

all: $(EXEC)

$(EXEC): $(OBJS) $(INCS) $(PUBINCS)
	$(CC) -shared -fPIC $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

install: $(EXEC)
	-cp -f $(EXEC) $(NCCU_TARGET_DIR)/comm_port/
	$(STRIP) -s -o $(NCCU_TARGET_DIR)/comm_port/$(EXEC) $(EXEC)

$(TEST_EXEC): $(EXEC) $(TEST_OBJS) $(INCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJS) $(TEST_LIBS)
	
test: #$(TEST_EXEC)
	-echo Test making ...

clean:
	-rm -f $(EXEC) *.gdb *.elf *.o $(OBJS) core $(TEST_EXEC)
	-rm -f make_*.log *.map *.so

