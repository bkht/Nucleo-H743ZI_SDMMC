#ifndef __DMC_NET_H
#define __DMC_NET_H
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "main.h"
#include <stdint.h>
#include <string.h>

/*************** ETHERNET ***************/

/*
 *      IEEE 802.3 Ethernet magic constants.  The frame sizes omit the preamble
 *      and FCS/CRC (frame check sequence).
 */

#define ETH_IPLEN       4               /* Bytes in one IP addr   */
#define ETH_ALEN        6               /* Octets in one ethernet addr   */
#define ETH_HLEN        14              /* Total octets in header.       */
#define ETH_ZLEN        60              /* Min. octets in frame sans FCS */
#define ETH_DATA_LEN    1500            /* Max. octets in payload        */
#define ETH_FRAME_LEN   1514            /* Max. octets in frame sans FCS */
#define ETH_FCS_LEN     4               /* Octets in the FCS             */

/*
 *      These are the defined Ethernet Protocol ID's.
 */
#define ETH_P_IP        0x0800          /* Internet Protocol packet     */
#define ETH_P_IPV6      0x86DD          /* IPv6 over bluebook           */
#define ETH_P_ARP       0x0806          /* Address Resolution packet    */
#define ETH_P_8021Q     0x8100          /* 802.1Q VLAN Extended Header  */
#define ETH_P_PPP_DISC  0x8863          /* PPPoE discovery messages     */
#define ETH_P_PPP_SES   0x8864          /* PPPoE session messages       */

/*
 *      Non DIX types. Won't clash for 1500 types.
 */
#define ETH_P_802_3     0x0001          /* Dummy type for 802.3 frames  */

// Ethernet header 14 bytes
struct ethhdr {
  unsigned char   h_dest[ETH_ALEN];       /* destination eth addr */
  unsigned char   h_source[ETH_ALEN];     /* source ether addr    */
  uint16_t        h_proto;                /* packet type ID field */
} __attribute__((packed));

/*************** ARP ***************/

#define ARP_HLEN        28              /* Total octets in header.       */

#define ARPHRD_ETHER    1               /* Ethernet 10Mbps              */

/* ARP protocol opcodes. */
#define ARPOP_REQUEST   1               /* ARP request                  */
#define ARPOP_REPLY     2               /* ARP reply                    */
#define ARPOP_RREQUEST  3               /* RARP request                 */
#define ARPOP_RREPLY    4               /* RARP reply                   */
#define ARPOP_InREQUEST 8               /* InARP request                */
#define ARPOP_InREPLY   9               /* InARP reply                  */
#define ARPOP_NAK       10              /* (ATM)ARP NAK                 */

// ARP packet 28 bytes
struct arphdr
{
  uint16_t          ar_hrd;         /* format of hardware address   */
  uint16_t          ar_pro;         /* format of protocol address   */
  uint8_t           ar_hln;         /* length of hardware address   */
  uint8_t           ar_pln;         /* length of protocol address   */
  uint16_t          ar_op;          /* ARP opcode (command)         */
  uint8_t           ar_sha[ETH_ALEN];       /* sender hardware address      */
  uint8_t           ar_sip[ETH_IPLEN];      /* sender IP address            */
  uint8_t           ar_tha[ETH_ALEN];       /* target hardware address      */
  uint8_t           ar_tip[ETH_IPLEN];      /* target IP address            */
};

/*************** IP ***************/

#define IPVERSION       4
#define MAXTTL          255
#define IPDEFTTL        128  // 64
#define IP_HLEN         20

// IP header 28 bytes
struct iphdr {
  uint8_t   ihl:4,
            version:4;
  uint8_t   tos;
  uint16_t  tot_len;
  uint16_t  id;
  uint16_t  frag_off;
  uint8_t   ttl;
  uint8_t   protocol;
  uint16_t  check;
  uint32_t  saddr;
  uint32_t  daddr;
  /*The options start here. */
};

#define IPPROTO_IP   0
#define IPPROTO_ICMP 1
#define IPPROTO_TCP  6
#define IPPROTO_UDP  17


/*************** IPv6 ***************/

struct in6_addr {
	union {
		uint8_t         u6_addr8[16];
		uint16_t        u6_addr16[8];
		uint32_t        u6_addr32[4];
	} in6_u;
#define s6_addr                 in6_u.u6_addr8
#define s6_addr16               in6_u.u6_addr16
#define s6_addr32               in6_u.u6_addr32
};

#define IPV6VERSION     6
#define IPV6_HOPLIMIT   64
#define IPV6_HLEN       40

struct ipv6hdr {
	uint8_t                 priority:4,
				                  version:4;
	uint8_t                 flow_lbl[3];
	uint16_t                payload_len;
	uint8_t                 nexthdr;
	uint8_t                 hop_limit;
	struct  in6_addr        saddr;
	struct  in6_addr        daddr;
};

/*************** ICMP ***************/

#define ICMP_ECHOREPLY          0       /* Echo Reply                   */
#define ICMP_DEST_UNREACH       3       /* Destination Unreachable      */
#define ICMP_SOURCE_QUENCH      4       /* Source Quench                */
#define ICMP_REDIRECT           5       /* Redirect (change route)      */
#define ICMP_ECHO               8       /* Echo Request                 */
#define ICMP_TIME_EXCEEDED      11      /* Time Exceeded                */
#define ICMP_PARAMETERPROB      12      /* Parameter Problem            */
#define ICMP_TIMESTAMP          13      /* Timestamp Request            */
#define ICMP_TIMESTAMPREPLY     14      /* Timestamp Reply              */
#define ICMP_INFO_REQUEST       15      /* Information Request          */
#define ICMP_INFO_REPLY         16      /* Information Reply            */
#define ICMP_ADDRESS            17      /* Address Mask Request         */
#define ICMP_ADDRESSREPLY       18      /* Address Mask Reply           */
#define NR_ICMP_TYPES           18

// ICMP packet 8 bytes + len echo
struct icmphdr {
  uint8_t          type;
  uint8_t          code;
  uint16_t       checksum;
  union {
    struct {
      uint16_t  id;
      uint16_t  sequence;
    } echo;
    uint32_t  gateway;
    struct {
      uint16_t  __unused;
      uint16_t  mtu;
    } frag;
  } un;
};

/*************** UDP ***************/

#define UDP_HLEN 8

struct udphdr {
  uint16_t  source;
  uint16_t  dest;
  uint16_t  len;
  uint16_t  check;
};

void dmc_checksum_init(uint8_t type);
void dmc_checksum_step(uint8_t *buf, uint16_t len);
uint16_t dmc_checksum_end(void);
uint16_t dmc_checksum(uint8_t *buf, uint16_t len, uint8_t type);
uint16_t dmc_be16(uint16_t x);
uint32_t dmc_be32(uint32_t x);
void dmc_build_ethernet(struct ethhdr * hdr, uint8_t * dst, uint8_t * src, uint16_t h_proto);
void dmc_reply_ethernet(struct ethhdr * hdr, uint8_t * src);
void dmc_swap_ethernet(struct ethhdr * hdr);
void dmc_get_ethernet(uint8_t * buf, uint8_t * dstmac, uint8_t * srcmac, uint16_t * proto);
uint8_t dmc_ethernet_checkdest(struct ethhdr * hdr, uint8_t * mac);
uint16_t dmc_get_ethertype(struct ethhdr * hdr);
void dmc_process_arp(uint8_t * buf, uint8_t * mac, uint8_t * ip);
void dmc_build_ip(struct iphdr * hdr, uint8_t * dst, uint8_t * src,uint8_t protocol, uint16_t len);
void dmc_build_ipv6(struct ipv6hdr *hdr, struct in6_addr *dst, struct in6_addr *src,
    uint8_t protocol, uint16_t len);
uint8_t dmc_get_ipproto(struct iphdr * hdr, uint8_t * ip);
void dmc_swap_ip(struct iphdr * hdr);
void dmc_process_icmp(uint8_t * buf);
void dmc_build_udp(struct udphdr * hdr, uint16_t dst, uint16_t src, uint16_t len);
void dmc_build_udp_checksum_v6(struct ipv6hdr *ip6, struct udphdr *udp, uint16_t len);
void dmc_swap_udp(struct udphdr * hdr);

#ifdef __cplusplus
}
#endif
#endif /* __DMC_NET_H */
