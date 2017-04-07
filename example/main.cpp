#include "DSVSCA.h"
#include <thread>
#include <chrono>

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

    std::atomic_int progress(0);
    process_info info;
    info.format = &format;
    info.filter = &filter;
    info.sofa_file_name = sofaFile;
    info.progress = &progress;
    if (block_size > 0) info.block_size = block_size;

    std::thread worker([&info] {
        DSVSCA::process_filter_graph(info);
    });

    int progress_i = info.progress->load();
    while (progress_i < 100) {
        printf("Progress: %d%\n", progress_i);
        progress_i = info.progress->load();
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    printf("Progress: 100%\nCleaning Up.\n");
    worker.join();

    end = clock();

    std::cout << "Processing Time: " << (double)(end - begin) / CLOCKS_PER_SEC << " s" << std::endl;
    return 0;
}
