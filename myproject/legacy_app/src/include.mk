#complile flags
CFLAGS =

#link flags
LDFLAGS =

ifeq ($(NCCU_CROSS_PLATFORM), arm)
	GCC_PREFIX = aarch64-linux-gnu-
else
	GCC_PREFIX = 
endif


CC = $(GCC_PREFIX)gcc
LD = $(GCC_PREFIX)ld
AR = $(GCC_PREFIX)ar
RANLIB=$(GCC_PREFIX)ranlib
STRIP= $(GCC_PREFIX)strip

#link flags
LDFLAGS += -lpthread -lrt -lm -L$(NCCU_SOURCE_DIR)/publib/ -L$(NCCU_SOURCE_DIR)/include/3rd_party/sqlite

ifeq ($(NCCU_MAKE_TYPE), release)
	CFLAGS  += -DNDEBUG #-o2
	LDFLAGS += -Wl,-Map,map-file -s 
else
	CFLAGS  += -g -ggdb -rdynamic -D_DEBUG=1
	LDFLAGS += -Wl,-Map,$@.map
endif

ifeq ($(NCCU_CROSS_PLATFORM), arm)
	CFLAGS += -D_HAS_FLASH_MEM=1
endif

# LJH241101 MU3
ifeq ($(MU_TYPE), mu3)
	CFLAGS += -DIMMU3
else
	CFLAGS += -DIMMU2
endif

####Jimmy note: added this to get rid of the warning of 'strptime' function
CFLAGS += -D_GNU_SOURCE

####according to libc, this will MACRO will let app to use thread-safe funtions if have
CFLAGS += -D_REENTRANT

#####Jimmy note, this -fPIC solves the compiling problem in x64 platform
CFLAGS += -fPIC 

STRIP= $(GCC_PREFIX)strip -o -s

CFLAGS +=  -W -Wall -Wstrict-prototypes -Wundef -Wunknown-pragmas -Wunreachable-code 
CFLAGS += -Wfloat-equal -W -Wimplicit -Wconversion
#CFLAGS += -Werror #treat warning as error
CFLAGS += -I$(NCCU_SOURCE_DIR)/include -I$(NCCU_SOURCE_DIR)/include/3rd_party/openssl

#public dependent head files.
PUBINCS = $(NCCU_SOURCE_DIR)/include/app_incs.h

LDFLAGS += $(NCCU_SOURCE_DIR)/include/3rd_party/openssl/libssl.so.1.1
LDFLAGS += $(NCCU_SOURCE_DIR)/include/3rd_party/openssl/libcrypto.so.1.1

CFLAGS += -I$(NCCU_SOURCE_DIR)/include/3rd_party/log4c/include
LDFLAGS += -L$(NCCU_SOURCE_DIR)/include/3rd_party/libexpat/lib -lexpat
LDFLAGS += -L$(NCCU_SOURCE_DIR)/include/3rd_party/log4c/lib -llog4c

CFLAGS+= -I$(NCCU_SOURCE_DIR)/include/3rd_party/libwebsockets
#public librarys
# PUBLIBS += $(NCCU_SOURCE_DIR)/include/3rd_party/openssl/libssl.so.1.1
# PUBLIBS += $(NCCU_SOURCE_DIR)/include/3rd_party/openssl/libcrypto.so.1.1
PUBLIBS = -lpub
PUBLIBS += -lsqlite3
PUBLIBS += $(NCCU_SOURCE_DIR)/include/3rd_party/zlib/libz.so.1.2.11
