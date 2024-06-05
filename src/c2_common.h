
#pragma once

#include <stdint.h>

// TODO This needs to be exported by Codec2
enum class C2PixelFormat : uint32_t {
    kUnknown = 0,
    // RGB-Alpha 8 bit per channel
    kRGBA = 1,
    // RGBA 8 bit compressed
    kRGBA_UBWC = 0xC2000000,
    // NV12 EXT with 128 width and height alignment
    kNV12 = 0x7FA30C04,
    // NV12 EXT with UBWC compression
    kNV12UBWC = 0x7FA30C06,
    // 10-bit Tightly-packed and compressed YUV
    kTP10UBWC = 0x7FA30C09,
// Venus 10-bit YUV 4:2:0 Planar format
#ifndef CODEC2_CONFIG_VERSION_2_0
    kP010 = 0x7FA30C0A,
#else
    kP010 = 54,
#endif
    /// YVU 4:2:0 Planar (YV12)
    kYV12 = 842094169,
};

/**
 * @brief current C2 mode
*/
enum class C2ModeType : uint32_t {
    VideoEncode,
    VideoDecode,
    AudioEncode,
    AudioDecode,
};

enum class C2CodecType : uint32_t {
    H264VideoEncode,
    H265VideoEncode,
    HEICVideoEncode
};

enum class C2EventType : uint32_t {
    kError,
    kEOS,
    kDrop
};