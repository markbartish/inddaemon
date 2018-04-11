CREATE TABLE unit_di_portmap (
    id INTEGER PRIMARY KEY,
    unit_id INTEGER,
    di_port TINYINT,
    di_port_inverted TINYINT, --Boolean
    di_port_label VARCHAR(100),
    FOREIGN KEY(unit_id) REFERENCES units(id)
);