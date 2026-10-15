
include $(NCCU_SOURCE_DIR)/include.mk

#the target output file
EXEC = nccu_app

#the head files the object files depend on
INCS = main.h

#the object files to generate the target
OBJS += ../moduals/data_processor/dpr_loader.o
OBJS += ../moduals/samplers/sampler_loader.o
OBJS += ../comm_ports/comm_loader.o
OBJS += ../data_interfaces/dai_loader.o
OBJS += ../moduals/reporters/reporters_loader.o
OBJS += main.o watch_dog.o main_args.o sig_handler.o

#for self test
TEST_EXEC = #test_wdt #the_test_file_execution
TEST_OBJS = #$(DIR1)/watch_dog.o #the_object_files_for_testing.o
TEST_LIBS = #and other required libs

#private shared libraries
LDFLAGS += -L$(NCCU_SOURCE_DIR)/moduals/data_processor/dpr
LIBS = -ldpr $(NCCU_SOURCE_DIR)/include/3rd_party/curl/libcurl.so.4

all: $(EXEC)

$(EXEC): $(OBJS) $(INCS) $(PUBINCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) $(PUBLIBS)


install: $(EXEC)
	cp -fr $(EXEC) $(NCCU_TARGET_DIR)/bin/$(EXEC)
	chmod +x $(NCCU_TARGET_DIR)/bin/$(EXEC)

$(TEST_EXEC): $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJS) $(TEST_LIBS)
	
test: $(TEST_EXEC)
	-echo Test making ...

clean:
	-rm -f $(EXEC) *.gdb *.elf *.o $(OBJS) core $(TEST_EXEC)
	-rm -f make_*.txt *.map *.so  
	-rm -f $(NCCU_TARGET_DIR)/moduals/data_processor/*.o
	-rm -f $(NCCU_TARGET_DIR)/moduals/samplers/*.o


