/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
void config(){
    // Almunge/Mogavägen
    struct modbus_unit_t mgv;
    mgv.id = 11;
    mgv.inverted = 0; // true
    mgv.n_of_dis = 16;
    gateways[1].ip = "172.20.120.110";
    gateways[1].port = 502;
    gateways[1].n_of_units = 1;
    gateways[1].units[0] = mgv;
    
    // Bärby/Ärnevi
    struct modbus_unit_t aenv;
    aenv.id = 21;
    aenv.inverted = 0; // true
    aenv.n_of_dis = 16;
    gateways[2].ip = "172.20.180.111";
    gateways[2].port = 502;
    gateways[2].n_of_units = 1;
    gateways[2].units[0] = aenv;
    
    // Uppsala Ö
    struct modbus_unit_t meg;
    struct modbus_unit_t peg;
    struct modbus_unit_t flg;
    struct modbus_unit_t knd;
    struct modbus_unit_t abv; 
    meg.id = 21;
    meg.inverted = 0;
    meg.n_of_dis = 16;
    peg.id = 31;
    peg.inverted = 1;
    peg.n_of_dis = 16;
    flg.id = 41;
    flg.inverted = 0;
    flg.n_of_dis = 16;
    knd.id = 51;
    knd.inverted = 0;
    knd.n_of_dis = 16;
    abv.id = 71;
    abv.inverted = 0;
    abv.n_of_dis = 16;
    
    gateways[3].ip = "172.20.180.111";
    gateways[3].port = 502;
    gateways[3].n_of_units = 1;
    gateways[3].units[0] = meg;
    gateways[3].units[1] = peg;
    gateways[3].units[2] = peg;
    gateways[3].units[3] = peg;
    gateways[3].units[4] = peg;
    
    uint8_t num_of_gateways = 4;
}
 */