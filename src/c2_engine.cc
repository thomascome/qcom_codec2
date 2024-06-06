#include "c2_engine.h"

#include <unistd.h>

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
void C2Engine::EventHandler(C2EventType event, void *payload) {
    base::LogDebug() << "callback event handle : " << (int)event;
}

FILE *fp = fopen("out.264", "wb");
char buf[2048];

void C2Engine::FrameAvailable(std::shared_ptr<C2Buffer> &c2buffer, uint64_t index,
                              uint64_t timestamp, C2FrameData::flags_t flags) {
    base::LogDebug() << "callback frame available";
    uint32_t fd = 0;
    uint32_t size = 0;
    if (c2buffer->data().type() == C2BufferData::LINEAR) {
        const C2ConstLinearBlock block = c2buffer->data().linearBlocks().front();
        const C2Handle *handle = block.handle();

        size = block.size();
        C2ReadView view = block.map().get();
        memcpy(buf, view.data(), size);
        base::LogDebug() << "C2BufferData type linear : " << size;
        fwrite(buf, size, 1, fp);
        fflush(fp);
    } else if (c2buffer->data().type() == C2BufferData::GRAPHIC) {
        const C2ConstGraphicBlock block = c2buffer->data().graphicBlocks().front();
        auto handle = static_cast<const android::C2HandleGBM *>(block.handle());

        size = handle->mInts.size;
        fd = handle->mFds.buffer_fd;
        base::LogDebug() << "C2BufferData type graphic : " << size;
    }

    // Check whether this is a key/sync frame.
    // error: ‘C2Param::CoreIndex::<unnamed enum> C2Param::CoreIndex::IS_STREAM_FLAG’ is protected within this context
    // std::shared_ptr<const C2Info> c2info =
    //     c2buffer->getInfo(C2StreamPictureTypeInfo::output::PARAM_TYPE);
    // auto pictype = std::static_pointer_cast<const C2StreamPictureTypeInfo::output>(c2info);

    // if (pictype && (pictype->value == C2Config::SYNC_FRAME)) {
    //     base::LogDebug() << "picture value is sync frame";
    // }

    // if (flags & C2FrameData::FLAG_CODEC_CONFIG) {
    //     base::LogDebug() << "GST_BUFFER_FLAG_HEADER";
    // }

    // if (flags & C2FrameData::FLAG_DROP_FRAME) {
    //     base::LogDebug() << "GST_BUFFER_FLAG_DROPPABLE";
    // }

    // if (!(flags & C2FrameData::FLAG_INCOMPLETE)) {
    //     base::LogDebug() << "GST_BUFFER_FLAG_MARKER";
    // }
}

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