CREATE TABLE units (
    id INT PRIMARY KEY, 
    ip VARCHAR(255), 
    port INT, 
    slave_id INT, 
    di_count SMALLINT,
    inverted TINYINT -- Boolean 0 = false, non-0 = true
);
