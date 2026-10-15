
include $(NCCU_SOURCE_DIR)/include.mk

#The directories will NOT be maked
all test:
	echo Make all for web_server, nothing.

install:
	-dos2unix start_web_server
#	-dos2unix start_web_server_pcs
	-mkdir -p $(NCCU_TARGET_DIR)/web/server
	-cp -f server_main.conf $(NCCU_TARGET_DIR)/web/server
	-cp -f start_web_server $(NCCU_TARGET_DIR)/web/server
#	-cp -f start_web_server_pcs $(NCCU_TARGET_DIR)/web/server
	-cp -f server.pem $(NCCU_TARGET_DIR)/web/server
	-cp -rf ./lib $(NCCU_TARGET_DIR)/web/server
	-cp -rf ./bin $(NCCU_TARGET_DIR)/web/server
	-chmod +x $(NCCU_TARGET_DIR)/web/server/start_web_server
#	-chmod +x $(NCCU_TARGET_DIR)/web/server/start_web_server_pcs
	-chmod +x $(NCCU_TARGET_DIR)/web/server/bin/* -R
#########use the following cmd to generate ssl key for server
####openssl req -new -x509 -keyout server.pem -out server.pem -days 36500 -nodes 

clean:
	-rm -f make_*.txt *.map
