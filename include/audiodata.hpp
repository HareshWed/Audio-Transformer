#pragma once
#include <cstdint>
#include <vector>

using Timestamp = int64_t;

enum class MediaType{
    Video,
    Audio,
    Subtitle
};

struct Packet{
    std::vector<uint8_t> data;
    MediaType media_type;
    Timestamp time_point;
};

struct Frame{
    std::vector<uint8_t> data;
    
    int sampleRate = 0;
    int numChannels = 0;
    int bitsPerSample = 0;

    int width;
    int height;
    Timestamp time_point;
};