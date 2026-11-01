#include <iostream>
#include "EmbeddedConcurrencySuite.h"

int main() {
    std::cout << "Starting Standalone Embedded Concurrency & HAL Host Unit Tests..." << std::endl;
    test_embedded::run_all_embedded_concurrency_tests();
    std::cout << "All Embedded Concurrency & HAL Host Unit Tests passed successfully!" << std::endl;
    return 0;
}
