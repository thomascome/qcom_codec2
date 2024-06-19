#include <unistd.h>

#include <iostream>
#include <string>

#include "base/log.h"
#include "src/c2_common.h"
#include "src/c2_engine.h"

/// Codec2 component instance.
C2Module *c2module = nullptr;

int main(int argc, const char *argv[]) {
    FILE *fp = fopen("sample.yuv", "rb");
    if (fp == NULL) {
        base::LogError() << "cannot open sample yuv";
        return 1;
    }
    int width = 1920;
    int height = 1080;
    int buffer_size = width * height * 3 / 2;
    uint8_t *mem_buffer = (uint8_t *)malloc(buffer_size);
    int read_size = fread(mem_buffer, buffer_size, 0, fp);
    fclose(fp);

    C2Engine *engine =
        C2Engine::new_c2_engine(C2ModeType::VideoEncode, C2CodecType::H264VideoEncode);
    engine->start_c2_engine();

    C2StreamBuffer stream_buffer;
    stream_buffer.data = mem_buffer;
    stream_buffer.size = buffer_size;
    stream_buffer.width = width;
    stream_buffer.height = height;
    stream_buffer.offset[0] = 0;
    stream_buffer.offset[1] = width * height;
    stream_buffer.stride[0] = width;
    stream_buffer.stride[1] = width;
    stream_buffer.planes = 2;
    stream_buffer.pixel_format = C2PixelFormat::kNV12;
    stream_buffer.isubwc = false;

    for (int i = 0; i < 300; i++) {
        engine->c2_engine_queue_buffer(&stream_buffer);
        usleep(33000);
    }
    while (true)
        ;
    free(mem_buffer);
    engine->stop_c2_engine();
    C2Engine::free_c2_engine(engine);
    base::LogInfo() << "end of main function";
    return 0;
}