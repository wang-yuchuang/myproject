
include $(NCCU_SOURCE_DIR)/include.mk

#the target output file
EXEC = web_main.api

#the head files the object files depend on
INCS = fcgi_lib/fastcgi.h fcgi_lib/fcgi_config.h fcgi_lib/fcgi_stdio.h fcgi_lib/fcgiapp.h fcgi_lib/fcgios.h
INCS += web_data_model.h fcgi_defs.h ../../../cJSON.h blob_param_mgmt.h
#fcgi_libs
OBJS = ../../../cJSON.o fcgi_lib/fcgiapp.o fcgi_lib/fcgi_stdio.o fcgi_lib/os_unix.o fcgi_lib/strerror.o
#api files
OBJS += session_mgmt.o rundata_exchange.o fcgi_main.o blob_param_mgmt.o pubfunc_web.o
OBJS += api_per_page/api_initialize.o api_per_page/api_login.o api_per_page/api_home.o api_per_page/api_quick_set.o api_per_page/api_detailed_info.o
OBJS += api_per_page/api_param_set.o api_per_page/api_pcs_param_set.o api_per_page/api_manual_ctrl.o  api_per_page/api_datetime_check.o 
OBJS += api_per_page/api_query_histalarm.o api_per_page/api_query_syslogs.o api_per_page/api_query_hisdata.o api_per_page/api_query_chgrecord.o
OBJS += api_per_page/api_data_backup_restore.o api_per_page/api_firmware_mgmt.o api_per_page/api_user_mgmt.o api_per_page/api_chgprofile_mgmt.o
OBJS += api_per_page/api_run_info.o api_per_page/api_about.o api_per_page/api_cell_barcode.o
OBJS += api_per_page/api_pcs_home.o api_per_page/api_pcs_query_histalarm.o api_per_page/api_pcs_run_info.o api_per_page/api_pcs_firmware_mgmt.o
OBJS += api_per_page/api_export_pcs.o api_per_page/api_param_reread.o api_per_page/api_bmsparam_sett.o api_per_page/api_bmsfault.o api_per_page/api_bmsevent.o
#edit by Kuangyue
#for self test
TEST_EXEC = 
TEST_OBJS =
TEST_LIBS = $(EXEC) -lpthread #and other required libs

#private shared libraries
#LDFLAGS += -L$(NCCU_SOURCE_DIR)/data_interfaces
#LIBS = -lpub -lflash_dxi -lrun_dxi
LIBS = $(NCCU_SOURCE_DIR)/include/3rd_party/nanomsg/libnanomsg.so.5.1.0 -L$(NCCU_SOURCE_DIR)/comm_ports/comm_ipc -lcomm_ipc

all: $(EXEC)

$(EXEC): $(OBJS) $(INCS) $(PUBINCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) $(PUBLIBS)
	
install: $(EXEC)
	-mkdir -p $(NCCU_TARGET_DIR)/web/www
	-cp $(EXEC) $(NCCU_TARGET_DIR)/web/www
	
$(TEST_EXEC): $(EXEC) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJS) $(TEST_LIBS)
	
test: $(TEST_EXEC)
	-echo Test making ...

clean:
	-rm -f $(EXEC) *.gdb *.elf *.o $(OBJS) core $(TEST_EXEC)
	-rm -f make_*.log *.map *.so *.api map_file
	
