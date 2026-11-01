#include <iostream>
#include "MmeSuite.h"
#include "MmeKickSuite.h"

int main() {
    std::cout << "=================================================================" << std::endl;
    std::cout << "  Multi-Machine Engines (MME) & ElektronTrackWrapper Unit Tests" << std::endl;
    std::cout << "=================================================================" << std::endl;
    try {
        test_mme::run_all_mme_suite_tests();
        std::cout << "\n-----------------------------------------------------------------" << std::endl;
        std::cout << "  MME-BD (Multi-Machine Bass Drum) 16-Model Verification Suite" << std::endl;
        std::cout << "-----------------------------------------------------------------" << std::endl;
        test_mme_kick::run_all_mme_kick_tests();
        std::cout << "\n>>> ALL MME, ELEKTRON TRACK & MME-BD TESTS PASSED 100%! <<<" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "MME Test Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "MME Test Unknown Exception!" << std::endl;
        return 2;
    }
}
