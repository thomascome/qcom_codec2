#include <iostream>
#include <string>

#include "c2-module.h"

/// Codec2 component instance.
C2Module *c2module = nullptr;

int main(int argc, const char *argv[]) {
    C2ModeType component_mode = C2ModeType::kVideoEncode;
    std::string name;
    //"video/x-h264"
    name = "c2.qti.avc.encoder";
    // "video/x-h265"
    // name = "c2.qti.hevc.encoder";
    // "image/heic"
    // name = "c2.qti.heic.encoder";
    try {
        c2module = C2Factory::GetModule(name, component_mode);
    } catch (std::exception &e) {
        std::cout << "Failed to create C2 module, error: " << e.what() << std::endl;
        return NULL;
    }
    delete c2module;
    std::cout << "end of main function" << std::endl;
}