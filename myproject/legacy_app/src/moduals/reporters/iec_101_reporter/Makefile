
include $(NCCU_SOURCE_DIR)/include.mk

#the target output file
EXEC = reporter_iec101.so 

#the head files the object files depend on
INCS = iec_101.h  

# iedinf.h

#the object files to generate the target
OBJS = iec_101_main.o iec_101_handler.o

#for self test
TEST_EXEC = 
TEST_OBJS =
TEST_LIBS = $(EXEC) -lpthread #and other required libs

#private shared libraries
#LDFLAGS += -L$(NCCU_SOURCE_DIR)/data_interfaces
#LIBS = -lpub -lflash_dxi -lrunb_dxi
LIBS = $(NCCU_SOURCE_DIR)/include/3rd_party/libiec104/lib60870.a

#private shared libraries
#LIBS = -dl -lpub -L$(NCCU_SOURCE_DIR)/include/3rd_party/libpahomqtt -lpaho-mqtt3cs -lpaho-mqtt3c

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
	-rm -f $(EXEC) *.gdb *.elf *.o ../*.o $(OBJS) core $(TEST_EXEC)
	-rm -f make_*.log *.map *.so
	
