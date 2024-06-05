
#pragma once

#include <stdint.h>

#include "c2_module.h"

class C2Engine : public IC2Notifier {
public:
    /**
     * @brief Initialize instance of Codec2 engine.
     * @param mode: The Codec2 component work mode.
     * @param codec_type: work mode detail codec type.
     * @param callbacks: Engine callback functions which will be called when an event
     *             occurs or an encoded/decoded output buffer is produced.
     * @param userdata: Private user defined data which will be attached to the callbacks.
     * @return : Pointer to Codec2 engine on success or NULL on failure.
     */
    static C2Engine *new_c2_engine(C2ModeType mode, C2CodecType codec_type);
    /**
     * @brief Deinitialise and free the Codec2 engine instance.
     * @engine: Pointer to Codec2 engine.
     * @return: NONE
     */
    static void free_c2_engine(C2Engine *engine);
public:
    virtual void EventHandler(C2EventType event, void *payload);
    virtual void FrameAvailable(std::shared_ptr<C2Buffer> &buffer, uint64_t index,
                                uint64_t timestamp, C2FrameData::flags_t flags);
public:
    C2Engine();
    ~C2Engine();
private:
    /// Component name, used mainly for debugging.
    std::string _name;
    /// Codec2 component instance.
    C2Module *_c2_module;
    /// Component mode/type: Encode or Decode.
    C2ModeType _mode;

    // /// Draining state & pending frames lock.
    // GMutex lock;
    // /// Condition signalled when pending frame has been processed.
    // GCond workdone;
    /// Tracking the number of pending frames.
    uint32_t _pending;
};