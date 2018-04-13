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
    int                     * sock_fds = malloc(UNITS_IN_USE * sizeof(int));
    struct sockaddr_in     ** serv_addrs = malloc(UNITS_IN_USE * sizeof(serv_addrs));
    uint8_t                ** reqs = malloc(UNITS_IN_USE * sizeof(reqs));
    int                       rc, ret;
    GatewayClientParameters * gwcps[UNITS_IN_USE];
    sqlite3                 * db;
    ModbusSlave            ** mbslaves;
    int                       slaves_count;
    char                    * errmsg = malloc(100);
    
    rc = sa_open_conn(&db);
    printf("sa_open_conn called, rc=%d\n", rc);
    slaves_count = sa_count_stmt(db, "units", NULL);
    printf("slaves_count = %d\n", slaves_count);
    //printf("id: %d, ip: %s\n", unit_list.slave->id, unit_list.slave->ip);
    
    if (slaves_count == -1){
        fprintf(stderr, "Something went wrong counting slaves, quitting.\n");
        exit(1);
    }
    
    mbslaves = malloc(slaves_count * sizeof(ModbusSlave));
    
    int sockfd = -1;
    
    mbslaves = malloc(slaves_count);
    for (int i = 0; i < slaves_count; i++){
        mbslaves[i] = malloc(sizeof(ModbusSlave));
    }
    
    sa_get_unit_list(db, mbslaves, slaves_count);
    
    printf("After sa_get_unit_list\n");
    
    for (int i = 0; i < slaves_count; i++){
        printf("mbslaves[i]->id = %d\n", mbslaves[i]->id);
        printf("mbslaves[i]->port = %d\n", mbslaves[i]->port);
        printf("mbslaves[i]->ip = %s\n", mbslaves[i]->ip);
        printf("mbslaves[i]->ip = %s\n", mbslaves[i]->id);
        printf("mbslaves[i]->ip = %s\n", mbslaves[i]->ip);
    }

    
    printf("sockfd before: %d\n", sockfd);
    int r = tcpcli_connect(mbslaves[0]->ip, mbslaves[0]->port, &sockfd, &errmsg);
    
    printf("sockfd after: %d\n", sockfd);
    
    return 0;
    
    printf("got past tcpcli_connect\n");
    if (r != 0){
        printf("Something was unsuccessfull during connect attempt\n");
        printf("r = %d", r);
        printf(errmsg);
        printf("\n");
    }

    close(sockfd);
    
    for (int i = 0; i < slaves_count; i++){
        free(mbslaves[i]);
    }
    free(mbslaves);
    
    free(errmsg);
    return 0;
    
    for (int i = 0; i < UNITS_IN_USE; i++){
        reqs[i] = malloc(REQUEST_SIZE * sizeof(uint8_t));
        serv_addrs[i] = malloc(sizeof(struct sockaddr_in));
    }

    //GatewayClientParameters ** gwcp = malloc(sizeof(GatewayClientParameters) * UNITS_IN_USE);
    //memset(gwcp, 0, sizeof(GatewayClientParameters) * UNITS_IN_USE);
    
    //Construct request for each unit
    for (int i = 0; i < UNITS_IN_USE; i++){
        construct_request(0, slaves[i].id, 
                slaves[i].n_of_dis, reqs[i], REQUEST_SIZE);
        printf("Constructed request for gw %s:\n", slaves[i].ip);
        print_bytes(reqs[i], REQUEST_SIZE);
    }
    
    for (int i= 0; i < UNITS_IN_USE; i++){
        int r = close(sock_fds[i]);
    }
    
    for (int i = 0; i < slaves_count; i++){
        printf("Starting thread %d  ", i);
        ret = pthread_create(&tids[i], NULL, gateway_client_transmit, gwcps[i]);
        printf("Done, ret code = %d\n", ret);
    }
    
    for (int i = 0; i < slaves_count; i++){
        printf("Waiting for thread %d to end ", i);
        pthread_join(tids[i], NULL);
        printf("Done\n");
    }
    printf("MAIN - after threads\n");
    
    
    printf("freeing reqs array \n");
    for (int i = 0; i < UNITS_IN_USE; i++){
        free(reqs[i]);
    }
    printf("done \n");
    
    
    rc = sqlite3_close(db);
    
    printf("sqlite3_close called, rc=%d\n", rc);
    
    printf("freeing reqs \n");
    free(reqs);
    printf("done \n");
    //free(gwcp);
    printf("freeing sock_fds  \n");
    free(sock_fds);
    printf("done \n");
    printf("freeing serv_addrs array \n");
    for (int i = 0; i < UNITS_IN_USE; i++){
        free(serv_addrs[i]);
    }
    printf("done \n");
    printf("freeing serv_addrs \n");
    free(serv_addrs);
    printf("done \n");
}