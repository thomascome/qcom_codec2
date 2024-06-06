#include "c2_utils.h"

bool C2Utils::ImportHandleInfo(C2StreamBuffer *stream_buffer, ::android::C2HandleGBM *handle) {
    return false;
}

std::shared_ptr<C2Buffer> C2Utils::CreateBuffer(C2StreamBuffer *stream_buffer,
                                                std::shared_ptr<C2GraphicBlock> &block) {
    return std::shared_ptr<C2Buffer>();
}