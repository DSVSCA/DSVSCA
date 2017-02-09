#include <iostream>
#include <vector>
#include <fstream>
#include <libavfilter/avfilter.h>


#ifndef DECOMPOSER_H
#define DECOMPOSER_H

class Decomposer {

private:
    std::vector<char> buffer;

    static AVFilterGraph *filter_graph = NULL;
    static AVFilterContext *abuffer_ctx = NULL;
    
    static AVFrame *oframe = NULL;
public:
    Decomposer(std::string fileName);
    
    std::vector<char> *getBuffer();

};

#endif
