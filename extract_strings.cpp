#include <iostream>

#include "scsp_cpp.hpp"

class MyHandler : public scsp::CallbacksEmpty {
    virtual void string_chunk(const uint8_t* buf, size_t len) {
        std::cout.write((const char*)buf, len);
    }
    virtual void string_close() {
        std::cout << std::endl;
    }
};

int main() {
    // Note; on Windows you should also set std::cin to binary mode
    std::ios::sync_with_stdio(false);
    
    MyHandler mh;
    if (!scsp::parse_from_istream(std::cin, mh)) {
        std::cerr << "Error" << std::endl;
        return 1;
    }
    return 0;
}
