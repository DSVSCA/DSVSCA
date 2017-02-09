#include <iostream>
#include "input/decomposer.h"
#include <libavfilter/avfilter.h>

int main(int argc, char *argv[]) {
    Decomposer *in = new Decomposer("sw.mkv");    
    
    return 0; 
}
