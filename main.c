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
#include <signal.h>

#include "modbus.h"
#include "gateway_client_thread.h"
#include "utils.h"
#include "tcp_client.h"
#include "modbus_client.h"
#include "sqlite_adapter.h"

#define POLL_INTERVAL_MICROSEC  1000000

#define DB_NAME "/home/Mark/nbproj/IndDaemon/inddaemon_one.db"

int interrupt = 0;

void int_handler(int signo)
{
    if (signo == SIGINT){   
        interrupt = 1;
    }
}

int main(int argc, char** argv){
    int              rc;
    int            * sock_fds;
    size_t           units_count;
    ModbusSlave    * mbslaves;
    char             errmsg[BUF_SIZE];
    int              r = -1;
    char             dbfile_name[BUF_SIZE];
    sqlite3        * pDb;

    signal(SIGINT, int_handler);
    
    if (argc > 1){
        strncpy(dbfile_name, argv[1], BUF_SIZE);
        printf("Database file name given.\n");
        printf("Will use '%s' as db\n", dbfile_name);
    }
    else{
        strncpy(dbfile_name, DB_NAME, BUF_SIZE);
    }
    
    rc = sa_open_conn(dbfile_name, &pDb);
    if (rc != SQLITE_OK){
        fprintf(stderr, "Error: Could not load dbfile: %s\n", dbfile_name);
        fprintf(stderr, "%s\n", sqlite3_errmsg(pDb));
        exit(1);
    }
    
    rc = sa_load_and_init_units(pDb, &mbslaves, 
                                &units_count);
    if (rc != SQLITE_OK){
        fprintf(stderr, "Error: Could not load units\n");
        fprintf(stderr, "%s\n", sqlite3_errstr(rc));
        exit(rc);
    }
    
    sock_fds = malloc(units_count * sizeof(int*));
    printf("After sa_get_unit_list\n");
    for (int i = 0; i < units_count; i++){
        printf("mbslaves[%d].db_id = %u\n",    i, mbslaves[i].db_id);
        printf("mbslaves[%d].ip = %s\n",       i, mbslaves[i].ip);
        printf("mbslaves[%d].id = %d\n",       i, mbslaves[i].id);
        printf("mbslaves[%d].port = %d\n",     i, mbslaves[i].port);
        printf("mbslaves[%d].n_of_dis = %d\n", i, mbslaves[i].n_of_dis);
    }
    
    // Make connections for all units
    for (int i = 0; i < units_count; i++){sock_fds[i] = 0;}
    for (int i = 0; i < units_count; i++){
        r = tcpcli_connect(mbslaves[i].ip, mbslaves[i].port, &sock_fds[i], errmsg);
        if (r != 0){
            fprintf(stderr, "Could not connect to %s:%d because:\n", mbslaves[i].ip, mbslaves[i].port);
            fprintf(stderr, "   %s\n", errmsg);
            continue;
        }
    }
    
    int pollcount = 100000;
    uint16_t trans_id = 0;
    uint16_t recv_trid = 0;
    uint16_t state = 0;
    uint16_t exception = 0;
    uint64_t send_time = 0;
    uint64_t receive_time = 0;
    sqlite3_stmt * trans_start_insert_stmt;
    sqlite3_stmt * trans_end_update_stmt;
    
    
    rc = sa_insert_poll_trans_send_stmt(pDb, &trans_start_insert_stmt);
    if (rc != SQLITE_OK){
        fprintf(stderr, "Error: %s\n", sqlite3_errstr(rc));
        fprintf(stderr, "rc = %d\n", rc);
        exit(rc);
    }
    rc = sa_update_poll_trans_receive_stmt(pDb, &trans_end_update_stmt);
    if (rc != SQLITE_OK){
        fprintf(stderr, "Error: %s\n", sqlite3_errstr(rc));
        fprintf(stderr, "rc = %d\n", rc);
        exit(rc);
    }

    for (int p = 0; p < pollcount; p++){
        if (interrupt){
            printf("Interrupt request detected (SIGINT). Quiting...\n");
            break;
        }
        for (int i = 0; i < units_count; i++){
            if (sock_fds[i] == 0){ continue; }
            trans_id = (uint16_t) p % (UINT16_MAX + 1);
            state = 0;
            exception = 0;
            printf("Connected to %s:%d successfully!\n", mbslaves[i].ip, mbslaves[i].port);
            send_time = utils_get_time_millis();
            sa_insert_poll_trans_send(trans_start_insert_stmt, mbslaves[i].db_id, trans_id, send_time);
            
            mbcli_get_unit_state(sock_fds[i], trans_id, mbslaves[i].id, mbslaves[i].n_of_dis,
                                     &state, &exception, &recv_trid);
            
            receive_time = utils_get_time_millis();
            printf ("state = 0x%04x, exception = 0x%04x\n", state, exception);
            sa_update_poll_trans_receive(trans_end_update_stmt, 
                                        mbslaves[i].db_id,
                                        recv_trid,
                                        send_time, 
                                        receive_time, 
                                        state, exception);
        }
        usleep(POLL_INTERVAL_MICROSEC);
    }
 
    printf("Finalizing all statements... ");
    sqlite3_finalize(trans_end_update_stmt);
    sqlite3_finalize(trans_start_insert_stmt);
    printf("Done!\n");
    printf("Closing all sockets... ");
    for (int i = 0; i < units_count; i++){
        close(sock_fds[i]);
        sock_fds[i] = 0;
    }
    printf("Done!\n");
    printf("Deallocating units memory... ");
    free(mbslaves);
    free(sock_fds);
    printf("Done!\n");
    printf("Closing db conn... ");
    rc = sqlite3_close(pDb);
    printf("Done!\n");
}

void loop(){
    for(;;){
        
    }
}