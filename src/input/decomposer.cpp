#include "decomposer.h"


Decomposer::Decomposer(std::string fileName) {
    std::ifstream is(fileName, std::ios::binary | std::ios::ate);
    std::streamsize size = is.tellg();
    is.seekg(0, std::ios::beg);
    
    buffer.resize(size);
    
    if(is.read(buffer.data(), size)) {
        std::cout << "Buffer is loaded!" << std::endl;
    }

}
