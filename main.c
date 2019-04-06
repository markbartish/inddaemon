/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: Mark
 *
 * Created on den 23 februari 2018, 21:42
 */

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
#include "gateway_client_thread.h"
#include "utils.h"
#include "config.h"
#include "tcp_client.h"
#include "modbus_client.h"
#include "sqlite_adapter.h"

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

int check_stop(pthread_t tid){
    return 1;
}

int main(int argc, char** argv){
    const size_t              REQUEST_SIZE = MBAP_HEADER_SIZE + REQUEST_PDU_SIZE;
    pthread_t                 tids[UNITS_IN_USE];
    int                     * sock_fds;
    struct sockaddr_in     ** serv_addrs = malloc(UNITS_IN_USE * sizeof(serv_addrs));
    uint8_t                ** reqs = malloc(UNITS_IN_USE * sizeof(reqs));
    int                       rc, ret;
    GatewayClientParameters * gwcps[UNITS_IN_USE];
    sqlite3                 * db;
    sqlite3                ** dbconns_for_slaves;
    ModbusSlave            ** mbslaves;
    int                       slaves_count;
    char                    * errmsg = malloc(1024);
    
    
    rc = sa_open_conn(&db);
    printf("sa_open_conn called, rc = %d\n", rc);
    slaves_count = sa_count_stmt(db, "units", NULL);
    printf("slaves_count = %d\n", slaves_count);
    //printf("id: %d, ip: %s\n", unit_list.slave->id, unit_list.slave->ip);
    
    if (slaves_count == -1){
        fprintf(stderr, "Something went wrong counting slaves, quitting.\n");
        exit(1);
    }
    
    dbconns_for_slaves = malloc(slaves_count * sizeof(dbconns_for_slaves));
    
    for (int i = 0; i < slaves_count; i++){
        rc = sa_open_conn(&dbconns_for_slaves[i]);
        if (rc != SQLITE_OK){
            fprintf(stderr, 
                    "Could not open sqlite-connection for slave %d\nExiting\n", i);
            exit(1);
        }
        printf("Opening sqlite-conn for slave %d -- OK!\n", i);
    }
    
    sock_fds = malloc(slaves_count * sizeof(int*));
    
    mbslaves = malloc(slaves_count * sizeof(ModbusSlave **));
    for (int i = 0; i < slaves_count; i++){
        mbslaves[i] = malloc(sizeof(ModbusSlave));
    }
    sa_get_unit_list(db, mbslaves, slaves_count);
    printf("After sa_get_unit_list\n");
    for (int i = 0; i < slaves_count; i++){
        printf("mbslaves[%d]->ip = %s\n",       i, mbslaves[i]->ip);
        printf("mbslaves[%d]->id = %d\n",       i, mbslaves[i]->id);
        printf("mbslaves[%d]->port = %d\n",     i, mbslaves[i]->port);
        printf("mbslaves[%d]->n_of_dis = %d\n", i, mbslaves[i]->n_of_dis);
    }

    int r = -1;
    for (int i = 0; i < slaves_count; i++){
        r = tcpcli_connect(mbslaves[i]->ip, mbslaves[i]->port, &sock_fds[i], errmsg);
        if (r != 0){
            printf("Could not connect to %s:%d because:\n", mbslaves[i]->ip, mbslaves[i]->port);
            printf("   %s\n", errmsg);
            continue;
        }
        printf("Connected to %s:%d successfully!\n", mbslaves[i]->ip, mbslaves[i]->port);
    }
    
    
    printf("got past tcpcli_connect\n");
    if (r != 0){
        printf("Something was unsuccessfull during connect attempt\n");
        printf("r = %d", r);
        printf(errmsg);
        printf("\n");
    }

    for (int i = 0; i < slaves_count; i++){
        close(sock_fds[i]);
    }
    
    for (int i = 0; i < slaves_count; i++){
        free(mbslaves[i]);
    }
    free(mbslaves);
    
    free(errmsg);

    rc = sqlite3_close(db);
    
    printf("sqlite3_close called, rc=%d\n", rc);

}