/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <string.h>
#include <stdio.h>

#include "utils.h"
#include "modbus.h"

static int modbus_construct_mbap_header(struct mbap_t mbap, uint8_t *result_buf){
    result_buf = memset(result_buf, 0x00, MBAP_HEADER_SIZE);
    result_buf[0] = (uint8_t) (mbap.trans_id >> 8);
    result_buf[1] = (uint8_t) (mbap.trans_id & 0xff);
    result_buf[2] = (uint8_t) (mbap.proto_id >> 8);
    result_buf[3] = (uint8_t) (mbap.proto_id & 0xff);
    result_buf[4] = (uint8_t) (mbap.msg_len  >> 8);
    result_buf[5] = (uint8_t) (mbap.msg_len  & 0xff);  
    result_buf[6] = mbap.unit_id;
    return MBAP_HEADER_SIZE;
}

static int modbus_construct_request_pdu(struct request_pdu_t req_pdu, uint8_t *result_buf){
    result_buf = memset(result_buf, 0x00, REQUEST_PDU_SIZE);
    result_buf[0] = req_pdu.func_code;
    result_buf[1] = (uint8_t) (req_pdu.start_addr >> 8);
    result_buf[2] = (uint8_t) (req_pdu.start_addr & 0xff);
    result_buf[3] = (uint8_t) (req_pdu.num_of_dis >> 8);
    result_buf[4] = (uint8_t) (req_pdu.num_of_dis & 0xff);
    return REQUEST_PDU_SIZE;
}

/**
 * 
 * @param req the request array that will be modified in place
 * @return 
 */
static void modbus_set_transaction_id(uint8_t *req, uint16_t trans_id){
    req[0] = (uint8_t) (trans_id >> 8);
    req[1] = (uint8_t) (trans_id & 0xff);
}

int modbus_construct_request(uint16_t trans_id, uint8_t unit_id,
                      uint16_t n_of_dis, uint8_t * out_buf, size_t out_buf_size, 
                      size_t * req_size){
    struct mbap_t mbap;
    struct request_pdu_t reqpdu;
    uint8_t mbap_buf[MBAP_HEADER_SIZE];
    uint8_t request_pdu_buf[REQUEST_PDU_SIZE];
    
    if (out_buf_size < sizeof(mbap_buf) + sizeof(request_pdu_buf)){
        // Not enough size allocated to produce the request
        return -1;
    }
    
    mbap.trans_id = trans_id; // unit specific parameter
    mbap.proto_id = MB_PROTO_ID;
    mbap.msg_len = sizeof(request_pdu_buf) + 1;
    mbap.unit_id = unit_id; // unit specific parameter
    
    reqpdu.func_code = MB_FUNC_READ_DI;
    reqpdu.start_addr = 0;
    reqpdu.num_of_dis = n_of_dis; // unit specific parameter
    
    modbus_construct_mbap_header(mbap, (uint8_t*) &mbap_buf);
    modbus_construct_request_pdu(reqpdu, (uint8_t*) &request_pdu_buf);

    
    memcpy(out_buf, mbap_buf, sizeof(mbap_buf));
    memcpy(&out_buf[sizeof(mbap_buf)], request_pdu_buf, sizeof(request_pdu_buf));
    *req_size = sizeof(mbap_buf) + sizeof(request_pdu_buf);
    /*
    printf("(construct_request) MBAP Header\n");
    print_bytes(mbap_buf, sizeof(mbap_buf));
    printf("(construct_request) PDU\n");
    print_bytes(request_pdu_buf, sizeof(request_pdu_buf));
    printf("(construct_request) Total array\n");
    print_bytes(out_buf, *req_size);    
    printf("\n");
    */
    return 0;
}

int modbus_decode_mbap_header(const uint8_t *arr, struct mbap_t *header){
    header->trans_id = (arr[0] << 8) | arr[1];
    header->proto_id = (arr[2] << 8) | arr[3];
    header->msg_len  = (arr[4] << 8) | arr[5];
    header->unit_id  = arr[6];
    return 0;
}
