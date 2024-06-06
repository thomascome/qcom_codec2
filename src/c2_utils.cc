#include "c2_utils.h"

#include <C2BlockInternal.h>
#include <C2Buffer.h>
#include <C2PlatformSupport.h>

#include "base/log.h"

bool C2Utils::ImportHandleInfo(C2StreamBuffer *stream_buffer, ::android::C2HandleGBM *handle) {
    return false;
}

std::shared_ptr<C2Buffer> C2Utils::CreateBuffer(C2StreamBuffer *stream_buffer,
                                                std::shared_ptr<C2GraphicBlock> &block) {
    C2GraphicView view = block->map().get();
    if (view.error() != C2_OK) {
        base::LogError() << "Failed to map C2 graphic block, error " << view.error();
        return nullptr;
    }

    // Fetch the array of pointers to the planes.
    uint8_t *const *data = view.data();
    // Fetch the GBM handle containing the destination stride and scanline.
    auto handle = static_cast<const android::C2HandleGBM *>(block->handle());

    for (uint32_t idx = 0; idx < stream_buffer->planes; idx++) {
        uint32_t n_rows = (idx == 0) ? stream_buffer->height : (stream_buffer->height / 2);

        // Set the source and destination pointers for the next plane.
        uint8_t *source = static_cast<uint8_t *>(stream_buffer->data) + stream_buffer->offset[idx];
        uint8_t *destination = static_cast<uint8_t *>(data[0]) +
                               (idx * handle->mInts.stride * handle->mInts.slice_height);

        for (uint32_t num = 0; num < n_rows; num++) {
            memcpy(destination, source, stream_buffer->stride[idx]);

            destination += handle->mInts.stride;
            source += stream_buffer->stride[idx];
        }
    }

    auto c2buffer = C2Buffer::CreateGraphicBuffer(
        block->share(C2Rect(block->width(), block->height()), ::C2Fence()));
    if (!c2buffer) {
        base::LogError() << "Failed to create graphic C2 buffer!";
        return nullptr;
    }

    return c2buffer;
}