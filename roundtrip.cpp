#include <iostream>

#include "scsp_cpp.hpp"

int main() {
    // Note; on Windows you should also set std::cin to binary mode
    std::ios::sync_with_stdio(false);
    
    scsp::Generator gen(std::cout);
    if (!scsp::parse_from_istream(std::cin, gen)) {
        std::cerr << "Error" << std::endl;
        return 1;
    }
    return 0;
}
