#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>

#include "modbus.h"
#include "modbus_client.h"
#include "modbus_slave.h"
#include "utils.h"

int mbcli_get_unit_state(const int        sockfd, 
                         const uint16_t   trans_id,
                         const uint8_t    unit_id,
                         const uint16_t   di_count,
                         uint16_t       * state,
                         uint16_t       * exception
                   ){
    uint8_t request[MODBUS_RESPONSE_BUF_SIZE];
    uint8_t response[MODBUS_RESPONSE_BUF_SIZE];
    Mbap    mbap_header;
    size_t  request_size;
    
    modbus_construct_request(trans_id, unit_id, di_count, 
            request, MODBUS_REQUEST_BUF_SIZE, &request_size);
    mbcli_poll_gateway(sockfd, request, request_size, response, MODBUS_RESPONSE_BUF_SIZE);
    modbus_decode_mbap_header(response, &mbap_header);
    printf("Got MBAP Header: trans=0x%04x, proto=0x%04x, length=0x%04x, unit=0x%02x\n", 
            mbap_header.trans_id,
            mbap_header.proto_id, 
            mbap_header.msg_len, 
            mbap_header.unit_id);
    printf("fcode=0x%02x\n", response[MB_FCODE_OFFSET]);
    printf("Data (hex): \n");
    //for (size_t i = 0; i < mbap_header.msg_len; i++){
    for (size_t i = 0; i < mbap_header.msg_len+8; i++){
        //printf("%02x ", response[MB_DATA_OFFSET + i]);
        printf("%02x ", response[i]);
    }
    printf("\n");
    // response[MB_DATA_OFFSET + 1] contains num of bytes to follow in the 
    // FC 02 specific message
    *state = (response[MB_DATA_OFFSET + 2] << 8) | (response[MB_DATA_OFFSET + 1]);
    *exception = 0;

    if (response[MB_FCODE_OFFSET] & MB_FCODE_EXCEPTION_BITMASK){
        printf("WARNING: Exception set!\n");
        *exception = response[MB_FCODE_OFFSET + 1];
    }
    return 0;
}


/**
 * Do one gateway polling
 * Preconditions for running this function is that
 * * Socket is initialized
 * * Gateway data is initialized
 * @param sockfd
 * @param gateway
 * @param req
 * @param req_size
 * @param resp_buf
 * @param resp_buf_size
 * @return 
 */

static int mbcli_poll_gateway(int       sockfd,
                       uint8_t * req, 
                       size_t    req_size, 
                       uint8_t * resp_buf, 
                       size_t    resp_buf_size){
    const size_t DATA_BUF_SIZE = 256;
    char buffer[DATA_BUF_SIZE];
    size_t mbap_header_bytes = 0;
    size_t pdu_data_bytes = 0;
    size_t bytes_written = 0;
    size_t bytes_read = 0;
    
    printf("       Request: \n");
    print_bytes(req, req_size);
    
    bytes_written = write(sockfd, req, req_size);
    if (bytes_written < 0){
        error("ERROR writing to socket");
    }
    else{
        printf("Successfully written %zu bytes to socket\n", bytes_written);
    }
    memset(buffer, 0, 256);
    
    //MBAP Header
    printf("Attempting to read MBAP header\n");
    bytes_read = read(sockfd, buffer, MBAP_HEADER_SIZE);
    if (bytes_read < 0){
         error("ERROR reading from socket");
    }
    else{
        mbap_header_bytes = bytes_read;
        printf("Successfully read %zu bytes of response\n", bytes_read);
        print_bytes(buffer, bytes_read);
    }
    Mbap mbap_header;
    modbus_decode_mbap_header(buffer, &mbap_header);
    uint16_t pdu_len = mbap_header.msg_len;
    // PDU
    printf("Attempting to read PDU\n");
    bytes_read = read(sockfd, &buffer[MBAP_HEADER_SIZE], DATA_BUF_SIZE);
    if (bytes_read < 0){
        error("ERROR reading from socket");
    }
    else{
        pdu_data_bytes = bytes_read;
        printf("Successfully read %zu bytes of response\n", bytes_read);
        print_bytes(&buffer[MBAP_HEADER_SIZE], bytes_read);
    }
    //printf("      mbap_header_bytes = %u, pdu_data_bytes = %u\n", 
    //       mbap_header_bytes, pdu_data_bytes);
    size_t bytes_to_copy = MBAP_HEADER_SIZE + pdu_len;
    if (bytes_to_copy > resp_buf_size){ bytes_to_copy = resp_buf_size; }
    memcpy(resp_buf, buffer, bytes_to_copy);
    return MBAP_HEADER_SIZE + pdu_len;
}
