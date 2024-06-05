/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *
 *     * Neither the name of Qualcomm Innovation Center, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __GST_C2_COMPONENT_H__
#define __GST_C2_COMPONENT_H__

#include <C2Buffer.h>
#include <C2Component.h>
#include <C2Config.h>
#include <QC2ComponentStoreFactory.h>

#include <atomic>
#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <queue>
#include <tuple>

#include "c2_common.h"

#if defined(ENABLE_AUDIO_PLUGINS)
#include <codec2/QC2Buffer.h>
#endif  // ENABLE_AUDIO_PLUGINS

/** IC2Notifier
 *
 * Interface class used by the module for informing when an event occurs or
 * a Codec2 output buffer is available from processing.
 **/
class IC2Notifier {
public:
    virtual ~IC2Notifier(){};

    virtual void EventHandler(C2EventType event, void *payload) = 0;
    virtual void FrameAvailable(std::shared_ptr<C2Buffer> &buffer, uint64_t index,
                                uint64_t timestamp, C2FrameData::flags_t flags) = 0;
};

/** C2LinearMemory
 *
 * Convenient wrapper class on top of a linear C2BlockPool for allocating
 * linear blocks of memory.
 **/
class C2LinearMemory {
public:
    C2LinearMemory(std::shared_ptr<C2BlockPool> pool) : pool_(pool){};
    ~C2LinearMemory(){};

    uint64_t GetLocalId() { return pool_->getLocalId(); }

    std::shared_ptr<C2LinearBlock> Fetch(uint32_t size);
private:
    std::shared_ptr<C2BlockPool> pool_;
};

/** C2GraphicMemory
 *
 * Convenient wrapper class on top of a graphic C2BlockPool for allocating
 * graphic blocks of memory.
 **/
class C2GraphicMemory {
public:
    C2GraphicMemory(std::shared_ptr<C2BlockPool> pool) : pool_(pool){};
    ~C2GraphicMemory(){};

    uint64_t GetLocalId() { return pool_->getLocalId(); }

    std::shared_ptr<C2GraphicBlock> Fetch(uint32_t width, uint32_t height, C2PixelFormat format,
                                          bool isheic);
private:
    std::shared_ptr<C2BlockPool> pool_;
};

/** C2Module
 *
 * A light abstraction class on top of the Codec2 component providing
 * convinient APIs for interaction with the underlying component and management
 * of the submitted work.
 **/
class C2Module {
public:
    C2Module(std::shared_ptr<C2Component> &component, C2ModeType mode);
    ~C2Module();

    c2_status_t Initialize(std::shared_ptr<IC2Notifier> &notifier);

    std::shared_ptr<C2GraphicMemory> GetGraphicMemory();
    std::shared_ptr<C2LinearMemory> GetLinearMemory();
#if defined(ENABLE_AUDIO_PLUGINS)
    std::shared_ptr<qc2audio::QC2BufferCirclePools> GetLinearCirclePool(uint32_t size);
#endif  // ENABLE_AUDIO_PLUGINS

    std::unique_ptr<C2Param> QueryParam(C2Param::Index index);
    c2_status_t SetParam(std::unique_ptr<C2Param> &param);

    c2_status_t Start();
    c2_status_t Stop();

    c2_status_t Flush(C2Component::flush_mode_t mode);
    c2_status_t Drain(C2Component::drain_mode_t mode);

    c2_status_t Queue(std::shared_ptr<C2Buffer> &buffer,
                      std::list<std::unique_ptr<C2Param>> &settings, uint64_t index,
                      uint64_t timestamp, uint32_t flags);

    // TODO Make them protected/private.
    void HandleWorkDone(std::list<std::unique_ptr<C2Work>> work);
    void HandleTripped(std::vector<std::shared_ptr<C2SettingResult>> results);
    void HandleError(uint32_t error);
private:
    enum class State : uint32_t {
        kCreated,
        kIdle,
        kRunning,
    };

    std::shared_ptr<C2Component> component_;
    std::shared_ptr<C2ComponentInterface> interface_;
    std::atomic<State> state_;

    std::shared_ptr<IC2Notifier> notifier_;

    std::shared_ptr<C2GraphicMemory> graphic_mem_;
    std::shared_ptr<C2LinearMemory> linear_mem_;
#if defined(ENABLE_AUDIO_PLUGINS)
    std::shared_ptr<qc2audio::QC2BufferCirclePools> linear_circle_pool_;
#endif  // ENABLE_AUDIO_PLUGINS

    C2ModeType mode_;

    std::mutex lock_;
};

// TODO Can be made part of the C2Module
class C2Listener : public C2Component::Listener {
public:
    C2Listener(C2Module *module) : module_(module) {}

    // Inherited from C2Component::Listener
    void onWorkDone_nb(std::weak_ptr<C2Component> component __unused,
                       std::list<std::unique_ptr<C2Work>> witems) override {
        module_->HandleWorkDone(std::move(witems));
    }

    void onTripped_nb(std::weak_ptr<C2Component> component __unused,
                      std::vector<std::shared_ptr<C2SettingResult>> results) override {
        module_->HandleTripped(std::move(results));
    }

    void onError_nb(std::weak_ptr<C2Component> component __unused, uint32_t error) override {
        module_->HandleError(error);
    }
private:
    C2Module *module_;
};

/** C2Factory
 *
 * Static class for retrieving a Codec2 component from teh store and creating
 * a module out of it.
 **/
class C2Factory {
public:
    static C2Module *GetModule(std::string name, C2ModeType mode);
private:
    using QC2ComponentStoreFactoryGetter_t = QC2ComponentStoreFactory *(*)(int major, int minor);

    static std::shared_ptr<QC2ComponentStoreFactory> factory_video_;
    static std::shared_ptr<QC2ComponentStoreFactory> factory_audio_;
    static std::mutex lock_;
};

#endif  // __GST_C2_COMPONENT_H__
