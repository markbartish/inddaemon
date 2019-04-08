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
    //const size_t              REQUEST_SIZE = MBAP_HEADER_SIZE + REQUEST_PDU_SIZE;
    int              rc;
    int            * sock_fds;
    size_t           units_count;
    ModbusSlave    * mbslaves;
    sqlite3       ** dbconns_for_units;
    char             errmsg[BUF_SIZE];
    int              r = -1;
    sqlite3_int64    poll_id = -1;
    
    sa_load_and_init_units(&mbslaves, &dbconns_for_units, &units_count);
    
    sock_fds = malloc(units_count * sizeof(int*));
    printf("After sa_get_unit_list\n");
    for (int i = 0; i < units_count; i++){
        printf("mbslaves[%d].db_id = %u\n",       i, mbslaves[i].db_id);
        printf("mbslaves[%d].ip = %s\n",       i, mbslaves[i].ip);
        printf("mbslaves[%d].id = %d\n",       i, mbslaves[i].id);
        printf("mbslaves[%d].port = %d\n",     i, mbslaves[i].port);
        printf("mbslaves[%d].n_of_dis = %d\n", i, mbslaves[i].n_of_dis);
    }

    for (int i = 0; i < units_count; i++){
        r = tcpcli_connect(mbslaves[i].ip, mbslaves[i].port, &sock_fds[i], errmsg);
        if (r != 0){
            printf("Could not connect to %s:%d because:\n", mbslaves[i].ip, mbslaves[i].port);
            printf("   %s\n", errmsg);
            continue;
        }
        uint16_t trans_id = 1;
        uint16_t state;
        uint16_t exception;
        printf("Connected to %s:%d successfully!\n", mbslaves[i].ip, mbslaves[i].port);
        sa_create_poll_trans(dbconns_for_units[i], mbslaves[i].db_id, trans_id, &poll_id);
        uint64_t send_time = utils_get_time_millis();
        sa_write_poll_trans_send(dbconns_for_units[i], poll_id, trans_id, send_time);
        mbcli_get_unit_state(sock_fds[i], trans_id, mbslaves[i].id, mbslaves[i].n_of_dis,
                                 &state, &exception);
        uint64_t receive_time = utils_get_time_millis();
        printf ("state = 0x%04x, exception = 0x%04x\n", state, exception);
        sa_write_poll_trans_receive(dbconns_for_units[i], 
                                    poll_id, trans_id, 
                                    receive_time, 
                                    state, exception);
    }
 
    printf("got past tcpcli_connect\n");


    for (int i = 0; i < units_count; i++){
        close(sock_fds[i]);
    }
    printf("Closed all sockets!\n");
    free(mbslaves);
    
    //Close all db connections
    for (int i = 0; i < units_count; i++){
        sqlite3_close(dbconns_for_units[i]);
    }
    free(dbconns_for_units);
    printf("sqlite3_close called, rc=%d\n", rc);

}

void loop(){
    for(;;){
        
    }
}