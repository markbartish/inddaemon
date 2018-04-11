CREATE TABLE polls (
    id INTEGER PRIMARY KEY, 
    unit_id INTEGER,
    trans_id INTEGER,
    trans_init_time INTEGER,
    trans_end_time INTEGER,
    state_data INTEGER,
    mb_excp_data INTEGER,
    socket_error_data INTEGER,
    socket_error_info TEXT,
    FOREIGN KEY(unit_id) REFERENCES units(id)
);