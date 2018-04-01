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



void error(char *msg)
{
    perror(msg);
    exit(1);
}


int check_stop(pthread_t tid){
    return 1;
}

int main(int argc, char** argv){
    const size_t              REQUEST_SIZE = MBAP_HEADER_SIZE + REQUEST_PDU_SIZE;
    const size_t              THREADS_NUM = 5; 
    pthread_t                 tids[UNITS_IN_USE];
    int                     * sock_fds = malloc(UNITS_IN_USE * sizeof(int));
    struct sockaddr_in     ** serv_addrs = malloc(UNITS_IN_USE * sizeof(serv_addrs));
    uint8_t                ** reqs = malloc(UNITS_IN_USE * sizeof(reqs));
    int                       ret;
    GatewayClientParameters * gwcps[UNITS_IN_USE];
    
    
    printf("Hello World\n");
    int sockfd = -1;
    char * errmsg = malloc(100 * sizeof(char));
    
    
    init_gateways();
    
    printf("sockfd before: %d\n", sockfd);
    int r = tcpcli_connect(slaves[0].ip, slaves[0].port, &sockfd, errmsg);
    
    printf("sockfd after: %d\n", sockfd);
    
    printf("got past tcpcli_connect\n");
    if (r != 0){
        printf("Something was unsuccessfull during connect attempt\n");
        printf(errmsg);
        printf("\n");
    }

    
    close(sockfd);
    
    free(errmsg);
    return 0;
    
    for (int i = 0; i < UNITS_IN_USE; i++){
        reqs[i] = malloc(REQUEST_SIZE * sizeof(uint8_t));
        serv_addrs[i] = malloc(sizeof(struct sockaddr_in));
    }
    

    //GatewayClientParameters ** gwcp = malloc(sizeof(GatewayClientParameters) * UNITS_IN_USE);
    //memset(gwcp, 0, sizeof(GatewayClientParameters) * UNITS_IN_USE);
    
    printf("Initializing units data: ");
    init_gateways();
    printf("Done!\n");
    
    //printf("Preparing gateway poll: ");
    for (int i = 0; i < UNITS_IN_USE; i++){
        //printf("Preparing gateway %d ", i);
        prepare_gateway_poll(slaves[i], serv_addrs[i], &sock_fds[i]);
        init_gwcp(POLL_INTERVAL_USEC, slaves[i], check_stop);
    }
    for (int i= 0; i < UNITS_IN_USE; i++){
        r = connect_to_gateway(*serv_addrs[i], sock_fds[i]);
    }
    
    //Construct request for each unit
    for (int i = 0; i < UNITS_IN_USE; i++){
        construct_request(0, slaves[i].id, 
                slaves[i].n_of_dis, reqs[i], REQUEST_SIZE);
        printf("Constructed request for gw %s:\n", slaves[i].ip);
        print_bytes(reqs[i], REQUEST_SIZE);
    }

    for (int iter = 0; iter < 10; iter++){
        for (int i = 0; i < UNITS_IN_USE; i++){
            break;
            uint8_t *req = reqs[i];
            uint64_t send_time;
            uint64_t recv_time;

            set_transaction_id(req, iter);
            send_time = getTimeMillis();
            printf("%s#%d:\n", slaves[i].ip, slaves[i].id);
            printf("OUT [%llu]: ", send_time);
            print_bytes(req, REQUEST_SIZE);

            // printf("Polling gateway %d: \n", i);
            uint8_t resp[UINT16_MAX];
            int resp_size = 0;
            resp_size = poll_gateway(sock_fds[i], slaves[i], req, 
                    REQUEST_SIZE, (uint8_t *) &resp, sizeof(resp));
            recv_time = getTimeMillis();    
            printf("IN: [%llu]: ", recv_time);
            if (slaves[i].inverted){
                print_bytes_msg(resp, "(inv)", resp_size);
            }
            else{
                print_bytes(resp, resp_size);
            }
            printf("Time diff %lld ms\n", recv_time - send_time);
        }
    }
    
    for (int i= 0; i < UNITS_IN_USE; i++){
        int r = close(sock_fds[i]);
    }

    
    printf("MAIN - before threads\n");
    
    for (int i = 0; i < THREADS_NUM; i++){

        printf("Starting thread %d  ", i);
        ret = pthread_create(&tids[i], NULL, gateway_client_transmit, gwcps[i]);
        printf("Done, ret code = %d\n", ret);
    }
    
    for (int i = 0; i < THREADS_NUM; i++){
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