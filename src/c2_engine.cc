#include "c2_engine.h"

#include "base/log.h"
#include "c2_utils.h"

/************* static method *************/
C2Engine *C2Engine::new_c2_engine(C2ModeType mode, C2CodecType codec_type) {
    C2Engine *engine = new C2Engine();
    if (engine == nullptr) {
        base::LogError() << "Cannot alloc memory for C2Engine";
        return nullptr;
    }

    // g_mutex_init(&engine->lock);
    // g_cond_init(&engine->workdone);
    engine->_mode = mode;

    switch (codec_type) {
        case C2CodecType::H264VideoEncode:
            engine->_name = "c2.qti.avc.encoder";
            break;
        case C2CodecType::H265VideoEncode:
            engine->_name = "c2.qti.hevc.encoder";
            break;
        case C2CodecType::HEICVideoEncode:
            engine->_name = "c2.qti.heic.encoder";
            break;
    }

    try {
        engine->_c2_module = C2Factory::GetModule(engine->_name, mode);
    } catch (std::exception &e) {
        base::LogError() << "Failed to create C2 module, error: " << e.what();
        free_c2_engine(engine);
        return nullptr;
    }

    try {
        std::shared_ptr<IC2Notifier> notifier(engine);
        engine->_c2_module->Initialize(notifier);
    } catch (std::exception &e) {
        base::LogError() << "Failed to initialize c2 engine, error: " << e.what();
        free_c2_engine(engine);
        return nullptr;
    }

    base::LogInfo() << "Created C2 engine success";
    return engine;
}

void C2Engine::free_c2_engine(C2Engine *engine) {
    delete engine;
}

/************* public method *************/
void C2Engine::EventHandler(C2EventType event, void *payload) {}

void C2Engine::FrameAvailable(std::shared_ptr<C2Buffer> &buffer, uint64_t index, uint64_t timestamp,
                              C2FrameData::flags_t flags) {}

bool C2Engine::start_c2_engine() {
    try {
        _c2_module->Start();
        base::LogDebug() << "Started c2module " << _name;
    } catch (std::exception &e) {
        base::LogError() << "Failed to start c2module, error: " << e.what();
        return false;
    }

    return true;
}

bool C2Engine::stop_c2_engine() {
    try {
        _c2_module->Stop();
        base::LogDebug() << "Stopped c2module " << _name;
    } catch (std::exception &e) {
        base::LogError() << "Failed to stop c2module, error: " << e.what();
        return false;
    }

    // Wait until all work is completed or EOS.
    // GST_C2_ENGINE_CHECK_AND_WAIT_PENDING_WORK(engine, 0);

    return true;
}

bool C2Engine::flush_c2_engine() {
    try {
        _c2_module->Flush(C2Component::FLUSH_COMPONENT);
        base::LogDebug() << "Flushed c2module " << _name;
    } catch (std::exception &e) {
        base::LogError() << "Failed to flush c2module, error: " << e.what();
        return false;
    }

    // Wait until all work is completed or EOS.
    // GST_C2_ENGINE_CHECK_AND_WAIT_PENDING_WORK(engine, 0);

    return true;
}

bool C2Engine::c2_engine_queue_buffer(C2StreamBuffer *stream_buffer) {
    std::list<std::unique_ptr<C2Param>> settings;
    std::shared_ptr<C2Buffer> c2buffer;
    uint64_t index = 0;
    uint64_t timestamp = 0;
    uint32_t flags = 0;

    std::shared_ptr<C2GraphicBlock> block;

    // no dma buffer implement
    {
        C2PixelFormat format = stream_buffer->pixel_format;

        uint32_t width = stream_buffer->width;
        uint32_t height = stream_buffer->height;
        bool isheic = false;

        std::shared_ptr<C2GraphicBlock> block;
        try {
            std::shared_ptr<C2GraphicMemory> c2_mem = _c2_module->GetGraphicMemory();
            block = c2_mem->Fetch(width, height, format, isheic);
        } catch (std::exception &e) {
            base::LogError() << "Failed to fetch memory block, error: " << e.what();
            return false;
        }
        c2buffer = C2Utils::CreateBuffer(stream_buffer, block);
    }
    try {
        _c2_module->Queue(c2buffer, settings, index, timestamp, flags);
        base::LogDebug() << "Queued buffer";
    } catch (std::exception &e) {
        base::LogError() << "Failed to queue frame, error: " << e.what();
        return false;
    }
    return true;
}

C2Engine::C2Engine() {}

C2Engine::~C2Engine() {}