
include $(NCCU_SOURCE_DIR)/include.mk

#the target output file
EXEC = sampler_stbatt.so

#the head files the object files depend on
INCS = st_batt_sampler.h 

#the object files to generate the target
OBJS = st_batt_sampler.o  can_nd_v33_batt.o can_infy_batt.o st_batt_datasave.o st_batt_health.o  ../sampler_pubfunc.o st_batt_statemachine.o imeu2_can.o imeu3_can.o
OBJS += st_batt_txt_upgrade.o modbus_fengch_batt.o st_batt_testmode.o st_batt_testmode_tow.o st_batt_testmode_ShortTime.o rpmsg_infy_batt.o
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
	
