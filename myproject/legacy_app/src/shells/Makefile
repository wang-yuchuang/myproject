
include $(NCCU_SOURCE_DIR)/include.mk

##SHELL_FILES = startrun start_ui stoprun log_cat modem_reset modem_dial run_app
SHELL_FILES =  startrun start_ui stoprun log_cat modem_reset modem_dial app_entry app_exit mount_data_binds
# LJH241101 MU3
SHELL_FILES3 = startrun stoprun modem_reset
#The directories will NOT be maked
all test:
	#echo Make all for shells, nothing.
	#Jimmy noted 20200424: if debug, should be run manually, so change the name

ifeq ($(NCCU_MAKE_TYPE), debug)
ifeq ($(MU_TYPE), mu3)
	-cp -f mu3/run_app.debug $(NCCU_TARGET_DIR)/run_app
else
	-cp -f run_app.debug $(NCCU_TARGET_DIR)/run_app
endif
else
ifeq ($(MU_TYPE), mu3)
	-cp -f mu3/run_app $(NCCU_TARGET_DIR)/run_app
else
	-cp -f run_app $(NCCU_TARGET_DIR)/run_app
endif
endif

install:
	-dos2unix make_md5sum
	-dos2unix make_md5sum.mu3
	#-mkdir -p $(NCCU_TARGET_DIR)/bin/
	-for x in $(SHELL_FILES); do \
			cp -f $$x $(NCCU_TARGET_DIR)/$$x; \
			dos2unix $(NCCU_TARGET_DIR)/$$x; \
			chmod 755 $(NCCU_TARGET_DIR)/$$x; \
	done;
	
	-dos2unix $(NCCU_TARGET_DIR)/run_app
	-chmod 755 $(NCCU_TARGET_DIR)/run_app
	#Jimmy added: just copy
	#replace it into extensions
	#-cp -f ./*.ko $(NCCU_TARGET_DIR);
	#-cp -f ./*.png $(NCCU_TARGET_DIR);
# LJH241101 MU3
ifeq ($(MU_TYPE), mu3)
	-cd mu3; \
	for x in $(SHELL_FILES3); do \
		cp -f $$x $(NCCU_TARGET_DIR)/$$x; \
		dos2unix $(NCCU_TARGET_DIR)/$$x; \
		chmod 755 $(NCCU_TARGET_DIR)/$$x; \
	done; \
	cd -;
endif

clean:
	-rm -f make_*.txt *.map
