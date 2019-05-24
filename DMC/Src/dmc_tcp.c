#include "dmc_tcp.h"

uint16_t mb_set_exception(uint8_t *buf, uint8_t exc)
{
  // http://www.simplymodbus.ca/exceptions.htm

  buf[MB_HDR_OFFSET + MB_HDR_FC] |= 0x80;
  buf[MB_HDR_OFFSET + MB_HDR_EXCEPTION_CODE] = exc;
  uint16_t len = 3;

  buf[MB_HDR_OFFSET + MB_HDR_LENGTH] = len << 8;
  buf[MB_HDR_OFFSET + MB_HDR_LENGTH + 1] = len & 0xff;
  len = 49;
  buf[IP_HDR_OFFSET + IP_HDR_TOTAL_LENGTH] = len << 8;
  buf[IP_HDR_OFFSET + IP_HDR_TOTAL_LENGTH + 1] = len & 0xff;
  // return total length
  return len + IP_HDR_OFFSET;
}

uint16_t mb_set_ok(uint8_t *buf)
{
  // http://www.simplymodbus.ca/exceptions.htm
  uint16_t len = 6;

  buf[MB_HDR_OFFSET + MB_HDR_LENGTH] = len << 8;
  buf[MB_HDR_OFFSET + MB_HDR_LENGTH + 1] = len & 0xff;
  len = 52;
  buf[IP_HDR_OFFSET + IP_HDR_TOTAL_LENGTH] = len << 8;
  buf[IP_HDR_OFFSET + IP_HDR_TOTAL_LENGTH + 1] = len & 0xff;
  // return total length
  return len + IP_HDR_OFFSET;
}

uint16_t mb_set_len(uint8_t *buf, uint16_t len)
{
  buf[MB_HDR_OFFSET + MB_HDR_LENGTH] = len << 8;
  buf[MB_HDR_OFFSET + MB_HDR_LENGTH + 1] = len & 0xff;
  len += 46;
  buf[IP_HDR_OFFSET + IP_HDR_TOTAL_LENGTH] = len << 8;
  buf[IP_HDR_OFFSET + IP_HDR_TOTAL_LENGTH + 1] = len & 0xff;
  // return total length
  return len + IP_HDR_OFFSET;
}

uint16_t mb_set_request(uint8_t *buf, uint16_t reg, uint16_t reg_count)
{
  uint16_t len = 6;
  memcpy(&buf[MB_HDR_OFFSET + MB_HDR_LENGTH], &len, 2);
  memcpy(&buf[MB_HDR_OFFSET + MB_HDR_REG], &reg, 2);
  memcpy(&buf[MB_HDR_OFFSET + MB_HDR_REG_COUNT], &reg_count, 2);

  buf[MB_HDR_OFFSET + MB_HDR_LENGTH] = len << 8;
  buf[MB_HDR_OFFSET + MB_HDR_LENGTH + 1] = len & 0xff;
  len += 46;
  buf[IP_HDR_OFFSET + IP_HDR_TOTAL_LENGTH] = len << 8;
  buf[IP_HDR_OFFSET + IP_HDR_TOTAL_LENGTH + 1] = len & 0xff;
  // return total length
  return len + IP_HDR_OFFSET;
}

uint16_t mb_set_reply(uint8_t *buf, uint8_t *bytes, uint8_t byte_count)
{
//  uint16_t len = byte_count + 3;
//  buf[MB_HDR_OFFSET + MB_HDR_BYTE_COUNT] = byte_count;

//  for (uint16_t i = 0; i < byte_count; i++)
//  {
//    buf[MB_HDR_OFFSET + MB_HDR_UI + i] = bytes[i];
//  }
  // Alter IP ID
  buf[IP_HDR_OFFSET + 4] = 0x8f;
  buf[IP_HDR_OFFSET + 5] = 0x52;

  buf[MB_HDR_OFFSET + MB_HDR_UI + 0] = 0x01;
  buf[MB_HDR_OFFSET + MB_HDR_UI + 1] = 0x10;
  buf[MB_HDR_OFFSET + MB_HDR_UI + 2] = 0x00;
  buf[MB_HDR_OFFSET + MB_HDR_UI + 3] = 0x00;
  buf[MB_HDR_OFFSET + MB_HDR_UI + 4] = 0x00;
  buf[MB_HDR_OFFSET + MB_HDR_UI + 5] = 0x01;
//  memcpy(&buf[MB_HDR_OFFSET + MB_HDR_BYTES], bytes, byte_count);

  buf[MB_HDR_OFFSET + MB_HDR_LENGTH] = byte_count << 8;
  buf[MB_HDR_OFFSET + MB_HDR_LENGTH + 1] = byte_count & 0xff;
  byte_count += 46;
  buf[IP_HDR_OFFSET + IP_HDR_TOTAL_LENGTH] = byte_count << 8;
  buf[IP_HDR_OFFSET + IP_HDR_TOTAL_LENGTH + 1] = byte_count & 0xff;
  // return total length
  return byte_count + IP_HDR_OFFSET;
}

void increment_acknowledge_number(uint8_t *buf)
{
  uint32_t tmp = ((buf[TCP_HDR_OFFSET + TCP_HDR_ACK_NUMBER + 0] << 24) & 0xff) |
                 ((buf[TCP_HDR_OFFSET + TCP_HDR_ACK_NUMBER + 1] << 16) & 0xff) |
                 ((buf[TCP_HDR_OFFSET + TCP_HDR_ACK_NUMBER + 2] << 8) & 0xff) |
                  (buf[TCP_HDR_OFFSET + TCP_HDR_ACK_NUMBER + 3] & 0xff);

  tmp += 15;

  buf[TCP_HDR_OFFSET + TCP_HDR_ACK_NUMBER + 0] = ((tmp & 0xff000000) >> 24);
  buf[TCP_HDR_OFFSET + TCP_HDR_ACK_NUMBER + 1] = ((tmp & 0xff0000) >> 16);
  buf[TCP_HDR_OFFSET + TCP_HDR_ACK_NUMBER + 1] = ((tmp & 0xff00) >> 8);
  buf[TCP_HDR_OFFSET + TCP_HDR_ACK_NUMBER + 1] = (tmp & 0xff);
}

void increment_identification(uint8_t *buf)
{
  uint16_t tmp = (buf[IP_HDR_OFFSET + IP_HDR_IDENTIFICATION] << 8) | (buf[IP_HDR_OFFSET + IP_HDR_IDENTIFICATION + 1] & 0xff);

  tmp++;

  buf[IP_HDR_OFFSET + IP_HDR_IDENTIFICATION] = (tmp >> 8);
  buf[IP_HDR_OFFSET + IP_HDR_IDENTIFICATION + 1] = (tmp & 0xff);
}

void increment_window_size(uint8_t *buf)
{
  uint16_t tmp = (buf[TCP_HDR_OFFSET + TCP_HDR_WINDOW_SIZE] << 8) | (buf[TCP_HDR_OFFSET + TCP_HDR_WINDOW_SIZE + 1] & 0xff);

  tmp += 24;

  buf[TCP_HDR_OFFSET + TCP_HDR_WINDOW_SIZE] = (tmp >> 8);
  buf[TCP_HDR_OFFSET + TCP_HDR_WINDOW_SIZE + 1] = (tmp & 0xff);
}

void swap_buf_all(uint8_t *buf)
{
  uint8_t tmp[6];

  // Swap MAC
  memcpy(tmp, &buf[ETH_HDR_DEST_MAC], ETH_HDR_SIZE_MAC);
  memcpy(&buf[ETH_HDR_DEST_MAC], &buf[ETH_HDR_SOURCE_MAC], ETH_HDR_SIZE_MAC);
  memcpy(&buf[ETH_HDR_SOURCE_MAC], tmp, ETH_HDR_SIZE_MAC);

  // Swap IP
  memcpy(tmp, &buf[IP_HDR_OFFSET + IP_HDR_DEST_IP], IP_HDR_SIZE_IP);
  memcpy(&buf[IP_HDR_OFFSET + IP_HDR_DEST_IP], &buf[IP_HDR_OFFSET + IP_HDR_SOURCE_IP], IP_HDR_SIZE_IP);
  memcpy(&buf[IP_HDR_OFFSET + IP_HDR_SOURCE_IP], tmp, IP_HDR_SIZE_IP);

  // Swap Port
  memcpy(tmp, &buf[TCP_HDR_OFFSET + TCP_HDR_DEST_PORT], TCP_HDR_SIZE_PORT);
  memcpy(&buf[TCP_HDR_OFFSET + TCP_HDR_DEST_PORT], &buf[TCP_HDR_OFFSET + TCP_HDR_SOURCE_PORT], TCP_HDR_SIZE_PORT);
  memcpy(&buf[TCP_HDR_OFFSET + TCP_HDR_SOURCE_PORT], tmp, TCP_HDR_SIZE_PORT);

  // Swap SEQ ACK Number
  memcpy(tmp, &buf[TCP_HDR_OFFSET + TCP_HDR_SEQ_NUMBER], TCP_HDR_SIZE_SEQ);
  memcpy(&buf[TCP_HDR_OFFSET + TCP_HDR_SEQ_NUMBER], &buf[TCP_HDR_OFFSET + TCP_HDR_ACK_NUMBER], TCP_HDR_SIZE_SEQ);
  memcpy(&buf[TCP_HDR_OFFSET + TCP_HDR_ACK_NUMBER], tmp, TCP_HDR_SIZE_SEQ);

  // Alter TCP_HDR_ACK_NUMBER
//  buf[TCP_HDR_OFFSET + TCP_HDR_ACK_NUMBER + 3] += 1;

  // Alter Window Size
//  buf[TCP_HDR_OFFSET + TCP_HDR_WINDOW_SIZE + 1] += 1;

}

void swap_buf_eth(uint8_t *buf)
{
  uint8_t tmp[6];

  // Swap MAC
  memcpy(tmp, &buf[ETH_HDR_DEST_MAC], ETH_HDR_SIZE_MAC);
  memcpy(&buf[ETH_HDR_DEST_MAC], &buf[ETH_HDR_SOURCE_MAC], ETH_HDR_SIZE_MAC);
  memcpy(&buf[ETH_HDR_SOURCE_MAC], tmp, ETH_HDR_SIZE_MAC);
}

void swap_buf_ip(uint8_t *buf)
{
  uint8_t tmp[4];

  // Swap IP
  memcpy(tmp, &buf[IP_HDR_OFFSET + IP_HDR_DEST_IP], IP_HDR_SIZE_IP);
  memcpy(&buf[IP_HDR_OFFSET + IP_HDR_DEST_IP], &buf[IP_HDR_OFFSET + IP_HDR_SOURCE_IP], IP_HDR_SIZE_IP);
  memcpy(&buf[IP_HDR_OFFSET + IP_HDR_SOURCE_IP], tmp, IP_HDR_SIZE_IP);
}

void swap_buf_tcp(uint8_t *buf)
{
  uint8_t tmp[2];

  // Swap Port
  memcpy(tmp, &buf[TCP_HDR_OFFSET + TCP_HDR_DEST_PORT], TCP_HDR_SIZE_PORT);
  memcpy(&buf[TCP_HDR_OFFSET + TCP_HDR_DEST_PORT], &buf[TCP_HDR_OFFSET + TCP_HDR_SOURCE_PORT], TCP_HDR_SIZE_PORT);
  memcpy(&buf[TCP_HDR_OFFSET + TCP_HDR_SOURCE_PORT], tmp, TCP_HDR_SIZE_PORT);
}

uint16_t dmc_tcp_get_total_length(uint8_t *buf)
{
  return ((buf[IP_HDR_OFFSET + IP_HDR_TOTAL_LENGTH] << 8) | (buf[IP_HDR_OFFSET + IP_HDR_TOTAL_LENGTH + 1] & 0xff));
}

void dmc_tcp_set_total_length(uint8_t *buf, uint16_t packet_len)
{
  buf[IP_HDR_OFFSET + IP_HDR_TOTAL_LENGTH] = (packet_len >> 8);
  buf[IP_HDR_OFFSET + IP_HDR_TOTAL_LENGTH + 1] = (packet_len & 0xff);
}

uint16_t dmc_tcp_get_modbus_transaction_identifier(uint8_t *buf)
{
  return ((buf[MB_HDR_OFFSET + MB_HDR_TI] << 8) | (buf[MB_HDR_OFFSET + MB_HDR_TI + 1] & 0xff));
}

void dmc_tcp_set_modbus_transaction_identifier(uint8_t *buf, uint16_t mb_ti)
{
  buf[MB_HDR_OFFSET + MB_HDR_TI] = (mb_ti >> 8);
  buf[MB_HDR_OFFSET + MB_HDR_TI + 1] = (mb_ti & 0xff);
}

// The IP checksum is calculated over the IP header only, starting with
// the header length field and a total length of 20 bytes unitl ip.dst
// You must set the IP checksum field to zero before you start the calculation.
// len for IP is 20.
//
// For UDP/TCP we do not make up the required pseudo header. Instead we
// use the ip.src and ip.dst fields of the real packet:
// The UDP checksum calculation starts with the ip.src field
// Ip.src = 4 bytes
// Ip.dst = 4 bytes,
// Udp header = 8 bytes + data length = 16 + len
// In other words the len here is 8 + length over which you actually
// want to calculate the checksum.
// You must set the checksum field to zero before you start the calculation.
// The same algorithm is also used for UDP and TCP checksums.
// len for UDP is: 8 + 8 + data length
// len for TCP is: 4 + 4 + 20 + option len + data length
//
// For more information on how this algorithm works see:
// http://www.netfor2.com/checksum.html
// http://www.msc.uky.edu/ken/cs471/notes/chap3.htm
// The RFC has also a C code example: http://www.faqs.org/rfcs/rfc1071.html
uint16_t dmc_tcp_checksum(uint8_t *buf, uint16_t len, uint8_t type)
{
  // type 0 = IP , ICMP
  //      1 = UDP
  //      2 = TCP
  uint32_t sum = 0;
//  dmc_puts("dmc_tcp_checksum\n");
//  if (type == 0)
//  {
//    // Do not add anything, standard IP checksum as described above
//    // Usable for ICMP and IP header
//  }
  if (type == 1)
  {
    sum += IP_PROTO_UDP_V; // protocol UDP
    // The length here is the length of UDP (data+header len)
    // =length given to this function - (IP.scr+IP.dst length)
    sum += len - 8; // = real UDP len
  }
  else if (type == 2)
  {
    sum += IP_PROTO_TCP_V;
    // The length here is the length of TCP (data+header len)
    // =length given to this function - (IP.scr+IP.dst length)
    sum += len - 8; // = real TCP len
  }

  // Build the sum of 16bit words
  while (len > 1)
  {
    sum += 0xFFFF & (((uint32_t) *buf << 8) | *(buf + 1));
    buf += 2;
    len -= 2;
  }

  // If there is a byte left then add it (padded with zero)
  if (len)
  {
    sum += ((uint32_t) (0xFF & *buf)) << 8;
  }
//  dmc_puthex4cr(sum);

  // Now calculate the sum over the bytes in the sum
  // until the result is only 16 bits long
  while (sum >> 16)
  {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }
//  dmc_puthex4cr(sum);

  // Build 1's complement:
  return ((uint16_t) sum ^ 0xFFFF);
}

void dmc_tcp_set_tcp_checksum(uint8_t *buf)
{
  // Zero the checksum
  buf[0x32] = 0;
  buf[0x33] = 0;
  // Calculate the checksum, len=8 (start from ip.src) + TCP_HEADER_LEN_PLAIN + data len
  uint16_t dlen = dmc_tcp_get_total_length(buf);
  uint16_t ck = dmc_tcp_checksum(&buf[0x1a], dlen - 12, 2);
  buf[0x32] = ck >> 8;
  buf[0x33] = ck & 0xff;
}

uint16_t CheckSum(uint16_t *buffer, int size)
{
    unsigned long cksum=0;
    while(size >1)
    {
        cksum+=*buffer++;
        size -=sizeof(uint16_t);
    }
    if(size)
        cksum += *(uint8_t*)buffer;

    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >>16);
    return (uint16_t)(~cksum);
}

uint16_t checksum16(void *data, unsigned int bytes){
    uint16_t *data_pointer = (uint16_t *) data;
    uint32_t total_sum;

    while(bytes > 1){
        total_sum += *data_pointer++;
        //If it overflows to the MSBs add it straight away
        if(total_sum >> 16){
            total_sum = (total_sum >> 16) + (total_sum & 0x0000FFFF);
        }
        bytes -= 2; //Consumed 2 bytes
    }
    if(1 == bytes){
        //Add the last byte
        total_sum += *(((uint8_t *) data_pointer) + 1);
        //If it overflows to the MSBs add it straight away
        if(total_sum >> 16){
            total_sum = (total_sum >> 16) + (total_sum & 0x0000FFFF);
        }
        bytes -= 1;
    }

    return (~((uint16_t) total_sum));
}
