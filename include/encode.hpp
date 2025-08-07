#pragma once
#include <memory>

#include "audiodata.hpp"

class Decoder{
    public:
        virtual std::unique_ptr<Frame> decode(const Packet &packet) = 0;
};

class RawVideoDecoder:public Decoder{
    public:
        RawVideoDecoder(int width, int height);
        std::unique_ptr<Frame> decode(const Packet &packet) override;
    private:
        int width, height;
};

class PcmAudioDecoder:public Decoder{
    public:
        PcmAudioDecoder(int sampleRate, int numChannels, int bitsPerSample);
        std::unique_ptr<Frame> decode (const Packet &packet) override;
    private:
        int sampleRate;
        int numChannels;
        int bitsPerSample;

};

class Encoder{
    public:
        virtual std::unique_ptr<Packet> encode(const Frame &frame) = 0;
        virtual void flush() {};
        virtual ~Encoder() = default;
};

class RawVideoEncoder:public Encoder{
    public:
        RawVideoEncoder(int width, int height);
        std::unique_ptr<Packet> encode(const Frame &frame) override;
    private:
        int width;
        int height;      
};

class PcmAudioEncoder:public Encoder{
    public:
        PcmAudioEncoder(int sampleRate, int numChannels, int bitsPerSample);
        std::unique_ptr<Packet> encode(const Frame &frame) override;
    private:
        int sampleRate;
        int numChannels;
        int bitsPerSample;
};