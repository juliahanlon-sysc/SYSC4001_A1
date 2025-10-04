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

    
    long long now = 0;  // clock in ms

    // Timing constants (ms)
    const int SWITCH_MODE = 1;   
    const int SAVE_CONTEXT = 10; 
    const int IRET        = 1;

    // Logger: "start_time, duration, description"
    auto logf = [&](long long start, int dur, const std::string& what){
        execution += std::to_string(start) + ", " + std::to_string(dur) + ", " + what + "\n";
    };

    // Runs a CPU burst for D ms 
    auto run_cpu = [&](int D){
        logf(now, D, "CPU burst");
        now += D;
    };

    // Handles a device interrupt request 
    auto service_interrupt = [&](int dev){
        int isr_time = delays[dev];

       
        auto bp = intr_boilerplate(static_cast<int>(now), dev, SAVE_CONTEXT, vectors);
        execution += bp.first;     
        now = bp.second;           // update time

        // Execute device-specific ISR activity, then return to user
        logf(now, isr_time, "execute ISR (END_IO device " + std::to_string(dev) + ")");
        now += isr_time;

        logf(now, IRET, "IRET");
        now += IRET;
    };

    // Handles a SYSCALL request
    auto handle_syscall = [&](int dev){
        int isr_time = delays[dev];

        auto bp = intr_boilerplate(static_cast<int>(now), dev, SAVE_CONTEXT, vectors);
        execution += bp.first;
        now = bp.second;

        logf(now, isr_time, "execute ISR (SYSCALL device " + std::to_string(dev) + ")");
        now += isr_time;

        logf(now, IRET, "IRET");
        now += IRET;
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

