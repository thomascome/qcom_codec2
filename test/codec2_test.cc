#include <iostream>
#include <string>

#include "base/log.h"
#include "src/c2_engine.h"

/// Codec2 component instance.
C2Module *c2module = nullptr;

int main(int argc, const char *argv[]) {
    C2Engine *engine =
        C2Engine::new_c2_engine(C2ModeType::VideoEncode, C2CodecType::H264VideoEncode);

    C2Engine::free_c2_engine(engine);
    base::LogInfo() << "end of main function";
    return 0;
}