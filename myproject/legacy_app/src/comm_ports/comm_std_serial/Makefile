
include $(NCCU_SOURCE_DIR)/include.mk

#the target output file
EXEC = libcomm_serial.so

#the head files the object files depend on
INCS = commserial.h

#the object files to generate the target
OBJS = commserial.o

#for self test
#TEST_EXEC = test_comm_serial
#TEST_OBJS = test_comm_serial.o
#TEST_LIBS = $(EXEC) -lpthread

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

