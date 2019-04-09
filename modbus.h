/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   modbus.h
 * Author: Mark
 *
 * Created on den 23 februari 2018, 21:42
 */

#ifndef MODBUS_H
#define MODBUS_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdint.h>
    
#define MBAP_HEADER_SIZE      0x07
#define REQUEST_PDU_SIZE      0x05

#define MB_FUNC_READ_DI       0x02
#define MB_PROTO_ID           0x00
    
#define MB_DATA_OFFSET        0x08
#define MB_FCODE_OFFSET       0x07

#define MB_FCODE_EXCEPTION_BITMASK  0x80

typedef struct mbap_t{
    uint16_t trans_id;
    uint16_t proto_id;
    uint16_t msg_len;
    uint8_t  unit_id;
} Mbap;

typedef struct request_pdu_t{
    uint8_t  func_code;
    uint16_t start_addr;
    uint16_t num_of_dis;
} RequestPDU;


static int  modbus_construct_mbap_header(struct mbap_t mbap_buf, uint8_t *result_buf);
static int  modbus_construct_request_pdu(struct request_pdu_t req_pdu_buf, uint8_t *result_buf);
static void modbus_set_transaction_id(uint8_t *req, uint16_t trans_id);
int         modbus_construct_request(uint16_t trans_id, uint8_t unit_id,
                                     uint16_t n_of_dis, uint8_t *out_buf, 
                                     size_t out_buf_size, size_t * request_size);
int         modbus_decode_mbap_header(const uint8_t *arr, struct mbap_t *header);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_H */

