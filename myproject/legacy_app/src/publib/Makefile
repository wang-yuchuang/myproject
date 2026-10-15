include $(NCCU_SOURCE_DIR)/include.mk

#the target output file
EXEC = libpub.so

#the head files the object files depend on
INCS = $(NCCU_SOURCE_DIR)/include/base_defines/basefunc.h
INCS += $(NCCU_SOURCE_DIR)/include/base_defines/iLog.h
INCS += $(NCCU_SOURCE_DIR)/include/base_defines/iMutex.h
INCS += $(NCCU_SOURCE_DIR)/include/base_defines/iNew.h
INCS += $(NCCU_SOURCE_DIR)/include/base_defines/iQueue.h
INCS += $(NCCU_SOURCE_DIR)/include/base_defines/iThread.h
INCS += $(NCCU_SOURCE_DIR)/include/base_defines/iLog4c.h

#the object files to generate the target
OBJS = iBasic_Funs.o iLog.o iMutex.o iNew.o iQueue.o iThread.o iLog4c.o iDataBase.o

#for self test
TEST_EXEC = 

#private shared libraries
#LDFLAGS += -I$(NCCU_SOURCE_DIR)/include/3rd_party/openssl
#LIBS = -ldl #-lssl -lcrypto
# LIBS += $(NCCU_SOURCE_DIR)/include/3rd_party/openssl/libssl.so.1.1
# LIBS += $(NCCU_SOURCE_DIR)/include/3rd_party/openssl/libcrypto.so.1.1

all: $(EXEC)

$(EXEC): $(OBJS) $(INCS) $(PUBINCS)
	$(CC) -shared -fPIC $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	#ln -fs `pwd`/$(EXEC) $(NCCU_SOURCE_DIR)/lib/$(EXEC)
	
install: $(EXEC)
	-mkdir -p $(NCCU_TARGET_DIR)/lib/
	cp -fr $(EXEC) $(NCCU_TARGET_DIR)/lib/$(EXEC)
	chmod +x $(NCCU_TARGET_DIR)/lib/$(EXEC)	
		
$(TEST_EXEC): $(EXEC) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJS) $(TEST_LIBS)
	
#test: $(TEST_EXEC)
	-echo Test making ...

clean:
	-rm -f $(EXEC) *.gdb *.elf *.o $(OBJS) core $(TEST_EXEC)
	-rm -f make_*.log *.map *.so
	-rm -f $(NCCU_SOURCE_DIR)/publib/$(EXEC)
	


