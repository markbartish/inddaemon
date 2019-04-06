#ifndef SQLITE_ADAPTER_H
#define SQLITE_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sqlite3.h>

#include "modbus_slave.h"

#define BUF_SIZE 1024
#define DB_DATA_TABLE_NAME "polls"
#define DB_NAME "/home/Mark/nbproj/IndDaemon/inddaemon.db"
#define DB_OPEN_MODE (SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX)

int sa_count_stmt(sqlite3 *pDb, const char *table, const char *where_cond){
    char sql_statement[] = "SELECT COUNT(*) FROM %s";
    char buf[BUF_SIZE];
    int rc;
    int cnt = -1;
    sqlite3_stmt *pStmt;
    
    snprintf(buf, BUF_SIZE, sql_statement, table);

    if (where_cond != NULL){
        printf("There is a where condition\n");
        snprintf(buf, BUF_SIZE, "%s WHERE %s", buf, where_cond);
    }

    snprintf(buf, BUF_SIZE, "%s;", buf);
    printf("%s\n", buf);
    
    rc = sqlite3_prepare_v2(pDb, buf, strlen(buf), &pStmt, NULL);
    
    printf("prepare returned %d\n", rc);
    if (rc != SQLITE_OK){
        printf("Error: %s\n", sqlite3_errmsg(pDb));
    }
    
    while ((rc = sqlite3_step(pStmt)) == SQLITE_ROW){
        cnt = sqlite3_column_int(pStmt, 0);
        printf("%s\n", buf);
    }
    rc = sqlite3_finalize(pStmt);
    return cnt;
}
    
size_t sa_get_units_count(){
    
}

int sa_get_unit_list(sqlite3 *pDb, ModbusSlave **unit_list, size_t units_count){
    int rc;
    char sql_statement[] = "SELECT ip, port, slave_id, di_count, inverted FROM units;";
    sqlite3_stmt *pStmt;

    rc = sqlite3_prepare_v2(pDb, sql_statement, sizeof(sql_statement), &pStmt, NULL);
    
    if (rc != SQLITE_OK){
        printf("Error: %s\n", sqlite3_errmsg(pDb));
        return rc;
    }
    
    int c = 0;
    while ((rc = sqlite3_step(pStmt)) == SQLITE_ROW && c < units_count){
        const unsigned char *ip = sqlite3_column_text(pStmt, 0);      
        int port                = sqlite3_column_int(pStmt, 1);
        int slave_id            = sqlite3_column_int(pStmt, 2);
        int di_count            = sqlite3_column_int(pStmt, 3);
        bool inverted           = sqlite3_column_int(pStmt, 4);
        
        strncpy(unit_list[c]->ip, ip, MBSLAVE_IP_SIZE);
        unit_list[c]->id        = (uint8_t)  slave_id;
        unit_list[c]->port      = (uint16_t) port;
        unit_list[c]->n_of_dis  = (uint8_t) di_count;
        unit_list[c]->inverted  = (bool) inverted;

        c++;
    }
    
    rc = sqlite3_finalize(pStmt);
}

int sa_open_conn(sqlite3 **ppDb){
    int rc = sqlite3_open_v2(DB_NAME, ppDb, DB_OPEN_MODE, NULL);
    if (rc != SQLITE_OK){
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(*ppDb));
        sqlite3_close(*ppDb);
    }
    return(rc);
}
 
/**
 * 
 * @param unit_id
 * @param trans_id
 * @param start_time
 * @param result_data
 * @param exception_data
 * @return 
 */
int write_poll_result(int unit_id, 
                      uint16_t trans_id, 
                      uint64_t start_time,
                      uint64_t end_time,
                      uint16_t state_data,
                      uint16_t exception_data,
                      uint32_t socket_error_data,
                      const char *socket_error_info){
    const char column_list[] = "unit_id, trans_id, trans_init_time, trans_end_time, state_data, mb_excp_data, socket_error_data, socket_error_info";
    const char sql_statement_templ[] = "INSERT INTO %s (%s) VALUES()";
    
}
#ifdef __cplusplus
}
#endif

#endif /* SQLITE_ADAPTER_H */

