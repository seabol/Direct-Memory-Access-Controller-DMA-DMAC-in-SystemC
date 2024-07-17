#include "DMA.h"

// Define FSM for dma working
#define SEND_ADDR_TO_SOURCE 0
#define GET_DATA_FROM_SOURCE 1
#define WRITE_DATA_TO_TARGET 2

#define DEBUG 1
void DMA::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay){
    S_cmd = trans.get_command();
    S_addr = trans.get_address();
    S_data = trans.get_data_ptr();
    S_length = trans.get_data_length();

    #ifdef DEBUG
    cout << "---------------------------------------\n";
    cout << "DMA slave port receive data:\n";
    cout << setw(10) << setfill(' ') << sc_time_stamp() << "ns\n";
    //cout << "cmd: " << S_cmd << endl;
    cout << "addr: " << std::hex << S_addr << endl;
    //cout << "data_ptr: " << std::hex << (reinterpret_cast<unsigned int*> (S_data)) << endl;
    cout << "data: " << std::hex << *(reinterpret_cast<unsigned int*> (S_data)) << endl;
    //cout << "length: " << std::dec << S_length << endl;
    cout << "--------------------------------------\n";
    #endif

    if(S_cmd == tlm::TLM_WRITE_COMMAND){
        if(S_length == 4){
            data = *(reinterpret_cast<int*> (S_data));
            S_rw = 1;
        }
        else{
            S_rw = 0;
            cout << "Write dma control reg error!\n";
        }
    }
    else{
        cout << "You can't read control reg in dma!\n";
    }
    wait(10, SC_NS);
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
}

void DMA::dma(){


    ///* Reset *///
    
    //* Reset output port *//
    interrupt = 0;

    //* Reset register *//
    //dma_base_addr = DMA_BASE_ADDR;
    source = 0;
    target = 0;
    size = 0;
    start = 0;
    state = 0;
    n_state = 0;
    counter = 0;

    ///* Function start *///
    while(1){


                //* Clear *//
        if(clear){
            //* Clear output port *//

            interrupt.write(0);

            //* Clear register *//
            source = 0;
            target = 0;
            size = 0;
            start = 0;
            state = 0;
            n_state = 0;
            counter = 0;
	    intr = 0;
	    clear = 0;
#ifdef DEBUG
	printf("In clear stage!\n");
#endif
        }
        //* Set control reg from cpu (Using Slave port) *//
        else if (!start && intr != 1){
		//printf("fuck\n");
            /* Slave port read from cpu*/
            if(S_rw.read()){
                switch (S_addr)
                {
                case 0x0:
                    source = data;
		#ifdef DEBUG
		    printf("---------Write control reg \"source\":  %d\n", data);
		#endif
                    break;
                case 0x4:
                    target = data;
		#ifdef DEBUG
		    printf("---------Write control reg \"target\":  %d\n", data);
		#endif
                    break;
                case 0x8:
                    size = data;
			
		#ifdef DEBUG
		    printf("---------Write control reg \"size\":  %d\n", data);
		#endif
                    break;
                case 0xc:
                    start = data; 
		#ifdef DEBUG
		    printf("---------Write control reg \"start\":  %d\n", data);
		#endif
                    break;
                default:
                    #ifdef DEBUG
                    cout << "Dma Read wrong sddress from slave port when reading from slave port!\n";
                    #endif
                    break;

                }
            }
            /* Slave port write to cpu (No use in this project)*/
            ///dma doesn't use slave port to write
            else{
            }
            #ifdef DEBUG
                cout << "-----------------------------------------------------\n";
                cout << "Dma control regs:\n";
                cout << "Address: " << S_addr << endl;
                cout << "Data: " << std::hex <<data << endl;
                cout << "Dma control reg:\n";
                cout << "source: " << std::hex << source << endl;
                cout << "target: " << std::hex << target << endl;
                cout << "size: " << std::dec << size << endl;
                cout << "start: " << start << endl;

            #endif
        }
        //* Dma start working and using master port *//
        else{
            tlm::tlm_command M_cmd;
            tlm::tlm_generic_payload* M_trans = new tlm::tlm_generic_payload;
            sc_time delay = sc_time(5, SC_NS);
            state = n_state;

            /* Data havn't been sent all out */
            if(counter < size){
                
                /* FSM */
                switch (n_state.read()) //output and n_state logic are not combinatial
                {
                case SEND_ADDR_TO_SOURCE:
                    M_cmd = tlm::TLM_READ_COMMAND;
                    M_addr = source.read() + counter.read();

                    #ifdef DEBUG
                    cout << "--------------------------------\n";
                    cout << "Dma Read through master port:\n";
                    //cout << "cmd: " << M_cmd << endl;
                    cout << "address: " << std::hex << M_addr << endl;
                    cout << "data_ptr: " << std::hex << reinterpret_cast<unsigned int*>(data_m_tmp) << endl;
                    cout << "state: 0\n";
                    // cout << "data_ptr: " << std:: hex << M_data_ptr << endl;

                    #endif

		    // for homework print
		    cout << "--------------------------------\n";
		    cout << "Dma Move data:\n";
                    cout << "Source address: " << std::hex << M_addr << endl;



                    M_trans->set_command(M_cmd);
                    M_trans->set_address(M_addr);
                    M_trans->set_data_ptr(data_m_tmp);
                    M_trans->set_data_length(4);
                    M_trans->set_streaming_width(4);
                    M_trans->set_byte_enable_ptr(0);
                    M_trans->set_dmi_allowed(false);
                    M_trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
                    socket_m->b_transport(*M_trans, delay);

                    n_state = WRITE_DATA_TO_TARGET;
                    break;
                case WRITE_DATA_TO_TARGET:
                    M_cmd = tlm::TLM_WRITE_COMMAND;
                    M_addr = target.read() + counter.read();

                    #ifdef DEBUG
                    cout << "--------------------------------\n";
                    cout << "Dma write throughmaster port:\n";
                    //cout << "cmd: " << M_cmd << endl;
                    cout << "address: " << std::hex << M_addr << endl;
                    cout << "state: 1\n";
                    #endif

		    // for homework print
                    cout << "Target address: " << std::hex << M_addr << endl;
                    cout << "Data: " << std::hex << *(reinterpret_cast<unsigned int*>(data_m_tmp)) << endl;

                    M_data_ptr = data_m_tmp;
                    M_trans->set_command(M_cmd);
                    M_trans->set_address(M_addr);
                    M_trans->set_data_ptr(M_data_ptr);

                    //* Manage boundary case*//
                    if(size.read() - counter.read() < 4){
                        M_trans->set_data_length(size.read() - counter.read());
                        M_trans->set_streaming_width(size.read() - counter.read());
                    }
                    else{
                        M_trans->set_data_length(4);
                        M_trans->set_streaming_width(4);
                    }
                    M_trans->set_byte_enable_ptr(0);
                    M_trans->set_dmi_allowed(false);
                    M_trans->set_response_status(
                        tlm::TLM_INCOMPLETE_RESPONSE);
                    socket_m->b_transport(*M_trans, delay);

                    n_state = SEND_ADDR_TO_SOURCE;
                    counter = counter.read() + 4;

                    //cout << "DMA:\n";
                    //cout << "Move data from" << "MEM[" << ((source.read() + counter.read()) ^ MEMORY_BASE_ADDR) << "] to MEM["
                     //       << ((target.read() + counter.read()) ^ MEMORY_BASE_ADDR) << "]\n";

                    break;
                default:
                    #ifdef DEBUG
                    cout << "FSM ERROR when dma working!\n";
                    #endif
                    break;
                }
            }
            /* Data have been sent all out */
	        else if(intr == 1){
	        	if(S_rw.read()){
		    	if(S_addr == 0xc && data == 0){
		    		clear = 1;
		    		intr = 0;
		    		interrupt.write(0);
		    		//cout << "set clear\n";

		    	}
		    	else{
		    		//cout << "Not set clear\n";
		    	}
    
		    }

	        }
            else{
		    //cout << "DMA done!\n";
                    start = 0;
                    intr = 1;
		    interrupt.write(intr);
		    //cout << "Before!!!!!!!!!!!\n";

		    //cout << "After!!!!!!!!!!!\n";

            }

        }
    
    	    wait();
    }


}
