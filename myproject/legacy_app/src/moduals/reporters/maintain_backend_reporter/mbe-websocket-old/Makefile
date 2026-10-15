
include $(NCCU_SOURCE_DIR)/include.mk

#the target output file
EXEC = reporter_maintainbe.so 

#the head files the object files depend on
INCS = maitain_be.h ../../../cJSON.h

#the object files to generate the target
OBJS = maitain_be_main.o  ../../../cJSON.o maitain_be_msgparser.o maitain_be_msghandler.o

#for self test
TEST_EXEC = 
TEST_OBJS =
TEST_LIBS = $(EXEC) -lpthread #and other required libs

#private shared libraries
#LDFLAGS += -L$(NCCU_SOURCE_DIR)/data_interfaces
#LIBS = -lpub -lflash_dxi -lrunb_dxi

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
	
