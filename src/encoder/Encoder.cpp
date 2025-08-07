#include "encode.hpp"

RawVideoEncoder::RawVideoEncoder(int width, int height) : width(width), height(height){}

std::unique_ptr<Packet> RawVideoEncoder::encode(const Frame &frame){
    auto packet = std::make_unique<Packet>();
    packet -> data = frame.data;
    packet -> time_point = frame.time_point;
    packet -> media_type = MediaType::Video;
    return packet;
}


PcmAudioEncoder::PcmAudioEncoder(int sampleRate, int numChannels, int bitsPerSample) : sampleRate(sampleRate), numChannels(numChannels), bitsPerSample(bitsPerSample){}

std::unique_ptr<Packet> PcmAudioEncoder::encode(const Frame &frame){
    auto packet = std::make_unique<Packet>();
    packet->data = frame.data;
    packet->time_point = frame.time_point;
    packet->media_type = MediaType::Audio;
    return packet;
}