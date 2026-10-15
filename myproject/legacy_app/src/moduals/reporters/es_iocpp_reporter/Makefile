
include $(NCCU_SOURCE_DIR)/include.mk

#the target output file
EXEC = reporter_es_iocpp.so 

#the head files the object files depend on
INCS = es_iocpp.h ../../cJSON.h

#the object files to generate the target
OBJS = es_iocpp_main.o  ../../cJSON.o es_iocpp_msgparser.o es_iocpp_msghandler.o ../reporters_pubfun.o

#for self test
TEST_EXEC = 
TEST_OBJS =
TEST_LIBS = $(EXEC) -lpthread #and other required libs

#private shared libraries
#LDFLAGS += -L$(NCCU_SOURCE_DIR)/data_interfaces
#LIBS = -lpub -lflash_dxi -lrunb_dxi

#private shared libraries
#LIBS = -dl -lpub $(NCCU_SOURCE_DIR)/include/3rd_party/curl/libcurl.so.4

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
	
