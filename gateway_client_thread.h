/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   gateway_client_thread.h
 * Author: Mark
 *
 * Created on den 25 februari 2018, 16:00
 */

#ifndef GATEWAY_CLIENT_THREAD_H
#define GATEWAY_CLIENT_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <time.h>
#include <stdbool.h>
#include "modbus_slave.h"
    
typedef struct gateway_client_parameters_t{
    useconds_t poll_interval;
    ModbusSlave slave;
    //Mbap mbap;
    //RequestPDU pdu;
    bool (*check_stop)(pthread_t tid);
} GatewayClientParameters;

void * gateway_client_transmit(void * argp);
void * gateway_client_receive(void * argp);


#ifdef __cplusplus
}
#endif

#endif /* GATEWAY_CLIENT_THREAD_H */

