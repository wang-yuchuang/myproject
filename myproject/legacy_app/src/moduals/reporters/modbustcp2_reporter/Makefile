include $(NCCU_SOURCE_DIR)/include.mk

#the target output file
EXEC = reporter_modbustcp2.so

 #the head files the object files depend on
INCS = modbus.h

#the object files to generate the target
OBJS = modbustcp2_main.o handle_frame.o modbustcp2_datamap.o ../reporters_pubfun.o
 

#for self test
TEST_EXEC = 
TEST_OBJS =
TEST_LIBS = $(EXEC) -lpthread #and other required libs

LIBS = $(NCCU_SOURCE_DIR)/include/3rd_party/libsocketcan/libsocketcan.so.2.2.0
 
all: $(EXEC)

$(EXEC): $(OBJS) $(INCS) $(PUBINCS)
	$(CC) -shared -fPIC $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	
install: $(EXEC)
	-cp $(EXEC) $(NCCU_TARGET_DIR)/reporter/
	
$(TEST_EXEC): $(EXEC) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJS) $(TEST_LIBS)
	
test: $(TEST_EXEC)
	-echo Test making ...

clean:
	-rm -f $(EXEC) *.gdb *.elf *.o $(OBJS) core $(TEST_EXEC)
	-rm -f make_*.log *.map *.so
	
