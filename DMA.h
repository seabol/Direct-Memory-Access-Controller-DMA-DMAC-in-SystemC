#include<iomanip>
#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"
//#include "para.h"
using namespace std;
using namespace sc_core;
using namespace sc_dt;

// #define DMA_BASE_ADDR 0x4000


SC_MODULE(DMA) {

    ///* Declare in/out port *///
    sc_in<bool> clk;
    sc_in<bool> rst;
  //  sc_in<bool> clear;
    sc_out<bool> interrupt;
    unsigned int intr;

tlm_utils::simple_target_socket<DMA> socket_s;
 tlm_utils::simple_initiator_socket<DMA> socket_m; 
//tlm_utils::simple_target_socket<DMA> socket_s;
 //   tlm_utils::simple_initiator_socket<DMA> socket_m;
    ///* Declare Master port with TLM 2.0*///
    unsigned char* M_data_ptr;
    unsigned int M_addr;
    unsigned char data_m_tmp[4];

    ///* Declare Slave port with TLM 2.0*///
    tlm::tlm_command S_cmd;
    sc_signal<sc_uint<1> > S_rw;
    unsigned int S_addr;
    unsigned char *S_data;
    unsigned int S_length;
    unsigned int data;//assign to reg

    ///* Control reg and base_addr *///
    //sc_signal<sc_uint<32> > dma_base_addr;
    sc_signal<sc_uint<32> > source; // 0x0
    sc_signal<sc_uint<32> > target; // 0x4
    sc_signal<sc_uint<32> > size;   // 0x8
    sc_signal<bool> start;          // 0xc
    sc_signal<bool> clear;	    // 0x10

    ///* Reg *///
    sc_signal<sc_uint<2> > state;
    sc_signal<sc_uint<2> > n_state;
    sc_signal<sc_uint<32> > counter;
    
    void dma();
    void b_transport(tlm::tlm_generic_payload&, sc_time&);

    SC_CTOR(DMA):socket_s("socket_s"), socket_m("socket_m"){
	socket_s.register_b_transport(this, &DMA::b_transport);
        SC_CTHREAD(dma, clk.pos());
        reset_signal_is(rst, false);
    }
};
