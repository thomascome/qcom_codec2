#include <iostream>
#include <string>

#include "base/log.h"
#include "src/c2_common.h"
#include "src/c2_engine.h"

/// Codec2 component instance.
C2Module *c2module = nullptr;

int main(int argc, const char *argv[]) {
    C2Engine *engine =
        C2Engine::new_c2_engine(C2ModeType::VideoEncode, C2CodecType::H264VideoEncode);

    engine->start_c2_engine();
    FILE *fp = fopen("sample.yuv", "rb");
    if (fp == NULL) {
        base::LogError() << "cannot open sample yuv";
        return 1;
    }
    int buffer_size = 1920 * 1080 * 3 / 2;
    uint8_t *mem_buffer = (uint8_t *)malloc(buffer_size);
    fread(mem_buffer, buffer_size, 0, fp);
    C2StreamBuffer stream_buffer;
    stream_buffer.data = mem_buffer;
    stream_buffer.size = buffer_size;
    stream_buffer.width = 1920;
    stream_buffer.height = 1080;
    stream_buffer.offset[0] = 0;
    stream_buffer.offset[1] = 1920 * 1080;
    stream_buffer.stride[0] = 1920;
    stream_buffer.stride[1] = 1920 / 2;
    stream_buffer.planes = 2;
    stream_buffer.pixel_format = C2PixelFormat::kNV12;
    stream_buffer.isubwc = false;

    engine->c2_engine_queue_buffer(&stream_buffer);

    free(mem_buffer);
    fclose(fp);
    engine->stop_c2_engine();
    C2Engine::free_c2_engine(engine);
    base::LogInfo() << "end of main function";
    return 0;
}