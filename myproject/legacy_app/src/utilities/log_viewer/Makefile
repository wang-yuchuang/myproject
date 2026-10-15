
include $(NCCU_SOURCE_DIR)/include.mk

#the target output file
EXEC = log_view

#the head files the object files depend on
INCS = 

#the object files to generate the target
OBJS = log_read.o 

all: $(EXEC)
	
$(EXEC): $(OBJS) $(INCS) $(PUBINCS)
	$(CC)  $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) $(PUBLIBS)

install: $(EXEC) 
	-mkdir -p $(NCCU_TARGET_DIR)/bin/
	cp -fr $(EXEC) $(NCCU_TARGET_DIR)/bin/$(EXEC)
	chmod +x $(NCCU_TARGET_DIR)/bin/$(EXEC)

	
test: $(EXEC)
	-echo No Test to be made, skip ...

clean:
	-rm -f $(EXEC) *.gdb *.elf *.o $(OBJS) core $(TEST_EXEC)
	-rm -f make_*.log *.map	


