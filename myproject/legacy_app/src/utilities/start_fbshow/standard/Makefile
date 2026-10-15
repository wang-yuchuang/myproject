include $(NCCU_SOURCE_DIR)/include.mk

#the target output file
EXEC = start_fbshow

INCS = data_def.h
#the object files to generate the target
OBJS += ui_handler.o main.o
LIBS = -L../ -lqrencode

all: $(EXEC)

$(EXEC): $(OBJS) $(INCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)


install: $(EXEC)
	-mkdir -p $(NCCU_TARGET_DIR)/startup_show/images
	-cp -fr ./images/* $(NCCU_TARGET_DIR)/startup_show/images
	-cp -fr $(EXEC) $(NCCU_TARGET_DIR)/startup_show/$(EXEC)
	-chmod +x $(NCCU_TARGET_DIR)/startup_show/$(EXEC)


clean:
	-rm -f $(EXEC) *.gdb *.elf *.o $(OBJS) core
	-rm -f make_*.txt *.map *.so

