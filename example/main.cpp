#include "DSVSCA.h"

int main(int argc, char *argv[]) {
    std::string videoFile = std::string(argv[1]);
    std::string sofaFile = std::string(argv[2]);
    int block_size = 0;
    if (argc > 3) block_size = atoi(argv[3]);

    clock_t begin = clock();
    Format format(videoFile);
    Filter filter(&format);
    clock_t end = clock();

    std::cout << "Filter initialization: " << (double)(end - begin) / CLOCKS_PER_SEC << " s" << std::endl;

    begin = clock();
    process_info info;
    info.format = &format;
    info.filter = &filter;
    info.sofa_file_name = sofaFile;
    if (block_size > 0) info.block_size = block_size;
    DSVSCA::process_filter_graph(info);
    end = clock();

    std::cout << "Processing Time: " << (double)(end - begin) / CLOCKS_PER_SEC << " s" << std::endl;
    return 0;
}
