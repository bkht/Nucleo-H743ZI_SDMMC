#include "dmc_tcpbase.h"

#define NETBUF_SZ 256
#define NET_TIMEOUT 10

#define MSG_DISCOVER	"/nrf/discover"
#define MSG_IO		"/nrf/io"
#define MSG_INIT	"/nrf/init"
#define MSG_BOUND	"/nrf/bound"

#define HZ 200

static enum {
	MODE_UNICAST = 0,
	MODE_MCAST = 1,
	MODE_MCAST6 = 2,
} net_mode = MODE_UNICAST;

static uint8_t my_mac_ee[6] = {0x00, 0x21, 0xf3, 0x00, 0x32, 0x02};
static uint8_t my_ip_ee[4] = {172, 16, 0, 33};

/* TODO: manage defaults and set MAC in phy */
static uint8_t my_mac[6] = {0x00, 0x21, 0xf3, 0x00, 0x88, 0x51};;
static uint8_t my_ip[4] = {172, 16, 0, 33};

static uint8_t remote_mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t remote_ip[4] = {0, 0, 0, 0};

static uint8_t remote_mcast_mac[6] = {0x01, 0x00, 0x5e, 0x18, 0x01, 0x01};
static uint8_t remote_mcast_ip[4] = {224, 24, 1, 1};
//#define IN6ADDR_ANY_INIT { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 } } }
//#define IN6ADDR_ME    { { { 0xfe,0x80,0,0,0,0,0,0,0x02,0x21,0xf3,0xff,0xfe,0x00,0x88,0x51 } } }
//#define IN6ADDR_MCAST { { { 0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0x24,0,0x01 } } }
//struct in6_addr my_ipv6 = IN6ADDR_ME;
//struct in6_addr remote_mcast_ipv6 = IN6ADDR_MCAST;
//static uint8_t remote_mcastv6_mac[6] = {0x33, 0x33, 0x00, 0x24, 0x00, 0x01};

static uint8_t bound = 0;
static uint8_t buf[NETBUF_SZ];

static uint16_t boundtime = 0;
static uint16_t lastshot = 0;

volatile uint16_t jiffies;

struct osc_nrf_frame {
	char fmt[4];
	uint32_t len;
	uint8_t data[PAYLOAD_SIZE];
};

struct osc_bound_frame {
	char fmt[8];
	uint32_t timeout;
	uint32_t ip;
	uint32_t mac_len;
	uint8_t mac_data[6];
	uint8_t _pad[2];
};

static void eeprom_restore (void)
{
	eeprom_read_block(my_mac, my_mac_ee, ETH_ALEN);
	eeprom_read_block(my_ip, my_ip_ee, 4);
}

static void eeprom_save (void)
{
	eeprom_write_block(my_mac, my_mac_ee, ETH_ALEN);
	eeprom_write_block(my_ip, my_ip_ee, 4);
}

#define DATA_OFF (ETH_HLEN + IP_HLEN + UDP_HLEN)

static void process_udp (uint8_t * buf)
{
	struct ethhdr * eth;
	struct iphdr * ip;
	struct udphdr * udp;
	uint8_t * data;
	uint16_t size;

	struct osc_bound_frame *frm;

	eth = (struct ethhdr *)&buf[0];
	ip = (struct iphdr *)&buf[ETH_HLEN];
	udp = (struct udphdr *)&buf[ETH_HLEN + IP_HLEN];
	data = &buf[ETH_HLEN + IP_HLEN + UDP_HLEN];

	if (be16(udp->dest) != 9999)
		return;

	swap_ethernet(eth);
	swap_ip(ip);
	swap_udp(udp);

	size = be16(udp->len) - UDP_HLEN;

	if (size == 16 && memcmp(data, MSG_INIT, sizeof(MSG_INIT) - 1) == 0) {
		memcpy(remote_ip, &ip->daddr, 4);
		memcpy(remote_mac, &eth->h_dest, ETH_ALEN);
		bound = 1;
		boundtime = jiffies;
	} else {
		return;
	}

	udp->dest = be16(9999);

	ip->ttl = IPDEFTTL;

	memset(data, 0, NETBUF_SZ - DATA_OFF);
	size = snprintf((char *)data, NETBUF_SZ - DATA_OFF,
			MSG_BOUND);
	size += (4 - (size % 4));

	frm = (struct osc_bound_frame *)&buf[DATA_OFF + size];
	frm->fmt[0] = ',';
	frm->fmt[1] = 'i';
	frm->fmt[2] = 'i';
	frm->fmt[3] = 'b';
	frm->timeout = be32(NET_TIMEOUT);
	memcpy(&frm->ip, remote_ip, 4);
	frm->mac_len = be32(6);
	memcpy(&frm->mac_data, remote_mac, 6);
	size += sizeof(struct osc_bound_frame);

	ip->tot_len = be16(IP_HLEN + UDP_HLEN + size);
	udp->len = be16(UDP_HLEN + size);

	ip->check = 0;
	ip->check = be16(checksum((uint8_t *)ip, IP_HLEN, 0));

	udp->check = 0;

	ksz8851_send_packet(buf, DATA_OFF + size);
}

static void process_periodic_udp(uint8_t * buf)
{
	struct ethhdr * eth;
	struct iphdr * ip;
	struct udphdr * udp;
	uint8_t * data;
	uint16_t size;

	eth = (struct ethhdr *)&buf[0];
	ip = (struct iphdr *)&buf[ETH_HLEN];
	udp = (struct udphdr *)&buf[ETH_HLEN + IP_HLEN];
	data = &buf[ETH_HLEN + IP_HLEN + UDP_HLEN];

	if (button_read())
		bound = 0;

	if (jiffies - boundtime > NET_TIMEOUT * HZ)
		bound = 0;

	if (!bound) {
		if (jiffies - lastshot < HZ)
			return;

		memset(data, 0, NETBUF_SZ - DATA_OFF);
		size = snprintf((char *)data, NETBUF_SZ - DATA_OFF,
				MSG_DISCOVER);
		size += (4 - (size % 4));

		buf[DATA_OFF + size] = ',';
		size += 4;

		build_ethernet(eth, NULL, my_mac, ETH_P_IP);
		build_ip(ip, NULL, my_ip, IPPROTO_UDP, UDP_HLEN + size);
		build_udp(udp, 9999, 9999, size);

		ksz8851_send_packet(buf, DATA_OFF + size);
	} else if (bound) {
		struct osc_nrf_frame *frm;

		if (!fifo_count(&rf_rx_fifo))
			return;

		memset(data, 0, NETBUF_SZ - DATA_OFF);
		size = snprintf((char *)data, NETBUF_SZ - DATA_OFF,
				MSG_IO);
		size += (4 - (size % 4));

		frm = (struct osc_nrf_frame *)&buf[DATA_OFF + size];
		frm->fmt[0] = ',';
		frm->fmt[1] = 'b';
		frm->len = be32(PAYLOAD_SIZE);
		memcpy(&frm->data, fifo_get_tail(&rf_rx_fifo), PAYLOAD_SIZE);
		fifo_pop(&rf_rx_fifo);
		size += sizeof(struct osc_nrf_frame);

		build_ethernet(eth, remote_mac, my_mac, ETH_P_IP);
		build_ip(ip, remote_ip, my_ip, IPPROTO_UDP, UDP_HLEN + size);
		build_udp(udp, 9999, 9999, size);

		ksz8851_send_packet(buf, DATA_OFF + size);
	}

	lastshot = jiffies;
}

static void process_mcast(uint8_t * buf)
{
	struct ethhdr * eth;
	struct iphdr * ip;
	struct udphdr * udp;
	uint8_t * data;
	uint16_t size;
	struct osc_nrf_frame *frm;

	eth = (struct ethhdr *)&buf[0];
	ip = (struct iphdr *)&buf[ETH_HLEN];
	udp = (struct udphdr *)&buf[ETH_HLEN + IP_HLEN];
	data = &buf[ETH_HLEN + IP_HLEN + UDP_HLEN];

	if (!fifo_count(&rf_rx_fifo))
		return;

	memset(data, 0, NETBUF_SZ - DATA_OFF);
	size = snprintf((char *)data, NETBUF_SZ - DATA_OFF,
			MSG_IO);
	size += (4 - (size % 4));

	frm = (struct osc_nrf_frame *)&buf[DATA_OFF + size];
	frm->fmt[0] = ',';
	frm->fmt[1] = 'b';
	frm->len = be32(PAYLOAD_SIZE);
	memcpy(&frm->data, fifo_get_tail(&rf_rx_fifo), PAYLOAD_SIZE);
	fifo_pop(&rf_rx_fifo);
	size += sizeof(struct osc_nrf_frame);

	build_ethernet(eth, remote_mcast_mac, my_mac, ETH_P_IP);
	build_ip(ip, remote_mcast_ip, my_ip, IPPROTO_UDP, UDP_HLEN + size);
	build_udp(udp, 9999, 9999, size);

	ksz8851_send_packet(buf, DATA_OFF + size);
}

#define DATAV6_OFF (ETH_HLEN + IPV6_HLEN + UDP_HLEN)
//static void process_mcast6(uint8_t *buf)
//{
//	struct ethhdr *eth;
//	struct ipv6hdr *ip6;
//	struct udphdr *udp;
//	uint8_t *data;
//	uint16_t size;
//	struct osc_nrf_frame *frm;
//
//	eth = (struct ethhdr *)&buf[0];
//	ip6 = (struct ipv6hdr *)&buf[ETH_HLEN];
//	udp = (struct udphdr *)&buf[ETH_HLEN + IPV6_HLEN];
//	data = &buf[ETH_HLEN + IPV6_HLEN + UDP_HLEN];
//
//	if (!fifo_count(&rf_rx_fifo))
//		return;
//
//	memset(data, 0, NETBUF_SZ - DATAV6_OFF);
//	size = snprintf((char *)data, NETBUF_SZ - DATAV6_OFF,
//			MSG_IO);
//	size += (4 - (size % 4));
//
//	frm = (struct osc_nrf_frame *)&buf[DATAV6_OFF + size];
//	frm->fmt[0] = ',';
//	frm->fmt[1] = 'b';
//	frm->len = be32(PAYLOAD_SIZE);
//	memcpy(&frm->data, fifo_get_tail(&rf_rx_fifo), PAYLOAD_SIZE);
//	fifo_pop(&rf_rx_fifo);
//	size += sizeof(struct osc_nrf_frame);
//
//	build_ethernet(eth, remote_mcastv6_mac, my_mac, ETH_P_IPV6);
//	build_ipv6(ip6, &remote_mcast_ipv6, &my_ipv6, IPPROTO_UDP, UDP_HLEN + size);
//	build_udp(udp, 9999, 9999, size);
//	build_udp_checksum_v6(ip6, udp, size);
//
//	ksz8851_send_packet(buf, DATAV6_OFF + size);
//}

static void process_unicast(uint8_t *buf)
{
	uint16_t len;

	if (ksz8851_has_data()) {
		len = ksz8851_read_packet(buf, NETBUF_SZ);
		if (!len)
			return;
    
		if (!ethernet_checkdest((struct ethhdr *)&buf[0], my_mac))
			return;

		if (get_ethertype((struct ethhdr *)&buf[0]) == be16(ETH_P_ARP)) {

			process_arp(buf, my_mac, my_ip);

		} else if (get_ethertype((struct ethhdr *)&buf[0]) == be16(ETH_P_IP)) {

			if (get_ipproto((struct iphdr *)&buf[ETH_HLEN], my_ip) == IPPROTO_ICMP)
				process_icmp(buf);
			else if (get_ipproto((struct iphdr *)&buf[ETH_HLEN], my_ip) == IPPROTO_UDP)
				process_udp(buf);
		}

	}
}


void net_poll(void)
{
	switch (net_mode) {
		case MODE_UNICAST:
			process_unicast(buf);
			process_periodic_udp(buf);
			break;
		case MODE_MCAST:
			process_mcast(buf);
			break;
		case MODE_MCAST6:
			process_mcast6(buf);
			break;
	}
}

void ISR_tcpbase(void)
{
  jiffies++;
}
