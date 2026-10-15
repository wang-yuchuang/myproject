include $(NCCU_SOURCE_DIR)/include.mk

#the target output file
EXEC = libcomm_can.so

#the head files the object files depend on
INCS = comm_socketcan.h

#the object files to generate the target
OBJS = comm_socketcan.o

#for self test
TEST_EXEC = 
TEST_OBJS = 
TEST_LIBS = $(EXEC) -lpthread #and other required libs

#private shared libraries

CFLAGS += -I$(NCCU_SOURCE_DIR)/include/3rd_party/libsocketcan
LDFLAGS += -L$(NCCU_SOURCE_DIR)/include/3rd_party/libsocketcan
LIBS += -lsocketcan

all: $(EXEC)

$(EXEC): $(OBJS) $(INCS) $(PUBINCS)
	$(CC) -shared -fPIC $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

install: $(EXEC)
	-cp -f $(EXEC) $(NCCU_TARGET_DIR)/comm_port/
	$(STRIP) -s -o $(NCCU_TARGET_DIR)/comm_port/$(EXEC) $(EXEC)

$(TEST_EXEC): $(EXEC) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJS) $(TEST_LIBS) $(LIBS)
	
test: $(TEST_EXEC)
	-echo Test making ...

clean:
	-rm -f $(EXEC) *.gdb *.elf *.o $(OBJS) core $(TEST_EXEC)
	-rm -f make_*.log *.map *.so

