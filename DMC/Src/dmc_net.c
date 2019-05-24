#include "dmc_net.h"

// https://github.com/fabiobaltieri/avr-nrf/blob/master/firmware/net/net.c

/* common functions */

static uint32_t partial_sum;
void dmc_checksum_init(uint8_t type)
{
  partial_sum = type;
}

void dmc_checksum_step(uint8_t *buf, uint16_t len)
{
  /* build the sum of 16bit words */
  while (len > 1) {
    partial_sum += 0xFFFF & (*buf << 8 | *(buf + 1));
    buf += 2;
    len -= 2;
  }
}

uint16_t dmc_checksum_end(void)
{
  /* now calculate the sum over the bytes in the sum */
  /* until the result is only 16bit long */
  while (partial_sum >> 16) {
    partial_sum = (partial_sum & 0xFFFF) + (partial_sum >> 16);
  }
  /* build 1's complement: */
  return ((uint16_t) partial_sum ^ 0xFFFF);
}

uint16_t dmc_checksum(uint8_t *buf, uint16_t len, uint8_t type)
{
  uint32_t sum = type;

  /* build the sum of 16bit words */
  while (len > 1){
    sum += 0xFFFF & (*buf << 8 | *(buf + 1));
    buf += 2;
    len -= 2;
  }
  /* if there is a byte left then add it (padded with zero) */
  if (len) {
    sum += (0xFF & *buf) << 8;
  }
  /* now calculate the sum over the bytes in the sum */
  /* until the result is only 16bit long */
  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }
  /* build 1's complement: */
  return ((uint16_t) sum ^ 0xFFFF);
}

uint16_t dmc_be16(uint16_t x)
{
  return (x >> 8) | ((x & 0xff) << 8);
}

uint32_t dmc_be32(uint32_t x)
{
  return (
      ((x & 0xff000000) >> 24) |
      ((x & 0x00ff0000) >> 8)  |
      ((x & 0x0000ff00) << 8)  |
      ((x & 0x000000ff) << 24) );
}

/* Ehernet layer */

void dmc_build_ethernet(struct ethhdr * hdr, uint8_t * dst, uint8_t * src, uint16_t h_proto)
{
  if (dst == NULL)
  {
    memset(&hdr->h_dest, 0xff, ETH_ALEN);
  }
  else
  {
    memcpy(&hdr->h_dest, dst, ETH_ALEN);
  }

  memcpy(&hdr->h_source, src, ETH_ALEN);

  hdr->h_proto = be16(h_proto);
}

void dmc_reply_ethernet(struct ethhdr * hdr, uint8_t * src)
{
  memcpy(&hdr->h_dest, &hdr->h_source, ETH_ALEN);
  memcpy(&hdr->h_source, src, ETH_ALEN);
}

void dmc_swap_ethernet(struct ethhdr * hdr)
{
  uint8_t tmp[ETH_ALEN];
  memcpy(tmp, &hdr->h_source, ETH_ALEN);
  memcpy(&hdr->h_source, &hdr->h_dest, ETH_ALEN);
  memcpy(&hdr->h_dest, tmp, ETH_ALEN);
}

void dmc_get_ethernet(uint8_t * buf, uint8_t * dstmac, uint8_t * srcmac, uint16_t * proto)
{
  struct ethhdr * eth;

  eth = (struct ethhdr *)&buf[0];

  memcpy(dstmac, eth->h_dest, ETH_ALEN);
  memcpy(srcmac, eth->h_source, ETH_ALEN);
  *proto = be16(eth->h_proto);
}

// Return 1, if matched
uint8_t dmc_ethernet_checkdest(struct ethhdr * hdr, uint8_t * mac)
{
  if (memcmp(&hdr->h_dest, mac, ETH_ALEN) == 0)
  {
    return 1;
  }
  return 0;
}

uint16_t dmc_get_ethertype(struct ethhdr * hdr)
{
  return hdr->h_proto;
}

/* ARP layer */

void dmc_process_arp(uint8_t * buf, uint8_t * mac, uint8_t * ip)
{
  struct arphdr * arp;

  arp = (struct arphdr *)&buf[ETH_HLEN];

  /* check arp request is correct */

  if (arp->ar_hrd != be16(ARPHRD_ETHER))
    return;

  if (arp->ar_pro != be16(ETH_P_IP))
    return;

  if (arp->ar_hln != ETH_ALEN)
    return;

  if (arp->ar_pln != ETH_IPLEN)
    return;

  if (arp->ar_op != be16(ARPOP_REQUEST))
    return;

  if (memcmp(arp->ar_tip, ip, 4) != 0)
    return;

  /* send a reply */

  reply_ethernet((struct ethhdr *)&buf[0], mac);

  arp->ar_op = be16(ARPOP_REPLY);

  memcpy(&arp->ar_tip, &arp->ar_sip, 4);
  memcpy(&arp->ar_tha, &arp->ar_sha, ETH_ALEN);

  memcpy(&arp->ar_sip, ip, 4);
  memcpy(&arp->ar_sha, mac, ETH_ALEN);

  ksz8851_send_packet(buf, ETH_HLEN + ARP_HLEN);
}

/* IP layer */

void dmc_build_ip(struct iphdr * hdr, uint8_t * dst, uint8_t * src, uint8_t protocol, uint16_t len)
{
  hdr->version = IPVERSION;
  hdr->ihl = IP_HLEN/4;
  hdr->frag_off = 0x40;
  hdr->ttl = IPDEFTTL;
  hdr->check = 0;
  hdr->tot_len = be16(IP_HLEN + len);
  hdr->protocol = protocol;

  if (dst == NULL)
    memset(&hdr->daddr, 0xff, 4);
  else
    memcpy(&hdr->daddr, dst, 4);

  memcpy(&hdr->saddr, src, 4);
  hdr->check = be16(checksum((uint8_t *)hdr, IP_HLEN, 0));
}

void dmc_build_ipv6(struct ipv6hdr *hdr, struct in6_addr *dst, struct in6_addr *src,
    uint8_t protocol, uint16_t len)
{
  hdr->version = IPV6VERSION;

  hdr->payload_len = be16(len);
  hdr->nexthdr = protocol;
  hdr->hop_limit = IPV6_HOPLIMIT;

  memcpy(&hdr->daddr, dst, sizeof(struct in6_addr));
  memcpy(&hdr->saddr, src, sizeof(struct in6_addr));
}

uint8_t dmc_get_ipproto(struct iphdr * hdr, uint8_t * ip)
{
  if (hdr->version != IPVERSION)
    return -1;

  if (hdr->ihl != IP_HLEN/4)
    return -1;

  if (checksum((uint8_t *)hdr, IP_HLEN, 0) != 0)
    return -1;

  if (memcmp(&hdr->daddr, ip, 4) != 0)
    return -1;

  return hdr->protocol;
}

void dmc_swap_ip(struct iphdr * hdr)
{
  uint8_t tmp[4];

  memcpy(tmp, &hdr->saddr, 4);
  memcpy(&hdr->saddr, &hdr->daddr, 4);
  memcpy(&hdr->daddr, tmp, 4);
}

/* ICMP layer */

void dmc_process_icmp(uint8_t * buf)
{
  struct ethhdr * eth;
  struct iphdr * ip;
  struct icmphdr * icmp;

  eth = (struct ethhdr *)&buf[0];
  ip = (struct iphdr *)&buf[ETH_HLEN];
  icmp = (struct icmphdr *)&buf[ETH_HLEN + IP_HLEN];

  if (icmp->type != ICMP_ECHO)
    return;

  swap_ethernet(eth);
  swap_ip(ip);

  ip->ttl = IPDEFTTL;

  ip->check = 0;
  ip->check = be16(checksum((uint8_t *)ip, IP_HLEN, 0));

  icmp->type = ICMP_ECHOREPLY;

  icmp->checksum = 0;
  icmp->checksum = be16(checksum((uint8_t *)icmp,
               be16(ip->tot_len) - IP_HLEN, 0));

  ksz8851_send_packet(buf, ETH_HLEN + be16(ip->tot_len));
}

/* UDP layer */

void dmc_build_udp(struct udphdr * hdr, uint16_t dst, uint16_t src, uint16_t len)
{
  hdr->source = be16(src);
  hdr->dest = be16(dst);
  hdr->len = be16(UDP_HLEN + len);
  hdr->check = 0;
}

void dmc_build_udp_checksum_v6(struct ipv6hdr *ip6, struct udphdr *udp, uint16_t len)
{
  uint16_t zero;

  checksum_init(0);
  checksum_step((uint8_t *)&ip6->saddr, sizeof(ip6->saddr));
  checksum_step((uint8_t *)&ip6->daddr, sizeof(ip6->daddr));
  zero = be16(UDP_HLEN + len);
  checksum_step((uint8_t *)&zero, sizeof(zero));
  zero = be16(IPPROTO_UDP);
  checksum_step((uint8_t *)&zero, sizeof(zero));
  checksum_step((uint8_t *)udp, UDP_HLEN + len);

  udp->check = be16(checksum_end());
}

void dmc_swap_udp(struct udphdr * hdr)
{
  uint8_t tmp[2];

  memcpy(tmp, &hdr->source, 2);
  memcpy(&hdr->source, &hdr->dest, 2);
  memcpy(&hdr->dest, tmp, 2);
}
