#include "encode.hpp"


RawVideoDecoder::RawVideoDecoder(int width, int height): width(width), height(height) {}

std::unique_ptr<Frame> RawVideoDecoder::decode(const Packet &packet){
    if(packet.media_type != MediaType::Video){
        return nullptr;
    }
    auto frame = std::make_unique<Frame>();
    frame->data = packet.data;
    frame->width = width;
    frame->height = height;
    frame->time_point = packet.time_point;
    return frame;
}

PcmAudioDecoder::PcmAudioDecoder(int sampleRate, int numChannels, int bitsPerSample) : sampleRate(sampleRate), numChannels(numChannels), bitsPerSample(bitsPerSample){}

std::unique_ptr<Frame> PcmAudioDecoder::decode(const Packet &packet){
    if(packet.media_type != MediaType::Audio){
        return nullptr;
    }
    auto frame = std::make_unique<Frame>();
    frame->data = packet.data;
    frame->time_point = packet.time_point;
    frame->sampleRate = sampleRate;
    frame->numChannels = numChannels;
    frame->bitsPerSample = bitsPerSample;

    return frame;

}