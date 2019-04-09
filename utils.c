/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"

#if 0
struct timeval {
    time_t      tv_sec;     /* seconds */
    suseconds_t tv_usec;    /* microseconds */
};
#endif


void error(char *msg){
    perror(msg);
    exit(1);
}

uint64_t utils_get_time_millis(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

void print_bytes_msg(void *arr, char *msg, uint32_t size){
    for (int i=0; i < size; i++){
        printf("%02x ", ((uint8_t*) arr)[i]);
    }
    printf("%s", msg);
    printf("\n");
}

void print_bytes(void *arr, uint32_t size){
    for (int i=0; i < size; i++){
        printf("%02x ", ((uint8_t*) arr)[i]);
    }
    printf("\n");
}