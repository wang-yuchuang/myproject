#include <types.h>
#include <netdev.h>
#include <lwip/debug.h>
#include <lwip/def.h>
#include <lwip/tcpip.h>
#include <lwip/etharp.h>

static err_t netdev_linkoutput_fn(struct netif *netif, struct pbuf *p)
{
  struct net_driver_s *dev;

  dev = (struct net_driver_s *)netif->state;

  return dev->d_txavail ? dev->d_txavail(dev, p) : ERR_IF;
}

static err_t netdev_init_fn(struct netif *netif)
{
  struct net_driver_s *dev;

  netif->output = etharp_output;
  netif->linkoutput = netdev_linkoutput_fn;

  dev = (struct net_driver_s *)netif->state;

  if (dev->d_getmac) {
    netif->hwaddr_len = (u8_t)(dev->d_getmac(dev, netif->hwaddr, sizeof(netif->hwaddr)));
  }

  if (dev->d_getmtu) {
    netif->mtu = (u16_t)(dev->d_getmtu(dev));
  }

  if (dev->d_lltype == NET_LL_ETHERNET) {
    netif->name[0] = 'e';
    netif->name[1] = 'n';
    netif->num = 0;
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
  }

  if (dev->d_init) {
    dev->d_init(dev);
  }

  return ERR_OK;
}

err_t netdev_register(struct net_driver_s *dev, enum net_lltype_e lltype)
{
  LWIP_ASSERT("netdev_register: dev is NULL", dev != NULL);

  dev->d_lltype = (u8_t)lltype;

  netif_add(&dev->d_netif,
            ip_2_ip4(&dev->d_ipaddr),
            ip_2_ip4(&dev->d_netmask),
            ip_2_ip4(&dev->d_gateway),
            dev, netdev_init_fn, tcpip_input);

  netif_set_default(&dev->d_netif);
  netif_set_up(&dev->d_netif);

  return ERR_OK;
}
