
include $(NCCU_SOURCE_DIR)/include.mk

#the target output file
EXEC = reporter_mqtt.so 

#the head files the object files depend on
INCS = GH_DigitalEnergy.h es_mqtt.h ../../cJSON.h 

#the object files to generate the target
OBJS = es_mqtt_main_old4async.o ../../cJSON.o es_mqtt_msghandler.o es_mqtt_msgparser.o ../reporters_pubfun.o GH_DigitalEnergy_Pub.o

#for self test
TEST_EXEC = 
TEST_OBJS =
TEST_LIBS = $(EXEC) -lpthread #and other required libs

#private shared libraries
#LDFLAGS += -L$(NCCU_SOURCE_DIR)/data_interfaces
#LIBS = -lpub -lflash_dxi -lrunb_dxi

#private shared libraries
#LIBS = -dl -lpub -L$(NCCU_SOURCE_DIR)/include/3rd_party/libpahomqtt -lpaho-mqtt3cs -lpaho-mqtt3c
LIBS = -dl -lpub -L$(NCCU_SOURCE_DIR)/include/3rd_party/libpahomqtt -lpaho-mqtt3as -lpaho-mqtt3a

all: $(EXEC)

$(EXEC): $(OBJS) $(INCS) $(PUBINCS)
	$(CC) -shared -fPIC $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	
install: $(EXEC)
	-cp $(EXEC) $(NCCU_TARGET_DIR)/reporter/
	
$(TEST_EXEC): $(EXEC) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJS) $(TEST_LIBS)
	
test: $(TEST_EXEC)
	-echo Test making ...

clean:
	-rm -f $(EXEC) *.gdb *.elf *.o ../*.o $(OBJS) core $(TEST_EXEC)
	-rm -f make_*.log *.map *.so
	
