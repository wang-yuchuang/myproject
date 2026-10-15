
include $(NCCU_SOURCE_DIR)/include.mk
EXTENSION_FILES = install.sh
#for USB_WAN bug fix 201912
EXTENSION_FILES += option.ko dido.ko
#for support CAP_LCD
EXTENSION_FILES += eGTouchD eGTouchL.ini setqtenv.sh system_init.sh touchEvent.sh
EXTENSION_FILES += flash_erase
EXTENSION_FILES += zImage*
#EXTENSION_FILES += dtb_*
#added by Jimmy 20201106 for suportint FTPS(SSL curl)
EXTENSION_FILES += libcurl.so 
EXTENSION_FILES += hid-multitouch.ko libnopoll.so
# libnopoll.so 的 SONAME 是 libnopoll.so.0，部署时创建版本链接
EXTENSION_FILES += libpaho-mqtt3a.so.1 libpaho-mqtt3as.so.1 libpaho-mqtt3c.so.1 libpaho-mqtt3cs.so.1
#added by Jimmy 20210423 for new kernel4
EXTENSION_FILES_NEW = new_kernel4/disable_wd 
EXTENSION_FILES_NEW += new_kernel4/*.sh
EXTENSION_FILES_NEW += new_kernel4/zImage*
EXTENSION_FILES_NEW += new_kernel4/*.ko
EXTENSION_FILES_NEW += new_kernel4/*.dtb
#added by Jimmy 20240529 for Uboot/MLO upgrage
EXTENSION_FILES_NEW += new_kernel4/u-boot.img
EXTENSION_FILES_NEW += new_kernel4/MLO
#added by Jimmy 20210517 for SSH port reallocating
EXTENSION_FILES += sshd_config newpsd
#add by lpl 250218 for log4c
EXTENSION_FILES += log4crc
EXTENSION_FILES += liblog4c.so
EXTENSION_FILES += liblog4c.so.3
EXTENSION_FILES += liblog4c.so.3.3.1
#add by lpl 250314 for keepalived
EXTENSION_FILES += keepalived
#EXTENSION_FILES += keepalived.conf
EXTENSION_FILES += notify.sh
all test:
	echo Make all for extensions, nothing.
	
install:
	-mkdir -p $(NCCU_TARGET_DIR)/extensions/
	-for x in $(EXTENSION_FILES); do \
			chmod 755 $$x; \
			cp -f $$x $(NCCU_TARGET_DIR)/extensions/; \
	done;
	# 创建 libnopoll.so.0 -> libnopoll.so 的链接（SONAME 兼容）
	-ln -sf libnopoll.so $(NCCU_TARGET_DIR)/extensions/libnopoll.so.0
	-dos2unix $(NCCU_TARGET_DIR)/extensions/install.sh
	-dos2unix $(NCCU_TARGET_DIR)/extensions/setqtenv.sh
	-dos2unix $(NCCU_TARGET_DIR)/extensions/system_init.sh
	-dos2unix $(NCCU_TARGET_DIR)/extensions/touchEvent.sh
	-dos2unix $(NCCU_TARGET_DIR)/extensions/newpsd
	-dos2unix $(NCCU_TARGET_DIR)/extensions/sshd_config
	-dos2unix $(NCCU_TARGET_DIR)/extensions/log4crc
#	-dos2unix $(NCCU_TARGET_DIR)/extensions/keepalived.conf
	-dos2unix $(NCCU_TARGET_DIR)/extensions/notify.sh
	-mkdir -p $(NCCU_TARGET_DIR)/extensions/new_kernel4
	-for x in $(EXTENSION_FILES_NEW); do \
			chmod 755 $$x; \
			cp -f $$x $(NCCU_TARGET_DIR)/extensions/new_kernel4/; \
	done;
	-dos2unix $(NCCU_TARGET_DIR)/extensions/new_kernel4/install.sh
	-dos2unix $(NCCU_TARGET_DIR)/extensions/new_kernel4/install_immu2_driver.sh
clean:
	-rm -f make_*.txt *.map
