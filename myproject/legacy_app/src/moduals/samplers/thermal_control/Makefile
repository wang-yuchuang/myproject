include $(NCCU_SOURCE_DIR)/include.mk

#==========================================
# File: thermal_control/Makefile
# Founder: NJX01203
# Date: 2026.07.11
# Function: build sampler_thermal.so (BMS thermal management strategy)
#==========================================

#the target output file
EXEC = sampler_thermal.so

#the head files the object files depend on
INCS = thermal_control_main.h

#the object files to generate the target
OBJS = thermal_control_main.o ../sampler_pubfunc.o

#for self test
TEST_EXEC =
TEST_OBJS =
TEST_LIBS =


all: $(EXEC)

$(EXEC): $(OBJS) $(INCS) $(PUBINCS)
	$(CC) -shared -fPIC $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

install: $(EXEC)
	-cp $(EXEC) $(NCCU_TARGET_DIR)/sampler/
	$(STRIP) -s -o $(NCCU_TARGET_DIR)/sampler/$(EXEC) $(EXEC)
	chmod +x $(NCCU_TARGET_DIR)/sampler/$(EXEC)

$(TEST_EXEC): $(EXEC) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJS) $(TEST_LIBS)

test: $(TEST_EXEC)
	-echo Test making ...

clean:
	-rm -f $(EXEC) $(OBJS) $(TEST_EXEC) $(TEST_OBJS) *.o *.bak *.map
	-rm -rf *.map
