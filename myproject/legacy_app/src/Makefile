
include $(NCCU_SOURCE_DIR)/include.mk

#Jimmy note: private key and passwd is stored in Qt src file, here we use pub kye to engrypt the passwd
RSA_OUT_ROOT_KEY_FILE=$(NCCU_PKG_DIR)/root_key
#for IMSU-D bin file upgrading
#CCU_MD5SUM_PKG_SRC_FILE=$(NCCU_SOURCE_DIR)/ccu_bin/IMSU_APP.bin
#CCU_MD5SUM_PKG_LOG_FILE=$(NCCU_PKG_DIR)/ccu_pkg.sum
#added by Jimmy 20210319 for nps
NPS_CLIENT_FILE=$(NCCU_SOURCE_DIR)/nps_client/vcon_client
APP_UPDATE_FILE=$(NCCU_SOURCE_DIR)/utilities/usb_update/update_console/app_update

#The floowing directories will be made
DIRS = publib 
DIRS += utilities/log_viewer
DIRS += utilities/system_hand
#DIRS += utilities/start_fbshow/standard

#ifeq ($(NCCU_MAKE_MANIF), ST)
# DIRS += utilities/usb_update/update_ui_st
#else ifeq ($(NCCU_MAKE_MANIF), EX)
#	DIRS += utilities/usb_update/update_ui
#endif

DIRS += moduals/data_processor/dpr
DIRS += comm_ports/comm_std_serial
DIRS += comm_ports/comm_net_tcpip
DIRS += comm_ports/comm_http_ftp
DIRS += comm_ports/comm_can
DIRS += comm_ports/comm_ipc
DIRS += comm_ports/comm_websocket
DIRS += comm_ports/comm_websocket_ex
DIRS += comm_ports/comm_net_udp
DIRS += comm_ports/comm_rpmsg
#samplers
DIRS += moduals/samplers/kwhmeter_sampler
DIRS += moduals/samplers/dev_485_2_sampler
DIRS += moduals/samplers/system_sampler
#DIRS += moduals/samplers/cardreader_sampler
DIRS += moduals/samplers/ccu_sampler
#DIRS += moduals/samplers/inter_slave_sampler
DIRS += moduals/samplers/st_batt_sampler
DIRS += moduals/samplers/udp_inter_sampler
DIRS += moduals/samplers/uart_server_sampler
DIRS += moduals/samplers/aircon_sampler
DIRS += moduals/samplers/auxdev_sampler
DIRS += moduals/samplers/thermal_control
####basic libs
DIRS += shells

# QT_UPDT_DIR = utilities/usb_update/update_qt_console
# 	DIRS += $(QT_UPDT_DIR)
####extensions
DIRS += extensions

#DIRS += moduals/reporters/inter_master_reporter
# DIRS += moduals/reporters/iec_61850_reporter
# DIRS += moduals/reporters/iec_104_reporter
# DIRS += moduals/reporters/iec_101_reporter
DIRS += moduals/reporters/es_iocpp_reporter
DIRS += moduals/reporters/es_mqtt_reporter
DIRS += moduals/reporters/ify_cloud_reporter
DIRS += moduals/reporters/modbus_reporter
##web
DIRS += moduals/reporters/web_reporter/web_server
DIRS += moduals/reporters/web_reporter/back_api
DIRS += moduals/reporters/web_reporter/front
DIRS += moduals/reporters/web_reporter/web_api_reporter

#maintain_be(Jimmy 20220929: old-mbe is now phased-out!)
#DIRS += moduals/reporters/maintain_backend_reporter/mbe-websocket-old
DIRS += moduals/reporters/maintain_backend_reporter/mbe-mqtt
DIRS += moduals/reporters/ess_maintenance_reporter
DIRS += moduals/reporters/slaveEMS_reporter

CONFIG_FILE = $(NCCU_SOURCE_DIR)/config/System.cfg
ifeq ($(NCCU_MAKE_MANIF), EX)
	CONFIG_FILE = $(NCCU_SOURCE_DIR)/config/System_EX.cfg
endif

###main
DIRS += main

all test:
	#####Jimmy note: mkdir here so that some early make can be done in the folder
	-mkdir -p $(NCCU_TARGET_DIR)
	-rm -fr $(NCCU_TARGET_DIR)/*
	-mkdir -p $(NCCU_TARGET_DIR)/bin
# ifeq ($(MU_TYPE), mu3)
# 	-mkdir -p $(QT_UPDT_DIR)/../tmp
# else
# 	-mkdir -p $(QT_DIR)/../tmp
# endif
	for i in $(DIRS) ; do make -C $$i $@ || exit $?; done
	
install:
	#erase all file at target dir.
	####Jimmy note: mkdir should be put into making phase
	#-mkdir -p $(NCCU_TARGET_DIR)
	#-rm -fr $(NCCU_TARGET_DIR)/*
	#-mkdir -p $(NCCU_TARGET_DIR)/bin
	-mkdir -p $(NCCU_TARGET_DIR)/comm_port
	-mkdir -p $(NCCU_TARGET_DIR)/sampler
	-mkdir -p $(NCCU_TARGET_DIR)/reporter

ifeq ($(NCCU_CROSS_PLATFORM), x86)
	-mkdir -p $(NCCU_TARGET_DIR)/log/
	-touch -f $(NCCU_TARGET_DIR)/log/nccu.log
endif

#WLD01206 modify resolution Flag file
ifeq ($(LCD_TYPE), mu2_10lcd)
	-cp -f $(NCCU_SOURCE_DIR)/moduals/reporters/gui_reporter/qt_gui/ebmu_ui_10inch_lcd/mark_10inchlcd_flag.txt $(NCCU_TARGET_DIR)/
endif

	##copy config files
	-mkdir -p $(NCCU_TARGET_DIR)/config/
	-cp -fr $(CONFIG_FILE) $(NCCU_TARGET_DIR)/config/System.cfg
	##copy cafile *.pem
	-cp -fr $(NCCU_SOURCE_DIR)/config/*.pem $(NCCU_TARGET_DIR)/config/

#ifeq ($(NCCU_MAKE_TYPE), install_with_bsp)
#	-mkdir -p $(NCCU_TARGET_DIR)/bsp/
	#Jimmy added 20190223 for updating libs for sold out products
#	-cp -fr `find $(NCCU_SOURCE_DIR)/include/3rd_party/ -name *.so*` $(NCCU_TARGET_DIR)/bsp/
#	-cp -fr $(NCCU_SOURCE_DIR)/bsp/* $(NCCU_TARGET_DIR)/bsp/
	#All config files shall be writeable.
#	-chmod -R 777 $(NCCU_TARGET_DIR)/bsp/
	#Jimmy note: need to call this for dos2unix!!!!!!!!
	#-find $(NCCU_TARGET_DIR)/bsp/usb_down -type f | xargs dos2unix
#endif

#ifeq ($(NCCU_MAKE_TYPE), install_with_bsp_sh)
#	-mkdir -p $(NCCU_TARGET_DIR)/bsp/
#	-cp -fr $(NCCU_SOURCE_DIR)/bsp/usb_down $(NCCU_TARGET_DIR)/bsp/
#	-cp -fr $(NCCU_SOURCE_DIR)/bsp/driver $(NCCU_TARGET_DIR)/bsp/
#	-cp -fr $(NCCU_SOURCE_DIR)/bsp/app_update $(NCCU_TARGET_DIR)/bsp/
#	#All config files shall be writeable.
#	-chmod -R 777 $(NCCU_TARGET_DIR)/bsp/
#	#Jimmy note: need to call this for dos2unix!!!!!!!!
#	-find $(NCCU_TARGET_DIR)/bsp/usb_down -type f | xargs dos2unix
#endif

#ifeq ($(NCCU_MAKE_TYPE), install_with_bsp_font)
#	-mkdir -p $(NCCU_TARGET_DIR)/bsp/
#	-cp -fr $(NCCU_SOURCE_DIR)/bsp/fonts $(NCCU_TARGET_DIR)/bsp/fonts
#	#All config files shall be writeable.
#	-chmod -R 777 $(NCCU_TARGET_DIR)/bsp/
#endif

ifeq ($(NCCU_MAKE_TYPE), install_with_ccubin)
	-mkdir -p $(NCCU_TARGET_DIR)/ccu_bin/
	-rm -fr $(NCCU_TARGET_DIR)/ccu_bin/*
	-cp -fr $(NCCU_SOURCE_DIR)/ccu_bin/*.bin $(NCCU_TARGET_DIR)/ccu_bin/
endif

	for i in $(DIRS) ; do make -C $$i $@ || exit $?; done
	
	##addded by Jimmy 20210319 for nps client
	cp -fr $(NPS_CLIENT_FILE) $(NCCU_TARGET_DIR)/bin
	##added by zj 20250525
	cp -fr $(APP_UPDATE_FILE) $(NCCU_TARGET_DIR)/bin
	chmod +x $(NCCU_TARGET_DIR)/bin/* -R

	#-cd $(NCCU_TARGET_DIR)/; rm -rf `find -name _svn`; cd - 
	-mkdir -p $(NCCU_PKG_DIR)/
	-rm -fr $(NCCU_PKG_DIR)/*

	#make pkg
	-cd $(NCCU_TARGET_DIR)/; tar -czf $(NCCU_PKG_DIR)/app_nccu.tar.gz *; cd -
	-cd $(NCCU_TARGET_DIR)/; cd -

#	-rm -f $(MD5SUM_PKG_LOG_FILE)

# LJH241101 MU3
#ifeq ($(MU_TYPE), mu3)
#	$(NCCU_SOURCE_DIR)/shells/make_md5sum.mu3 $(MD5SUM_PKG_SRC_FILE) $(MD5SUM_PKG_LOG_FILE)
#else
#	$(NCCU_SOURCE_DIR)/shells/make_md5sum $(MD5SUM_PKG_SRC_FILE) $(MD5SUM_PKG_LOG_FILE)
#endif

	#use OPENSSL api to encryt and generate the root_key file
ifndef NCCU_SKIP_ENCRYPT
	-rm -f $(RSA_OUT_ROOT_KEY_FILE)
	-openssl rsautl -encrypt -in $(RSA_IN_PASSWD_FILE) -inkey $(RSA_IN_PUB_KEY_FILE) -pubin -out $(RSA_OUT_ROOT_KEY_FILE) 
endif

remote_pkg:
	-mkdir -p $(NCCU_IMG_DIR)
	-rm -fr $(NCCU_IMG_DIR)/*
	-cd $(NCCU_PKG_DIR)/; tar -czf $(NCCU_IMG_DIR)/nccu_app.rpkg app_nccu.tar.gz pkg.sum; cd -

clean:
	for i in $(DIRS) ; do make -C $$i clean ; done
	-rm -f make_*.txt *.map
	-rm -fr $(NCCU_TARGET_DIR)/*
		
