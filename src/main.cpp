#include <iostream>
#include <fstream>
#include <vector>
int main(int argc, char *argv[]) {
    std::ifstream input("sw.mkv", std::ios::binary | std::ios::ate);
    std::streamsize size = input.tellg();
    input.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if(input.read(buffer.data(), size)) {
        for(auto i : buffer) {
            std::cout << i;
        }
    }
    return 0; 
}
