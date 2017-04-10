#include "DSVSCA.h"
#include <thread>
#include <chrono>

void print_help() {
    std::cout << "Mandatory arguments:" << std::endl
        << "-v, --video=VIDEO-FILE\t\tSpecifies the location of the video file to virtualize." << std::endl
        << "-s, --sofa=SOFA-FILE\t\tSpecifies the location of the SOFA file to use during the virtualization process." << std::endl
        << std::endl;

    std::cout << "Optional arguments:" << std::endl
        << "-h, --help\t\t\tPrints out the help menu specifying all required and optional parameters." << std::endl
        << "-b, --block-size=BLOCK-SIZE\tSpecifies the block size used when processing the audio. A smaller block size results in better virtualization but takes longer to process. The default size is " << BLOCK_SIZE << "." << std::endl
        << "-c, --coord-type=TYPE\t\tSpecifies the coordinate system used when specifying virtualized speaker placement. The values can be Cartesian or Spherical. The default value used is Cartesian." << std::endl
        << "-fl, --fl=X,Y,Z\t\t\tSpecifies the x, y, and z or phi, theta, and radius coordinates of the front left speaker. If this value is not specified, the default value of 1, 1, 0 is used." << std::endl
        << "-fc, --fc=X,Y,Z\t\t\tSpecifies the x, y, and z or phi, theta, and radius coordinates of the front center speaker. If this value is not specified, the default value of 1, 0, 0 is used." << std::endl
        << "-fr, --fr=X,Y,Z\t\t\tSpecifies the x, y, and z or phi, theta, and radius coordinates of the front right speaker. If this value is not specified, the default value of 1, -1, 0 is used." << std::endl
        << "-bl, --bl=X,Y,Z\t\t\tSpecifies the x, y, and z or phi, theta, and radius coordinates of the back left speaker. If this value is not specified, the default value of -1, 1, 0 is used." << std::endl
        << "-br, --br=X,Y,Z\t\t\tSpecifies the x, y, and z or phi, theta, and radius coordinates of the back right speaker. If this value is not specified, the default value of -1, -1, 0 is used." << std::endl
        << "-lfe, --lfe=X,Y,Z\t\tSpecifies the x, y, and z or phi, theta, and radius coordinates of the Low-Frequency Efects (subwoofer) speaker. If this value is not specified, the default value of 1, 0, 0 is used." << std::endl
        << std::endl;
}

coordinate parse_coordinates(std::string coordinates) {
    coordinate to_return;

    size_t last_pos = 0;
    size_t pos = 0;
    int current_coord = 0;
    while (current_coord < 3) {
        pos = coordinates.find(",", pos);
        std::string coord_str = pos != std::string::npos ? coordinates.substr(last_pos, pos - last_pos) : coordinates.substr(last_pos);

        int coord = atoi(coord_str.c_str());
        last_pos = pos + 1;
        if (pos + 1 < coordinates.size()) pos++;

        if (current_coord == 0) to_return.x = coord;
        else if (current_coord == 1) to_return.y = coord;
        else if (current_coord == 2) to_return.z = coord;
        current_coord++;
    }

    return to_return;
}

process_info parse_inputs(int argc, char ** argv) {
    process_info info;

    for (int i = 1; i < argc; i++) {
        std::string current_input = std::string(argv[i]);
        int index_of_eq = current_input.find("=");
        std::string arg_name;
        std::string arg_val;

        if (index_of_eq == std::string::npos) {
            if (current_input[1] != '-') arg_name = current_input.substr(1);
            else arg_name = current_input.substr(2);

            if (i + 1 < argc) arg_val = std::string(argv[++i]);
        }
        else {
            arg_name = current_input.substr(2, index_of_eq - 2);
            arg_val = current_input.substr(index_of_eq + 1);
        }

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
        else if (arg_name == "video" || arg_name == "v") {
            info.video_file_name = arg_val;
        }
        else if (arg_name == "sofa" || arg_name == "s") {
            info.sofa_file_name = arg_val;
        }
        else if (arg_name == "block-size" || arg_name == "b") {
            info.block_size = atoi(arg_val.c_str());
        }
        else if (arg_name == "coord-type" || arg_name == "c") {
            std::transform(arg_val.begin(), arg_val.end(), arg_val.begin(), ::tolower);
            if (arg_val == "cartesian") info.coord_type = Filter::Cartesian;
            else if (arg_val == "spherical") info.coord_type = Filter::Spherical;
        }
        else if (arg_name == "help" || arg_name == "h") {
            print_help();
            exit(0);
        }
    }

    return info;
}

int main(int argc, char ** argv) {
    process_info info = parse_inputs(argc, argv);

    if (info.video_file_name.empty() || info.sofa_file_name.empty()) {
        std::cout << "A required parameter was not provided." << std::endl << std::endl;
        print_help();
        return 1;
    }

    clock_t begin = clock();
    Format format(info.video_file_name);
    Filter filter(&format);
    clock_t end = clock();

    std::cout << "Filter initialization: " << (double)(end - begin) / CLOCKS_PER_SEC << " s" << std::endl;

    begin = clock();
    info.format = &format;
    info.filter = &filter;

    std::atomic_int progress(0);
    info.progress = &progress;

    std::thread worker([&info] {
        DSVSCA::process_filter_graph(info);
    });

    int progress_i = info.progress->load();
    while (progress_i < 100) {
        progress_i = info.progress->load();
        std::cout << "Progress: " << progress_i << "%\r" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    std::cout << "Progress: 100%" << std::endl << "Cleaning Up." << std::endl;
    worker.join();

    end = clock();

    std::cout << "Processing Time: " << (double)(end - begin) / CLOCKS_PER_SEC << " s" << std::endl;
    return 0;
}
