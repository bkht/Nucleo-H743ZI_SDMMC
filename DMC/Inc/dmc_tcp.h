#ifndef __DMC_TCP_H
#define __DMC_TCP_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32h7xx_hal.h"
#include "main.h"
#include <stdint.h>
#include <string.h>

/* ETH header */
#define ETH_HDR_OFFSET        0
#define ETH_HDR_SIZE          14
#define ETH_HDR_SIZE_MAC      6
#define ETH_HDR_DEST_MAC      0
#define ETH_HDR_SOURCE_MAC    6
#define ETH_HDR_TYPE          12

/* IP header */
#define IP_HDR_OFFSET         14
#define IP_HDR_SIZE           20
#define IP_HDR_SIZE_IP        4
#define IP_HDR_TOTAL_LENGTH   2
#define IP_HDR_IDENTIFICATION 4
#define IP_HDR_PROTOCOL       9
#define IP_HDR_HDR_CHECKSUM   10
#define IP_HDR_SOURCE_IP      12
#define IP_HDR_DEST_IP        16

/* TCP header */
#define TCP_HDR_OFFSET        34
#define TCP_HDR_SIZE          20
#define TCP_HDR_SIZE_PORT     2
#define TCP_HDR_SOURCE_PORT   0
#define TCP_HDR_DEST_PORT     2
#define TCP_HDR_SIZE_SEQ      4
#define TCP_HDR_SEQ_NUMBER    4
#define TCP_HDR_ACK_NUMBER    8
#define TCP_HDR_WINDOW_SIZE   14
#define TCP_HDR_CHECKSUM      16

/* TCP Payload */
#define TCP_PAYLOAD           54

#define IP_PROTO_ICMP_V 1
#define IP_PROTO_TCP_V 6
// 17=0x11
#define IP_PROTO_UDP_V 17

/* modbus */
#define MB_HDR_OFFSET         54
#define MB_HDR_TI             0
#define MB_HDR_PI             2
#define MB_HDR_LENGTH         4
#define MB_HDR_UI             6
#define MB_HDR_FC             7
#define MB_HDR_REG            8
#define MB_HDR_REG_COUNT      10
#define MB_HDR_BYTE_COUNT     8
#define MB_HDR_BYTES          9
#define MB_HDR_EXCEPTION_CODE 8

#define MB_EXCEPTION_ILLEGAL_FUNCTION                         0x01
#define MB_EXCEPTION_ILLEGAL_DATA_ADDRESS                     0x02
#define MB_EXCEPTION_ILLEGAL_DATA_VALUE                       0x03
#define MB_EXCEPTION_SLAVE_DEVICE_FAILURE                     0x04
#define MB_EXCEPTION_ACKNOWLEDGE                              0x05
#define MB_EXCEPTION_SLAVE_DEVICE_BUSY                        0x06
#define MB_EXCEPTION_NEGATIVE_ACKNOWLEDGE                     0x07
#define MB_EXCEPTION_MEMORY_PARITY_ERROR                      0x08
#define MB_EXCEPTION_GATEWAY_PATH_UNAVAILABLE                 0x10
#define MB_EXCEPTION_GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND  0x11

uint16_t mb_set_exception(uint8_t *buf, uint8_t exc);
uint16_t mb_set_ok(uint8_t *buf);

uint16_t mb_set_len(uint8_t *buf, uint16_t len);
uint16_t mb_set_request(uint8_t *buf, uint16_t reg, uint16_t reg_count);
uint16_t mb_set_reply(uint8_t *buf, uint8_t *bytes, uint8_t byte_count);
void increment_acknowledge_number(uint8_t *buf);
void increment_identification(uint8_t *buf);
void increment_window_size(uint8_t *buf);

void swap_buf_all(uint8_t *buf);
void swap_buf_eth(uint8_t *buf);
void swap_buf_ip(uint8_t *buf);
void swap_buf_tcp(uint8_t *buf);

uint16_t dmc_tcp_get_total_length(uint8_t *buf);
void dmc_tcp_set_total_length(uint8_t *buf, uint16_t packet_len);

uint16_t dmc_tcp_get_modbus_transaction_identifier(uint8_t *buf);
void dmc_tcp_set_modbus_transaction_identifier(uint8_t *buf, uint16_t mb_ti);

uint16_t dmc_tcp_checksum(uint8_t *buf, uint16_t len, uint8_t type);
void dmc_tcp_set_tcp_checksum(uint8_t *buf);

#ifdef __cplusplus
}
#endif
#endif /* __DMC_TCP_H */
