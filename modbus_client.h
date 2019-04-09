/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   modbus_client.h
 * Author: Mark
 *
 * Created on den 20 mars 2018, 22:31
 */

#ifndef MODBUS_CLIENT_H
#define MODBUS_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif
    
#define MODBUS_REQUEST_BUF_SIZE  256
#define MODBUS_RESPONSE_BUF_SIZE 256

int mbcli_get_unit_state(const int        sockfd, 
                         const uint16_t   trans_id,
                         const uint8_t    unit_id,
                         const uint16_t   di_count,
                         uint16_t       * state,
                         uint16_t       * exception);

static int mbcli_poll_gateway(int sockfd,
                 uint8_t *req, 
                 size_t req_size, 
                 uint8_t *resp_buf, 
                 size_t resp_buf_size);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_CLIENT_H */

