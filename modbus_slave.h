/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   modbus_slave.h
 * Author: Mark
 *
 * Created on den 22 mars 2018, 20:10
 */

#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdbool.h>

#define MBSLAVE_IP_SIZE 64
    
typedef struct modbus_slave_t{
    char                  ip[MBSLAVE_IP_SIZE];
    uint16_t              port;
    uint8_t               id;
    uint8_t               n_of_dis;
    bool                  inverted;
} ModbusSlave;


#ifdef __cplusplus
}
#endif

#endif /* MODBUS_SLAVE_H */

