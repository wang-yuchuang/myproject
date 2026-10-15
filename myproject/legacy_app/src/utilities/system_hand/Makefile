include $(NCCU_SOURCE_DIR)/include.mk

#the target output file
EXEC = syshand

#the head files the object files depend on
INCS := smain.h handler.h

#the object files to generate the target
OBJS = smain.o handler.o 

TEST_LIBS = #$(EXEC1) -ldl 

#private shared libraries
#LIBS =  -lm -lpthread -lpub  #and other required libs

#CFLAGS = 
#LDFLAGS = -lpthread

all: $(EXEC)

# $(EXEC): $(OBJS) $(INCS)
# 	$(CC) $(CFLAGS) $(LDFLAGS) -o $@  $(OBJS)
	#ln -fs `pwd`/$(EXEC1) $(NCCU_SOURCE_DIR)/bin/$(EXEC1)
$(EXEC): $(OBJS) $(INCS) $(PUBINCS)
	$(CC)  $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) $(PUBLIBS)	

install: $(EXEC)	
	-mkdir -p $(NCCU_TARGET_DIR)/bin/
	cp -fr $(EXEC) $(NCCU_TARGET_DIR)/bin/$(EXEC)
	chmod +x $(NCCU_TARGET_DIR)/bin/$(EXEC)


$(TEST_EXEC): $(EXEC) $(TEST_OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJS) $(TEST_LIBS) 
			
	
test: $(TEST_EXEC)
	-echo Test making ...

clean:
	-rm -f $(EXEC) *.gdb *.elf *.o $(OBJS) core $(TEST_EXEC)
	-rm -f make_*.txt *.map
	#-rm -f $(NCCU_SOURCE_DIR)/system_hand/$(EXEC)
