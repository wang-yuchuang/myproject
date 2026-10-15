#ifndef NET_DEV_H_
#define NET_DEV_H_

#include <lwip/opt.h>
#include <lwip/netif.h>
#include <lwip/pbuf.h>
#include <lwip/err.h>

struct net_driver_s;

/* Link layer type.  This type is used with netdev_register in order to
 * identify the type of the network driver.
 */

enum net_lltype_e {
  NET_LL_ETHERNET = 0, /* Ethernet */
  NET_LL_LOOPBACK,     /* Local loopback */
  NET_LL_SLIP,         /* Serial Line Internet Protocol (SLIP) */
  NET_LL_TUN,          /* TUN Virtual Network Device */
  NET_LL_BLUETOOTH,    /* Bluetooth */
  NET_LL_IEEE80211,    /* IEEE 802.11 */
  NET_LL_IEEE802154,   /* IEEE 802.15.4 MAC */
  NET_LL_PKTRADIO      /* Non-standard packet radio */
};

struct net_driver_s {
  struct netif d_netif;
  u8_t d_lltype; /* See enum net_lltype_e */

  /** IP address configuration in network byte order */
  ip_addr_t d_ipaddr;
  ip_addr_t d_netmask;
  ip_addr_t d_gateway;

  err_t (*d_init)(struct net_driver_s *dev);
  err_t (*d_txavail)(struct net_driver_s *dev, struct pbuf *p);
  u32_t (*d_getmac)(struct net_driver_s *dev, void *buf, u32_t max_len);
  u32_t (*d_getmtu)(struct net_driver_s *dev);

  void *d_private;
};

err_t netdev_register(struct net_driver_s *dev, enum net_lltype_e lltype);

static inline void netdev_input(struct net_driver_s *dev, struct pbuf *p) {
  if (dev->d_netif.input(p, &dev->d_netif) != ERR_OK) {
    pbuf_free(p);
  }
}

#endif /* NET_DEV_H_ */
