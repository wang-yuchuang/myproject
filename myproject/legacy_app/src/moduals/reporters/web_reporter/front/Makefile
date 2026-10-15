
include $(NCCU_SOURCE_DIR)/include.mk

#The directories will NOT be maked
all test:
	echo Make all for web_front, nothing.

install:
	-mkdir -p $(NCCU_TARGET_DIR)/web/www
	-cp -rf ./www/normal $(NCCU_TARGET_DIR)/web/www

#	-mkdir -p $(NCCU_TARGET_DIR)/web/www
#	-cp -rf ./www/* $(NCCU_TARGET_DIR)/web/www
#	-cp -rf ./www/dep-frontend.zip $(NCCU_TARGET_DIR)/web/www

clean:
	-rm -f make_*.txt *.map
