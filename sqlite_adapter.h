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
    /*
    if (where_cond != NULL){
        printf("There is a where condition\n");
        snprintf(buf, BUF_SIZE, " WHERE %s", where_cond);
    }
    */
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

int sa_get_unit_list(sqlite3 *pDb, ModbusSlaveLinkedList *unit_list){
    int rc;
    char sql_statement[] = "SELECT * FROM units;";
    sqlite3_stmt *pStmt;

    rc = sqlite3_prepare_v2(pDb, sql_statement, sizeof(sql_statement), &pStmt, NULL);
    
    ModbusSlaveLinkedList *prev = NULL;
    while ((rc = sqlite3_step(pStmt)) == SQLITE_ROW){
        //printf("executed sqlite3_step, rc = %d\n", rc);
        int id = sqlite3_column_int(pStmt, 0);
        unsigned char *ip = sqlite3_column_text(pStmt, 1);
        //printf("id: %d\n", id);
        //printf("ip: %s\n", ip);
        
        unit_list->slave = malloc(sizeof(ModbusSlave));
        unit_list->slave->id = id;
        size_t ip_size = strlen(ip);
        unit_list->slave->ip = malloc(ip_size);
        strncpy(unit_list->slave->ip, ip, ip_size);
        
        if (prev != NULL){ prev->next = unit_list->slave; }
        unit_list->next = NULL;
        prev = unit_list->slave;
    }
    
    rc = sqlite3_finalize(pStmt);
    
    printf("inside get_unit_list\n");
}

    
int sa_open_conn(sqlite3 **ppDb){
    int rc = sqlite3_open_v2(DB_NAME, ppDb, DB_OPEN_MODE, NULL);
    if(rc != SQLITE_OK){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*ppDb));
        sqlite3_close(*ppDb);
    }
    return(rc);
}


int callback(void *argp, int argc, char **argv, char **azColName){
    printf("Callback speaking\n");
    printf("argc = %d\n", argc);
    for (int i = 0; i < argc; i++){
        printf("argv[%d] = %s\n", i, argv[i]);
        printf("azColName[%d] = %s\n", i, azColName[i]); 
        printf("\n");
    }
    return 0;
}

int opendb(){
    sqlite3 *db;
    char *zErrMsg = NULL;
    int rc;
    sqlite3_stmt *ppStmt;
    const char *pzTail;
    rc = sqlite3_open(DB_NAME, &db);
    
    printf("opendb speaking\n");
    
    if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return(1);
    }    
    
    const char sql_statement[] = "SELECT * FROM units;";
    
    //rc = sqlite3_exec(db, sql_statement, callback, NULL, &zErrMsg);
    
    rc = sqlite3_prepare_v2(db, sql_statement, sizeof(sql_statement), &ppStmt, &pzTail);
    printf("executed sqlite3_prepare_v2, rc = %d\n", rc);
    printf("pzTail = %s\n", pzTail);
    
    while ( (rc = sqlite3_step(ppStmt)) == SQLITE_ROW){
        printf("executed sqlite3_step, rc = %d\n", rc);
        printf("Past sqlite3_exec\n");
        int v = sqlite3_column_int(ppStmt, 0);
        printf("column 0 value = %d\n", v);
    }
    
    rc = sqlite3_finalize(ppStmt);
    
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);
    
    return 0;
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
    //const char values[] = concat_poll_values();
    const char sql_statement_templ[] = "INSERT INTO %s (%s) VALUES()";
    
}


// Private helpers

static char *concat_poll_values(int unit_id, 
                                const char *trans_id,
                                const char *something_else){
    
}

#ifdef __cplusplus
}
#endif

#endif /* SQLITE_ADAPTER_H */

