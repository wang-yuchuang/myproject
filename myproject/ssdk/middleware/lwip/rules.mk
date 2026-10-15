LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)
LWIPDIR := $(LOCAL_DIR)/src
CONTRIBDIR := $(LOCAL_DIR)/contrib

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/src/include \
	$(LOCAL_DIR)/contrib/ports/cmsis_os2/include \
	$(LOCAL_DIR)/contrib

# COREFILES, CORE4FILES: The minimum set of files needed for lwIP.
COREFILES := $(LWIPDIR)/core/init.c \
	$(LWIPDIR)/core/def.c \
	$(LWIPDIR)/core/dns.c \
	$(LWIPDIR)/core/inet_chksum.c \
	$(LWIPDIR)/core/ip.c \
	$(LWIPDIR)/core/mem.c \
	$(LWIPDIR)/core/memp.c \
	$(LWIPDIR)/core/netif.c \
	$(LWIPDIR)/core/pbuf.c \
	$(LWIPDIR)/core/raw.c \
	$(LWIPDIR)/core/stats.c \
	$(LWIPDIR)/core/sys.c \
	$(LWIPDIR)/core/altcp.c \
	$(LWIPDIR)/core/altcp_alloc.c \
	$(LWIPDIR)/core/altcp_tcp.c \
	$(LWIPDIR)/core/tcp.c \
	$(LWIPDIR)/core/tcp_in.c \
	$(LWIPDIR)/core/tcp_out.c \
	$(LWIPDIR)/core/timeouts.c \
	$(LWIPDIR)/core/udp.c

CORE4FILES := $(LWIPDIR)/core/ipv4/acd.c \
	$(LWIPDIR)/core/ipv4/autoip.c \
	$(LWIPDIR)/core/ipv4/dhcp.c \
	$(LWIPDIR)/core/ipv4/etharp.c \
	$(LWIPDIR)/core/ipv4/icmp.c \
	$(LWIPDIR)/core/ipv4/igmp.c \
	$(LWIPDIR)/core/ipv4/ip4_frag.c \
	$(LWIPDIR)/core/ipv4/ip4.c \
	$(LWIPDIR)/core/ipv4/ip4_addr.c

# APIFILES: The files which implement the sequential and socket APIs.
APIFILES := $(LWIPDIR)/api/api_lib.c \
	$(LWIPDIR)/api/api_msg.c \
	$(LWIPDIR)/api/err.c \
	$(LWIPDIR)/api/if_api.c \
	$(LWIPDIR)/api/netbuf.c \
	$(LWIPDIR)/api/netdb.c \
	$(LWIPDIR)/api/netifapi.c \
	$(LWIPDIR)/api/sockets.c \
	$(LWIPDIR)/api/tcpip.c

# NETIFFILES: Files implementing various generic network interface functions
NETIFFILES := $(LWIPDIR)/netif/ethernet.c

# CONTRIBAPPFILES: Contrib Applications.
CONTRIBAPPFILES := $(CONTRIBDIR)/apps/ping/ping.c

MODULE_SRCS += $(LOCAL_DIR)/contrib/ports/cmsis_os2/sys_arch.c
MODULE_SRCS += $(LOCAL_DIR)/contrib/ports/cmsis_os2/netdev.c
MODULE_SRCS += $(LOCAL_DIR)/contrib/ports/cmsis_os2/net_tool.c
MODULE_SRCS += $(LWIPDIR)/apps/lwiperf/lwiperf.c
MODULE_SRCS += $(LWIPDIR)/apps/tftp/tftp.c

MODULE_SRCS += $(COREFILES) \
	$(CORE4FILES) \
	$(APIFILES) \
	$(NETIFFILES) \
	$(CONTRIBAPPFILES)

MODULE_COMPILEFLAGS += -Wno-unused-but-set-variable -Wno-unused-variable

include mk/module.mk
