/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   debug.h
 * Author: Mark
 *
 * Created on den 20 mars 2018, 21:57
 */

#ifndef DEBUG_H
#define DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
    
    void dprintf(const char *fmt, ...){
        va_list args;
        //va_start(args, fmt);
        printf(fmt, args);
        //va_end(args);
    }


#ifdef __cplusplus
}
#endif

#endif /* DEBUG_H */

