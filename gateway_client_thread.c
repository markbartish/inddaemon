/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "modbus.h"
#include "gateway_client_thread.h"
#include "utils.h"


/*
 int sockfd, ModbusGateway gateway, 
        uint8_t *req, size_t req_size, uint8_t *resp_buf, size_t resp_buf_size
 */
void * gateway_client_transmit(void * argp){
    size_t RESP_BUF_SIZE = 65536;
    GatewayClientParameters *gwcp = 
        (GatewayClientParameters *) argp;
    pthread_t this_tid = pthread_self();
    uint8_t resp_buf[RESP_BUF_SIZE];
    
    while(!gwcp->check_stop(this_tid)){
        //poll_gateway();
        printf("Thread %d here, running...\n", this_tid);
        usleep(1000000);
    }
    return NULL;
}

void * gateway_client_receive(void *argp){
    int *sockfd = (int*) argp;
    
    return NULL;
}