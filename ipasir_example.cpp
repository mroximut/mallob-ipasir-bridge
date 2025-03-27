#include <iostream>
#include <chrono>
#include <thread>
#include "ipasir.h"

// Terminate callback example
int terminate_cb(void* data) {
    bool* should_terminate = static_cast<bool*>(data);
    return *should_terminate ? 1 : 0;
}

int main() {
    std::cout << "Initializing Mallob IPASIR solver..." << std::endl;
    
    // Get signature
    std::cout << "Solver: " << ipasir_signature() << std::endl;
    
    // Initialize solver
    void* solver = ipasir_init();
    
    //Add clauses for a simple SAT formula: (x1 OR x2) AND (NOT x1 OR x3) AND (NOT x2 OR NOT x3 OR x4)
    //Clause 1: x1 OR x2
    ipasir_add(solver, 1);
    ipasir_add(solver, 2);
    ipasir_add(solver, 0);  // End of clause
    
    // Clause 2: NOT x1 OR x3
    ipasir_add(solver, -1);
    ipasir_add(solver, 3);
    ipasir_add(solver, 0);  // End of clause
    
    // Clause 3: NOT x2 OR NOT x3 OR x4
    ipasir_add(solver, -2);
    ipasir_add(solver, -3);
    ipasir_add(solver, 4);
    ipasir_add(solver, 0);  // End of clause
    
    // Add assumption: x2 must be true
    ipasir_assume(solver, 2);
    ipasir_assume(solver, 1);
    ipasir_assume(solver, -3);
    
    // Set up termination callback (optional)
    //bool should_terminate = false;
    //ipasir_set_terminate(solver, &should_terminate, terminate_cb);
    
    // Pre-submit the job to Mallob (optional, will be done in solve() if not called)
    //std::cout << "Pre-submitting job to Mallob..." << std::endl;
    //mallob_ipasir_presubmit(solver);
    
    // Small delay to allow setup
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Solve the formula
    std::cout << "Solving formula..." << std::endl;
    int result = ipasir_solve(solver);
    
    // Process the result
    if (result == 10) {
        std::cout << "SATISFIABLE" << std::endl;
        std::cout << "Variable assignments:" << std::endl;
        for (int i = 1; i <= 4; i++) {
            int value = ipasir_val(solver, i);
            std::cout << "x" << i << " = " << (value > 0 ? "true" : "false") << std::endl;
        }
    } else if (result == 20) {
        std::cout << "UNSATISFIABLE" << std::endl;
        // Check if our assumption was the reason for UNSAT
        if (ipasir_failed(solver, 2)) {
            std::cout << "Assumption x2 = true contributed to unsatisfiability" << std::endl;
        }
    } else {
        std::cout << "Unknown result: " << result << std::endl;
    }
    
    // Clean up
    std::cout << "Releasing solver..." << std::endl;
    ipasir_release(solver);
    
    return 0;
}