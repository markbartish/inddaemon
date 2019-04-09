/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   utils.h
 * Author: Mark
 *
 * Created on den 9 mars 2018, 22:09
 */

#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void error(char *msg);  
    
uint64_t utils_get_time_millis();

void print_bytes_msg(void *arr, char *msg, uint32_t size);
void print_bytes(void *arr, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_H */

