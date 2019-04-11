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
#define DB_OPEN_MODE (SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX)

typedef struct data_kvp_t{
    char * column;
    void * value;
} DataKvp;
    
static int sa_write_poll_trans_common(sqlite3 * pDb, 
                               const char sql_statement[], 
                               size_t stmt_len);



int sa_open_conn(const char *db_filename, sqlite3 **ppDb){
    int rc = sqlite3_open_v2(db_filename, ppDb, DB_OPEN_MODE, NULL);
    if (rc != SQLITE_OK){
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(*ppDb));
        sqlite3_close(*ppDb);
    }
    return(rc);
}

int sa_count_stmt(sqlite3    * pDb, 
                  const char * table, 
                  const char * where_cond, 
                  size_t     * count){
    char           sql_statement_templ[] = "SELECT COUNT(*) FROM %s %s;";
    char           sql_statement[BUF_SIZE];
    int            rc;
    sqlite3_stmt * pStmt;

    snprintf(sql_statement, BUF_SIZE, sql_statement_templ, 
             table, where_cond == NULL ? "" : where_cond);
    printf("Count statement:\n");
    printf("'%s'\n", sql_statement);
    rc = sqlite3_prepare_v2(pDb, sql_statement, strlen(sql_statement), &pStmt, NULL);
    if (rc != SQLITE_OK){
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(pDb));
        return rc;
    }
    
    while ((rc = sqlite3_step(pStmt)) == SQLITE_ROW){
        *count = sqlite3_column_int(pStmt, 0);
    }
    rc = sqlite3_finalize(pStmt);
    return rc;
}

int sa_insert(sqlite3 *pDb, char table_name[], DataKvp values[]){
    
}

int sa_update(sqlite3 *pDb, char table_name[], DataKvp values[], char where_clause[]){
    
}

int sa_get_unit_list(sqlite3       * pDb,
                     ModbusSlave  ** units_list, 
                     const size_t    units_count){
    int             rc = 0;
    char            sql_statement[BUF_SIZE];
    sqlite3_stmt  * pStmt;

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
    
    if (rc != SQLITE_DONE){
        return rc;
    }
    
    rc = sqlite3_finalize(pStmt);
    return rc;
}

int sa_load_and_init_units(sqlite3      * pDb,
                           ModbusSlave ** units_list,
                           size_t       * units_count){
    int           rc;
    
    rc = sa_count_stmt(pDb, "units", NULL, units_count);
    if (rc != SQLITE_OK){
        fprintf(stderr, "Error executing units count query\n");
        return rc;
    }
    printf("units_count = %u\n", *units_count);

    if (*units_count == -1){
        fprintf(stderr, "Something went wrong counting slaves, quitting.\n");
        exit(1);
    }
    
    rc = sa_get_unit_list(pDb, units_list, *units_count);
    printf("got past sa_get_unit_list, rc = %d\n", rc);
    printf("units_list = 0x%08x\n", units_list);

    for (int i = 0; i < *units_count; i++){
        printf("%s, %d, %d, %d, %d\n", (*units_list)[i].ip, 
                                       (*units_list)[i].id, 
                                       (*units_list)[i].port,
                                       (*units_list)[i].n_of_dis,
                                       (*units_list)[i].inverted);   
    }
    return rc;
}

int sa_insert_poll_trans_send_stmt(sqlite3 * pDb, sqlite3_stmt  ** ppStmt){
    int rc;
    const char      sql_statement_templ[] = "INSERT INTO %s (%s) VALUES(%s);";
    char            sql_statement[BUF_SIZE];
    const char      values_list[]         = "?, ?, ?"; 
    const char      column_list[]         = "unit_id, trans_id, trans_init_time";
    
    snprintf(sql_statement, BUF_SIZE, sql_statement_templ, DB_POLLS_TABLE_NAME, column_list, values_list);
    rc = sqlite3_prepare_v2(pDb, sql_statement, sizeof(sql_statement), ppStmt, NULL);
    
    if (rc != SQLITE_OK){
        fprintf(stderr, "Could not prepare SQL statement\n");
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(pDb));
        sqlite3_finalize(*ppStmt);
    }
    return rc;
}

/*
 * To be called when starting a modbus transaction.
 * Will issue an INSERT statement.
 */
int sa_insert_poll_trans_send(sqlite3_stmt       * pStmt_trans_start,
                         const uint16_t       unit_db_id,
                         const uint16_t       trans_id,
                         const uint64_t       trans_init_time){
    int             rc;
    
    if ((rc = sqlite3_bind_int(pStmt_trans_start, 1, unit_db_id)) != SQLITE_OK){
        fprintf(stderr, "Could not bind int\n");
        fprintf(stderr, "%s\n", sqlite3_errstr(rc));
        return rc;
    }
    
    if ((rc = sqlite3_bind_int(pStmt_trans_start, 2, trans_id)) != SQLITE_OK){
        fprintf(stderr, "Could not bind int\n");
        fprintf(stderr, "%s\n", sqlite3_errstr(rc));
        return rc;
    }
    
    if ((rc = sqlite3_bind_int64(pStmt_trans_start, 3, trans_init_time)) != SQLITE_OK){
        fprintf(stderr, "Could not bind int64\n");
        fprintf(stderr, "%s\n", sqlite3_errstr(rc));
        return rc;
    }
    rc = sqlite3_step(pStmt_trans_start);
    if (rc != SQLITE_DONE){
        fprintf(stderr, "Could not perform INSERT\n");
        fprintf(stderr, "Error: %s\n", sqlite3_errstr(rc));
        fprintf(stderr, "rc = %d\n", rc);
    }
    return sqlite3_reset(pStmt_trans_start);
}

int sa_update_poll_trans_receive_stmt(sqlite3 * pDb, sqlite3_stmt  ** ppStmt){
    int             rc;
    const char      update_values_list[]         = 
    "trans_end_time = ?, state_data = ?, mb_excp_data = ?"; 
    const char      condition_values_list[]      = 
    "unit_id = ? AND trans_id = ? AND trans_init_time = ?";
    const char      sql_statement_templ[] = "UPDATE %s SET %s WHERE %s;";
    char            sql_statement[BUF_SIZE];
    
    snprintf(sql_statement, BUF_SIZE, sql_statement_templ, DB_POLLS_TABLE_NAME, 
            update_values_list, condition_values_list);
    printf("UPDATE statement:\n");
    printf("%s\n", sql_statement);
    rc = sqlite3_prepare_v2(pDb, sql_statement, sizeof(sql_statement), ppStmt, NULL);
    if (rc != SQLITE_OK){
        fprintf(stderr, "Could not create prepared statement for receive UPDATE\n");
        fprintf(stderr, "%s\n", sqlite3_errstr(rc));        
    }
    return rc;
}

/*
 * To be called when sending receiving a response to a poll request
 * Will issue an UPDATE statement
 */
int sa_update_poll_trans_receive(sqlite3_stmt      * pStmt_trans_receive,
                                const uint16_t       unit_id,
                                const uint16_t       trans_id,
                                const uint64_t       trans_init_time,
                                const uint64_t       trans_end_time,
                                const uint16_t       state,
                                const uint16_t       excp_data){
    int rc;

    if ((rc = sqlite3_bind_int64(pStmt_trans_receive, 1, (sqlite_int64) trans_end_time)) != SQLITE_OK){
        fprintf(stderr, "Could not bind int\n");
        fprintf(stderr, "%s\n", sqlite3_errstr(rc));
        return rc;
    }
    if ((rc = sqlite3_bind_int(pStmt_trans_receive, 2, state)) != SQLITE_OK){
        fprintf(stderr, "Could not bind int\n");
        fprintf(stderr, "%s\n", sqlite3_errstr(rc));
        return rc;
    }
    if ((rc = sqlite3_bind_int(pStmt_trans_receive, 3, excp_data)) != SQLITE_OK){
        fprintf(stderr, "Could not bind int64\n");
        fprintf(stderr, "%s\n", sqlite3_errstr(rc));
        return rc;
    }
    if ((rc = sqlite3_bind_int(pStmt_trans_receive, 4, unit_id)) != SQLITE_OK){
        fprintf(stderr, "Could not bind int\n");
        fprintf(stderr, "%s\n", sqlite3_errstr(rc));
        return rc;
    }
    if ((rc = sqlite3_bind_int(pStmt_trans_receive, 5, trans_id)) != SQLITE_OK){
        fprintf(stderr, "Could not bind int\n");
        fprintf(stderr, "%s\n", sqlite3_errstr(rc));
        return rc;
    }
    if ((rc = sqlite3_bind_int64(pStmt_trans_receive, 6, (sqlite_int64) trans_init_time)) != SQLITE_OK){
        fprintf(stderr, "Could not bind int64\n");
        fprintf(stderr, "%s\n", sqlite3_errstr(rc));
        return rc;
    } 
    rc = sqlite3_step(pStmt_trans_receive);
    if (rc != SQLITE_DONE){
        fprintf(stderr, "Could not perform UPDATE\n");
        fprintf(stderr, "Error: %s\n", sqlite3_errstr(rc));
        fprintf(stderr, "rc = %d\n", rc);
    }

    return sqlite3_reset(pStmt_trans_receive);
}

#ifdef __cplusplus
}
#endif

#endif /* SQLITE_ADAPTER_H */

