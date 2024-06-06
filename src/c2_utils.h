#pragma once

#include <C2AllocatorGBM.h>
#include <C2Config.h>

#include "c2_common.h"

class C2Utils {
public:
    /** 
     * @brief Fills Codec2 GBM handle with the information (fd, width, height, etc.) imported from the GStreamer buffer.
     * @buffer: Pointer to custom buffer.
     * @handle: Pointer to Codec2 GBM handle to be filled with data.
     *
     * @return: true on success or false on failure.
    */
    static bool ImportHandleInfo(C2StreamBuffer *stream_buffer, ::android::C2HandleGBM *handle);
    /**
     * @brief Copy the data from the Custom stream buffer into the Codec2 graphic block and place it into a Codec2 buffer wrapper.
     * @param buffer: stream_buffer to custom buffer.
     * @param block: Reference to Codec2 graphic block.
     *
     * @return: Empty shared pointer on failure.
    */
    static std::shared_ptr<C2Buffer> CreateBuffer(C2StreamBuffer *stream_buffer,
                                                  std::shared_ptr<C2GraphicBlock> &block);
};