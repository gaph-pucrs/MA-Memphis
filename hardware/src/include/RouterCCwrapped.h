#ifndef _SCGENMOD_RouterCC_
#define _SCGENMOD_RouterCC_

#include <systemc.h>

#include "standards.h"
#define ROUTER_VHD

class RouterCC : public sc_foreign_module
{
public:
    sc_in<bool> clock;
    sc_in<bool> reset;
    sc_in<regNport > rx;
    sc_in<regflit > data_in[NPORT];
    
    sc_out<regNport > credit_o;
    sc_out<regNport > tx;
    sc_out<regflit > data_out[NPORT];
    sc_in<regNport > credit_i;
    
    // RouterCC(sc_module_name nm)
    // : sc_foreign_module(nm,"RouterCC", num_generics, generic_list),
    RouterCC(sc_module_name nm, const char* hdl_name,int num_generics, const char** generic_list)
     : sc_foreign_module(nm),
       clock("clock"),
       reset("reset"),
       rx("rx"),
       credit_o("credit_o"),
       tx("tx"),
       credit_i("credit_i")
    { 
        elaborate_foreign_module(hdl_name, num_generics, generic_list);
    }
    ~RouterCC()
    {}

  // public:
  //   regaddress address;
};

#endif

