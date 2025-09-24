/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */

#include "interrupts.hpp" // provides: parse_args, parse_trace, write_output

int main(int argc, char** argv) {

    // vectors: std::vector<std::string> of ISR addresses (index = device id)
    // delays : std::vector<int> of per-device I/O delays (ms)  (index = device id)
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    std::string trace;      //!< stores single line of trace file
    std::string execution;  //!< accumulates the execution output

    /******************ADD YOUR VARIABLES HERE*************************/

    // ---- Simulation state ----
    long long now = 0;  // simulated clock in milliseconds

    // ---- Interrupt timing constants ("simulation knobs") ----

    // Time to switch CPU from user mode to kernel mode (and back out later).
    const int T_SWITCH_MODE = 1;

    // Time to save the current process context (registers, state) when entering kernel mode.
    const int T_SAVE_CONTEXT = 10;

    // Time to compute the memory position of the interrupt vector for the device.
    // Each vector entry is 2 bytes → position = device_id * 2.
    const int T_FIND_VECTOR = 1;

    // Time to fetch (read) the ISR address from the vector table at the computed position.
    const int T_GET_ISR = 1;

    // Time to actually run the ISR body (the “service routine” code).
    // This is the heavy part that you’ll vary between 40–200 ms in experiments.
    const int T_ISR_BODY = 40;

    // Time to restore the saved process context and execute the IRET instruction,
    // returning the CPU from kernel mode back to user mode.
    const int T_IRET = 1;

    // ---- Utility helpers ----

    // Trim helper: removes leading/trailing whitespace
    auto trim = [](std::string s){
        size_t a = s.find_first_not_of(" \t\r\n");
        size_t b = s.find_last_not_of(" \t\r\n");
        if (a == std::string::npos) return std::string();
        return s.substr(a, b - a + 1);
    };

    // Each vector entry is 2 bytes, so device_id * 2 gives memory position of its vector.
    auto vector_mem_pos = [&](int dev){ return dev * 2; };

    // Logger: appends a line "start_time, duration, description" to execution string.
    auto logf = [&](long long start, int dur, const std::string& what){
        execution += std::to_string(start) + ", " + std::to_string(dur) + ", " + what + "\n";
    };

    // Handles a device interrupt (END_IO from trace)
    auto service_interrupt = [&](int dev){
        // 1) Switch to kernel
        logf(now, T_SWITCH_MODE, "switch to kernel mode");
        now += T_SWITCH_MODE;

        // 2) Save context
        logf(now, T_SAVE_CONTEXT, "save context");
        now += T_SAVE_CONTEXT;

        // 3) Find vector position
        logf(now, T_FIND_VECTOR, "find vector " + std::to_string(dev) +
                                  " at mem " + std::to_string(vector_mem_pos(dev)));
        now += T_FIND_VECTOR;

        // 4) Get ISR address
        logf(now, T_GET_ISR, "get ISR address " + vectors[dev]);
        now += T_GET_ISR;

        // 5) Execute ISR body
        logf(now, T_ISR_BODY, "execute ISR (device " + std::to_string(dev) + ")");
        now += T_ISR_BODY;

        // 6) Return to user (IRET)
        logf(now, T_IRET, "IRET");
        now += T_IRET;
    };

    // Handles a SYSCALL request: perform syscall ISR sequence (no scheduling here;
    // we rely on explicit END_IO lines in the trace for completions)
    auto start_syscall = [&](int dev){
        // Entry sequence for syscall
        logf(now, T_SWITCH_MODE, "switch to kernel mode");
        now += T_SWITCH_MODE;

        logf(now, T_SAVE_CONTEXT, "save context");
        now += T_SAVE_CONTEXT;

        logf(now, T_FIND_VECTOR, "find vector " + std::to_string(dev) +
                                  " at mem " + std::to_string(vector_mem_pos(dev)));
        now += T_FIND_VECTOR;

        logf(now, T_GET_ISR, "get ISR address " + vectors[dev]);
        now += T_GET_ISR;

        logf(now, T_ISR_BODY, "execute ISR (syscall device " + std::to_string(dev) + ")");
        now += T_ISR_BODY;

        logf(now, T_IRET, "IRET");
        now += T_IRET;
    };

    // Runs a CPU burst for D ms (no preemption when using explicit END_IO)
    auto run_cpu = [&](int D){
        logf(now, D, "CPU burst");
        now += D;
    };

    /******************************************************************/

    // Parse each line of the input trace file
    while (std::getline(input_file, trace)) {
        auto [activity, duration_intr] = parse_trace(trace);
        std::string act = trim(activity);  // robust against stray spaces

        /******************ADD YOUR SIMULATION CODE HERE*************************/

        if (act == "CPU") {
            run_cpu(duration_intr);
        } else if (act == "SYSCALL") {
            start_syscall(duration_intr);
        } else if (act == "END_IO") {
            service_interrupt(duration_intr);
        }
        // else: ignore unknown/blank lines

        /************************************************************************/
    }

    input_file.close();

    write_output(execution);

    return 0;
}
