include $(NCCU_SOURCE_DIR)/include.mk

#==========================================
# File: aircon_sampler/Makefile
# Founder: NJX01203
# Date: 2026.07.02
# Function: build sampler_aircon.so (TF/EU3 liquid-cooling AC on RS485)
#==========================================

#the target output file
EXEC = sampler_aircon.so

#the head files the object files depend on
INCS = aircon_sampler_main.h aircond_regs.h

#the object files to generate the target
OBJS = aircon_sampler_main.o ../pv_inverter_sampler/pv_modbus_rtu.o ../sampler_pubfunc.o

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
