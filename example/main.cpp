#include "DSVSCA.h"
#include <thread>
#include <chrono>

coordinate parse_coordinates(std::string coordinates) {
    coordinate to_return;

    size_t last_pos = 0;
    size_t pos = 0;
    int current_coord = 0;
    while ((pos = coordinates.find(",", pos)) != std::string::npos && current_coord < 3) {
        int coord = atoi(coordinates.substr(last_pos, pos - last_pos).c_str());
        last_pos = pos;

        if (current_coord == 0) to_return.x = coord;
        else if (current_coord == 1) to_return.y = coord;
        else if (current_coord == 2) to_return.z = coord;
        current_coord++;
    }

    return to_return;
}

process_info parse_inputs(int argc, char ** argv) {
    process_info info;

    std::atomic_int progress(0);
    info.progress = &progress;

    for (int i = 0; i < argc; i++) {
        std::string current_input = std::string(argv[i]);
        int index_of_eq = current_input.find("=");
        std::string arg_name = current_input.substr(2, index_of_eq - 2);
        std::string arg_val = current_input.substr(index_of_eq + 1);

        Filter::Channel channel;

        if ((channel = Filter::str_to_channel(arg_name)) != Filter::INVALID) {
            std::transform(arg_name.begin(), arg_name.end(), arg_name.begin(), ::tolower);
            coordinate coords = parse_coordinates(arg_val);
            if (arg_name == "fl") {
                info.coords.insert(std::make_pair(Filter::FL, coords));
            }
            else if (arg_name == "fc") {
                info.coords.insert(std::make_pair(Filter::FC, coords));
            }
            else if (arg_name == "fr") {
                info.coords.insert(std::make_pair(Filter::FR, coords));
            }
            else if (arg_name == "bl") {
                info.coords.insert(std::make_pair(Filter::BL, coords));
            }
            else if (arg_name == "br") {
                info.coords.insert(std::make_pair(Filter::BR, coords));
            }
            else if (arg_name == "lfe") {
                info.coords.insert(std::make_pair(Filter::LFE, coords));
            }
        }
        else if (arg_name == "video") {
            info.video_file_name = arg_val;
        }
        else if (arg_name == "sofa") {
            info.sofa_file_name = arg_val;
        }
        else if (arg_name == "block-size") {
            info.block_size = atoi(arg_val.c_str());
        }
        else if (arg_name == "coord-type") {
            std::transform(arg_val.begin(), arg_val.end(), arg_val.begin(), ::tolower);
            if (arg_val == "cartesian") info.coord_type = Filter::Cartesian;
            else if (arg_val == "spherical") info.coord_type = Filter::Spherical;
        }
    }

    return info;
}

int main(int argc, char ** argv) {
    process_info info = parse_inputs(argc, argv);

    clock_t begin = clock();
    Format format(info.video_file_name);
    Filter filter(&format);
    clock_t end = clock();

    std::cout << "Filter initialization: " << (double)(end - begin) / CLOCKS_PER_SEC << " s" << std::endl;

    begin = clock();
    info.format = &format;
    info.filter = &filter;
    DSVSCA::process_filter_graph(info);

    std::thread worker([&info] {
        DSVSCA::process_filter_graph(info);
    });

    int progress_i = info.progress->load();
    while (progress_i < 100) {
        std::cout << "Progress: " << progress_i << "%\r" << std::flush;
        progress_i = info.progress->load();
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    std::cout << "Progress: 100%" << std::endl << "Cleaning Up." << std::endl;
    worker.join();

    end = clock();

    std::cout << "Processing Time: " << (double)(end - begin) / CLOCKS_PER_SEC << " s" << std::endl;
    return 0;
}
