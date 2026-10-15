
include $(NCCU_SOURCE_DIR)/include.mk

#the target output file
EXEC = sampler_ccu.so

#the head files the object files depend on
INCS = ccu_sampler_main.h
#the object files to generate the target
# LJH241101 MU3
OBJS = ccu_sampler_main.o
ifeq ($(MU_TYPE), mu3)
	OBJS += ccu_com_232.o
else
	OBJS += ccu_com_eth.o
endif
# ESS
OBJS += ess_sampler_main.o ess_ems_management.o
# LJH241111 PcsSCADA
# OBJS += inpcs_sampler_main.o inpcs_statemachine.o inpcs235kw_statemachine.o
# LJH241201 SEMS
OBJS += sems_sampler_main.o sems_ems_management.o sems_statemachine.o sems_msg_handler.o sems_msg_parsers.o
OBJS += ../sampler_pubfunc.o
OBJS += ccu_protocol_parsers.o
OBJS += ems_management.o
OBJS += ccu_history_esdata.o
OBJS +=	ccu_pcs_handle.o
OBJS += ccu_bin_upgrade.o
OBJS += ccu_rundata_update.o
OBJS += ccu_rpmsg_parsers.o
OBJS += ccu_com_rpmsg.o 
OBJS += power_device_unit/cansend.o power_device_unit/N2402.o power_device_unit/data_exchange.o power_device_unit/can_comm_main.o
 
#########Standalone
#INCS += ./ccu_232_standalone/ccu_232_sample.h
#OBJS += ./ccu_232_standalone/ccu_232_sample.o 
########Multimain
#INCS += ./ccu_tcp_multimain/multimain_tcp_sample.h
#OBJS += ./ccu_tcp_multimain/multimain_tcp_sample.o 

########Inter Slave Tcp main
#INCS += ./ccu_tcp_slave/inter_slave_main.h
#OBJS += ./ccu_tcp_slave/inter_slave_main.o 
#OBJS += ./ccu_tcp_slave/inter_slv_cmd_handler.o 

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
	-mkdir -p $(NCCU_TARGET_DIR)/sampler/
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
	-rm -f power_device_unit/*.o power_device_unit/*.baksed
	#-rm -f $(NCCU_TARGET_DIR)/sampler/$(EXEC)
	
