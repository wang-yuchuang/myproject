
include $(NCCU_SOURCE_DIR)/include.mk

#the target output file
EXEC = sampler_uart_server.so

#the head files the object files depend on
INCS = uart_server_main.h

#the object files to generate the target
OBJS = uart_server_main.o ../sampler_pubfunc.o modbus_rtu.o dlt645_kWmeter.o


#CFLAGS += -I$(SAMPLER_BASE_DIR)
#CFLAGS += -D_DLL_	#to make DLL(.so) version

#for self test
TEST_EXEC = #test_scu #the_test_file_execution
TEST_OBJS = #the_object_files_for_testing.o
TEST_LIBS = $(EXEC) #and other required libs


all: $(EXEC)

$(EXEC): $(OBJS) $(INCS) $(PUBINCS)
	$(CC) -shared -fPIC $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

install: $(EXEC)
	-cp $(EXEC) $(NCCU_TARGET_DIR)/sampler/
	$(STRIP) -s -o $(NCCU_TARGET_DIR)/sampler/$(EXEC) $(EXEC)
	chmod +x $(NCCU_TARGET_DIR)/sampler/$(EXEC)

$(TEST_EXEC): $(EXEC) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJS) $(TEST_LIBS)
	
test: $(TEST_EXEC)
	-echo Test making ...

clean:
	-rm -f $(EXEC) *.gdb *.elf *.o $(OBJS) core $(TEST_EXEC)
	-rm -f make_*.txt *.map *.so 
	#-rm -f $(NCCU_TARGET_DIR)/sampler/$(EXEC)
	
