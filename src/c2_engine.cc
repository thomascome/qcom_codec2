#include "c2_engine.h"

#include "base/log.h"

C2Engine *C2Engine::new_c2_engine(C2ModeType mode, C2CodecType codec_type) {
    C2Engine *engine = new C2Engine();
    if (engine == nullptr) {
        return nullptr;
    }

    // g_mutex_init(&engine->lock);
    // g_cond_init(&engine->workdone);
    engine->_mode = mode;

    std::string name = "";
    switch (codec_type) {
        case C2CodecType::H264VideoEncode:
            name = "c2.qti.avc.encoder";
            break;
        case C2CodecType::H265VideoEncode:
            name = "c2.qti.hevc.encoder";
            break;
        case C2CodecType::HEICVideoEncode:
            name = "c2.qti.heic.encoder";
            break;
    }

    try {
        engine->_c2_module = C2Factory::GetModule(name, mode);
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

    return engine;
}

void C2Engine::free_c2_engine(C2Engine *engine) {
    delete engine;
}

void C2Engine::EventHandler(C2EventType event, void *payload) {}

void C2Engine::FrameAvailable(std::shared_ptr<C2Buffer> &buffer, uint64_t index, uint64_t timestamp,
                              C2FrameData::flags_t flags) {}

C2Engine::C2Engine() {}

C2Engine::~C2Engine() {}