include $(NCCU_SOURCE_DIR)/include.mk

#==========================================
# File: auxdev_sampler/Makefile
# Founder: NJX01203
# Date: 2026.07.01
# Function: build sampler_auxdev.so (U3Z water chiller + IMLU2 light strip on shared UART5)
#==========================================

#the target output file
EXEC = sampler_auxdev.so

#the head files the object files depend on
INCS = auxdev_sampler_main.h u3z_protocol.h imlu2_protocol.h

#the object files to generate the target
OBJS = auxdev_sampler_main.o u3z_protocol.o imlu2_protocol.o ../sampler_pubfunc.o

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
	-rm -f $(EXEC) *.gdb *.elf *.o $(OBJS) core $(TEST_EXEC)
	-rm -f make_*.txt *.map *.so
