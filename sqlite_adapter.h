#ifndef SQLITE_ADAPTER_H
#define SQLITE_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sqlite3.h>

#include "modbus_slave.h"

#define BUF_SIZE 1024
#define DB_POLLS_TABLE_NAME "polls"
#define DB_UNITS_TABLE_NAME "units"
#define DB_NAME "/home/Mark/nbproj/IndDaemon/inddaemon.db"
#define DB_OPEN_MODE (SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX)

typedef struct data_kvp_t{
    char * column;
    void * value;
} DataKvp;
    
static int sa_write_poll_trans_common(sqlite3 * pDb, 
                               const char sql_statement[], 
                               size_t stmt_len);



int sa_open_conn(sqlite3 **ppDb){
    int rc = sqlite3_open_v2(DB_NAME, ppDb, DB_OPEN_MODE, NULL);
    if (rc != SQLITE_OK){
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(*ppDb));
        sqlite3_close(*ppDb);
    }
    return(rc);
}

int sa_count_stmt(const char *table, const char *where_cond){
    char           sql_statement[] = "SELECT COUNT(*) FROM %s";
    char           buf[BUF_SIZE];
    int            rc;
    int            cnt = -1;
    sqlite3      * pDb;
    sqlite3_stmt * pStmt;
    
    sa_open_conn(&pDb);
    snprintf(buf, BUF_SIZE, sql_statement, table);
    if (where_cond != NULL){
        printf("There is a where condition\n");
        snprintf(buf, BUF_SIZE, "%s WHERE %s", buf, where_cond);
    }
    snprintf(buf, BUF_SIZE, "%s;", buf);
    rc = sqlite3_prepare_v2(pDb, buf, strlen(buf), &pStmt, NULL);
    if (rc != SQLITE_OK){
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(pDb));
    }
    
    while ((rc = sqlite3_step(pStmt)) == SQLITE_ROW){
        cnt = sqlite3_column_int(pStmt, 0);
    }
    rc = sqlite3_finalize(pStmt);
    rc = sqlite3_close(pDb);
    return cnt;
}

int sa_insert(sqlite3 *pDb, char table_name[], DataKvp values[]){
    
}

int sa_update(sqlite3 *pDb, char table_name[], DataKvp values[], char where_clause[]){
    
}

int sa_get_unit_list(ModbusSlave  ** units_list, 
                     const size_t    units_count){
    int rc;
    char          sql_statement[BUF_SIZE];
    sqlite3       * pDb;
    sqlite3_stmt  * pStmt;

    sa_open_conn(&pDb);
    snprintf(sql_statement, BUF_SIZE, "SELECT id, ip, port, slave_id, di_count, inverted FROM %s;", DB_UNITS_TABLE_NAME);
    rc = sqlite3_prepare_v2(pDb, sql_statement, sizeof(sql_statement), &pStmt, NULL);
    
    if (rc != SQLITE_OK){
        printf("Error: %s\n", sqlite3_errmsg(pDb));
        sqlite3_finalize(pStmt);
        return rc;
    }
    
    *units_list = (ModbusSlave *) malloc(units_count * sizeof(ModbusSlave));
    printf("allocated for %u x %u bytes\n", units_count, sizeof(ModbusSlave));
    printf("allocated unit_list at 0x%08x\n", *units_list);
    
    int c = 0;
    while ((rc = sqlite3_step(pStmt)) == SQLITE_ROW && c < units_count){
        uint16_t db_id             = (uint16_t) sqlite3_column_int(pStmt, 0);
        const unsigned char *ip = sqlite3_column_text(pStmt, 1);      
        int port                = sqlite3_column_int(pStmt, 2);
        int slave_id            = sqlite3_column_int(pStmt, 3);
        int di_count            = sqlite3_column_int(pStmt, 4);
        bool inverted           = sqlite3_column_int(pStmt, 5);
        
        (*units_list)[c].db_id     = db_id;
        strncpy((*units_list)[c].ip, ip, MBSLAVE_IP_SIZE);
        (*units_list)[c].id        = (uint8_t)  slave_id;
        (*units_list)[c].port      = (uint16_t) port;
        (*units_list)[c].n_of_dis  = (uint8_t) di_count;
        (*units_list)[c].inverted  = (bool) inverted;

        printf("%u, %s, %d, %d, %d, %d\n",  (*units_list)[c].db_id,
                                       (*units_list)[c].ip, 
                                       (*units_list)[c].id, 
                                       (*units_list)[c].port,
                                       (*units_list)[c].n_of_dis,
                                       (*units_list)[c].inverted);

        
        c++;
    }
    
    sqlite3_finalize(pStmt);
    rc = sqlite3_close(pDb);
    if (rc != SQLITE_OK){
        fprintf(stderr, "Error: could not close db, rc = %d\n", rc);
    }
    return rc;
}

int sa_load_and_init_units(ModbusSlave ** units_list, 
                           sqlite3    *** dbconns_for_slaves, 
                           size_t       * units_count){
    int           rc;

    printf("sa_open_conn called, rc = %d\n", rc);
    *units_count = sa_count_stmt("units", NULL);
    printf("units_count = %u\n", *units_count);

    if (*units_count == -1){
        fprintf(stderr, "Something went wrong counting slaves, quitting.\n");
        exit(1);
    }
    
    rc = sa_get_unit_list(units_list, *units_count);
    printf("got past sa_get_unit_list, rc = %d\n", rc);
    printf("units_list = 0x%08x\n", units_list);

    for (int i = 0; i < *units_count; i++){
        printf("%s, %d, %d, %d, %d\n", (*units_list)[i].ip, 
                                       (*units_list)[i].id, 
                                       (*units_list)[i].port,
                                       (*units_list)[i].n_of_dis,
                                       (*units_list)[i].inverted);   
    }
    
    *dbconns_for_slaves = (sqlite3 **) malloc(*units_count * sizeof(sqlite3*));
    printf("dbconns_for_slaves = 0x%08x\n", *dbconns_for_slaves);

    for (int i = 0; i < *units_count; i++){
        rc = sa_open_conn(&(*dbconns_for_slaves)[i]);
        if (rc != SQLITE_OK){
            fprintf(stderr, 
                    "Could not open sqlite-connection for slave %d\nExiting\n", i);
            exit(1);
        }
        printf("Opening sqlite-conn for slave %d -- OK!\n", i);
    }
}

/*
 * To be called when starting a modbus transaction.
 * Will issue an INSERT statement.
 */
int sa_create_poll_trans(sqlite3            * pDb,
                         const uint16_t       unit_db_id,
                         const uint16_t       trans_id,
                         sqlite_int64       * poll_id){
    const char      column_list[]         = "unit_id, trans_id";
    const char      values_list_templ[]   = "%u, %u";
    const char      sql_statement_templ[] = "INSERT INTO %s (%s) VALUES(%s);";
    char            values_list[BUF_SIZE];
    char            sql_statement[BUF_SIZE];
    sqlite3_stmt  * pStmt;
    int             rc;
    
    snprintf(values_list, BUF_SIZE, values_list_templ, unit_db_id, trans_id);
    snprintf(sql_statement, BUF_SIZE, sql_statement_templ, DB_POLLS_TABLE_NAME, column_list, values_list);
    printf("sa_create_poll_trans: sql_statement: %s\n", sql_statement);
    rc = sqlite3_prepare_v2(pDb, sql_statement, sizeof(sql_statement), &pStmt, NULL);
    if (rc != SQLITE_OK){
        fprintf(stderr, "Could not prepare SQL statement\n");
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(pDb));
        sqlite3_finalize(pStmt);
        return rc;
    }
    rc = sqlite3_step(pStmt);
    if (rc != SQLITE_DONE){
        fprintf(stderr, "Could not perform INSERT\n");
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(pDb));
        fprintf(stderr, "rc = %d\n", rc);
    }
    *poll_id = sqlite3_last_insert_rowid(pDb);
    sqlite3_finalize(pStmt);
    return rc;
}

/*
 * To be called when sending out a poll request.
 * Will issue an UPDATE statement.
 */
int sa_write_poll_trans_send(sqlite3            * pDb,
                             const sqlite_int64   poll_id,
                             const uint16_t       trans_id,
                             const uint64_t       start_time){
    const char      sql_statement_templ[] = 
    "UPDATE %s SET trans_init_time = %llu WHERE id = %lld;";
    char            sql_statement[BUF_SIZE];

    snprintf(sql_statement, BUF_SIZE, sql_statement_templ, DB_POLLS_TABLE_NAME,
             start_time, poll_id);
    return sa_write_poll_trans_common(pDb, sql_statement, BUF_SIZE);
}

/*
 * To be called when sending receiving a response to a poll request
 * Will issue an UPDATE statement
 */
int sa_write_poll_trans_receive(sqlite3            * pDb,
                                const uint32_t       poll_id,
                                const uint16_t       trans_id,
                                const uint64_t       end_time,
                                const uint16_t       state,
                                const uint16_t       excp_data){
    const char      sql_statement_templ[] = 
    "UPDATE %s SET trans_end_time = %llu, state_data = %s, mb_excp_data = %u WHERE id = %lld;";
    char            sql_statement[BUF_SIZE];
    char            state_str[BUF_SIZE] = "NULL";
    
    if (excp_data == 0){
        snprintf(state_str, BUF_SIZE, "%u", state);
    }
    
    snprintf(sql_statement, BUF_SIZE, sql_statement_templ, DB_POLLS_TABLE_NAME,
             end_time, state_str, excp_data, poll_id);
    return sa_write_poll_trans_common(pDb, sql_statement, BUF_SIZE);
}

static int sa_write_poll_trans_common(sqlite3 * pDb, 
                               const char sql_statement[], 
                               size_t stmt_len){
    int             rc;
    sqlite3_stmt  * pStmt;
    
    printf("sa_write_poll_trans_common: sql_statement: %s\n", sql_statement);
    rc = sqlite3_prepare_v2(pDb, sql_statement, stmt_len, &pStmt, NULL);
    if (rc != SQLITE_OK){
        fprintf(stderr, "Could not prepare SQL statement\n");
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(pDb));
        sqlite3_finalize(pStmt);
        return rc;
    }
    rc = sqlite3_step(pStmt);
    if (rc != SQLITE_DONE){
        fprintf(stderr, "Could not perform INSERT\n");
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(pDb));
        fprintf(stderr, "rc = %d\n", rc);
    }    
    sqlite3_finalize(pStmt);
    return rc;
}

#ifdef __cplusplus
}
#endif

#endif /* SQLITE_ADAPTER_H */

