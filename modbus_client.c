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

int poll_gateway(int sockfd,
                 uint8_t *req, 
                 size_t req_size, 
                 uint8_t *resp_buf, 
                 size_t resp_buf_size){

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    size_t bytes_written = write(sockfd, req, req_size);
    if (bytes_written < 0){
        error("ERROR writing to socket");
    }
    else{
        //printf("Successfully written %zu bytes to socket\n", bytes_written);
    }
    memset(buffer, 0, 256);
    
    //MBAP Header
    //printf("Attempting to read MBAP header\n");
    size_t bytes_read = read(sockfd, buffer, MBAP_HEADER_SIZE);
    if (bytes_read < 0){
         error("ERROR reading from socket");
    }
    else{
        //printf("Successfully read %zu bytes of response\n", bytes_read);
        //print_bytes(buffer, bytes_read);
    }
    Mbap mbap_header;
    decode_mbap_header(buffer, &mbap_header);
    uint16_t pdu_len = mbap_header.msg_len - 1;
    // PDU
    //printf("Attempting to read PDU\n");
    bytes_read = read(sockfd, &buffer[MBAP_HEADER_SIZE], pdu_len);
    if (bytes_read < 0){
         error("ERROR reading from socket");
    }
    else{
        //printf("Successfully read %d bytes of response\n", bytes_read);
        //print_bytes(&buffer[MBAP_HEADER_SIZE], bytes_read);
    }
    size_t bytes_to_copy = MBAP_HEADER_SIZE + pdu_len;
    if (bytes_to_copy > resp_buf_size){ bytes_to_copy = resp_buf_size; }
    memcpy(resp_buf, buffer, bytes_to_copy);
    return MBAP_HEADER_SIZE + pdu_len;
}


/**
 * Prepares gateway polling, 
 * given the gateway data we initialize the sockaddr_in structure and 
 * the file descriptor.
 * @param [in] slave
 * @param [out] serv_addr pointer to resulting serv_addr
 * @param [out] sockfd file descriptor
 * @return 
 */
int prepare_gateway_poll(ModbusSlave slave, 
        struct sockaddr_in *serv_addr, int *sockfd){
    struct hostent *server;
    
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (*sockfd < 0){
        error("ERROR opening socket");
    }
    
    // TODO: gethostbyname is obsolete, find alternative!
    server = gethostbyname(slave.ip);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    memset((char *) serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr->sin_family = AF_INET;
    memcpy((char *) &serv_addr->sin_addr.s_addr,
           (char *) server->h_addr,
           server->h_length);
    serv_addr->sin_port = htons(slave.port);
    
    return 0;
}
