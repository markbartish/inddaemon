/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   tcp_client.h
 * Author: Mark
 *
 * Created on den 31 mars 2018, 14:13
 */

#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#define INDD_ERROR_COULD_NOT_INIT_SOCKET 1
#define INDD_ERROR_BAD_ADDRINFO_RETURNED 2
#define INDD_ERROR_COULD_NOT_CONNECT     4

/**
 * 
 * @param ip_addr
 * @param port
 * @param sockfd [out] on successfull conection the file descriptor
 * @param errmsg [out] error msg on error, on success it is null
 * @return 0 on success, error code on failure
 */
int tcpcli_connect(const char * ip_addr, 
                   uint16_t port, 
                   int * sockfd, 
                   char * errmsg){
    struct addrinfo hints;
    struct addrinfo *result_ai;
    struct addrinfo *rp;
    int s;
    const size_t NUMSTRBUF_SIZE = 10;
    char err[1024];
    
    printf("tcpcli_connect: addr: %s, port: %u\n", ip_addr, port);
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;       /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;           /* Any protocol */
    
    hints.ai_addrlen = 0;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;
    hints.ai_socktype = 0;

    // Represent port as string
    char port_str[NUMSTRBUF_SIZE];
    snprintf(port_str, NUMSTRBUF_SIZE, "%u", port);
    
    s = getaddrinfo(ip_addr, port_str, &hints, &result_ai);
    
    if (s != 0) {
        snprintf(err, 1024, "getaddrinfo: %s\n", gai_strerror(s));
        printf(err);
        return INDD_ERROR_BAD_ADDRINFO_RETURNED;
    }
    
    for (rp = result_ai; rp != NULL; rp = rp->ai_next){
        *sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (*sockfd == -1){
            continue;
        }

        if (connect(*sockfd, rp->ai_addr, rp->ai_addrlen) != -1){
            break;                 // Success
        }

        // If we come this far -- failure!
        snprintf(err, 1024, "Could not connect to %s:%u\n", ip_addr, port);
        strerror_r(errno, err, 1024);
        strncpy(errmsg, err, 1024);
        close(*sockfd);
        return INDD_ERROR_COULD_NOT_CONNECT;
    }    
    return 0;
}


#ifdef __cplusplus
}
#endif

#endif /* TCP_CLIENT_H */

