/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */

#include "interrupts.hpp"  

int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    std::string trace;      //!< stores single line of trace file
    std::string execution;  //!< accumulates the execution output

    /******************ADD YOUR VARIABLES HERE*************************/

    
    long long now = 0;  // simulated clock in ms

    // Timing constants (all in ms) 
    const int T_SWITCH_MODE = 1;   // switch to/from kernel
    const int T_SAVE_CONTEXT = 10; // save process context
    const int T_FIND_VECTOR = 1;   // compute vector memory position
    const int T_GET_ISR     = 1;   // fetch ISR address
    const int T_IRET        = 1;   // restore context and return to user

    
    auto vector_mem_pos = [&](int dev){ return dev * 2; };  // 2 bytes per vector

    // Logger funtion that appends a line "start_time, duration, description" to execution string
    auto logf = [&](long long start, int dur, const std::string& what){
        execution += std::to_string(start) + ", " + std::to_string(dur) + ", " + what + "\n";
    };

    // Handles a device interrupt request
    auto service_interrupt = [&](int dev){
        int isr_time = delays[dev];  

        logf(now, T_SWITCH_MODE, "switch to kernel mode");
        now += T_SWITCH_MODE;

        logf(now, T_SAVE_CONTEXT, "save context");
        now += T_SAVE_CONTEXT;

        logf(now, T_FIND_VECTOR, "find vector " + std::to_string(dev) +
                                  " at mem " + std::to_string(vector_mem_pos(dev)));
        now += T_FIND_VECTOR;

        logf(now, T_GET_ISR, "get ISR address " + vectors[dev]);
        now += T_GET_ISR;

        logf(now, isr_time, "execute ISR (END_IO device " + std::to_string(dev) + ")");
        now += isr_time;

        logf(now, T_IRET, "IRET");
        now += T_IRET;
    };

    // Handles a SYSCALL request
    auto handle_syscall = [&](int dev){
        int isr_time = delays[dev];  // device-specific ISR time

        logf(now, T_SWITCH_MODE, "switch to kernel mode");
        now += T_SWITCH_MODE;

        logf(now, T_SAVE_CONTEXT, "save context");
        now += T_SAVE_CONTEXT;

        logf(now, T_FIND_VECTOR, "find vector " + std::to_string(dev) +
                                  " at mem " + std::to_string(vector_mem_pos(dev)));
        now += T_FIND_VECTOR;

        logf(now, T_GET_ISR, "get ISR address " + vectors[dev]);
        now += T_GET_ISR;

        logf(now, isr_time, "execute ISR (SYSCALL device " + std::to_string(dev) + ")");
        now += isr_time;

        logf(now, T_IRET, "IRET");
        now += T_IRET;
    };

    // Runs a CPU burst for D ms 
    auto run_cpu = [&](int D){
        logf(now, D, "CPU burst");
        now += D;
    };
   /******************************************************************/
  
    //parse each line of the input trace file
    while (std::getline(input_file, trace)) {
        auto [activity, duration_intr] = parse_trace(trace);

  /******************ADD YOUR SIMULATION CODE HERE*************************/

        if (activity == "CPU") {
            run_cpu(duration_intr);
        } else if (activity == "SYSCALL") {
            handle_syscall(duration_intr);
        } else if (activity == "END_IO") {
            service_interrupt(duration_intr);
        }
 /******************************************************************/

    }

    input_file.close();
   
    write_output(execution);
    
    return 0;
}

