#pragma once
#include <string>
#include <fstream>
#include <memory>
#include "audiodata.hpp"

// Demuxers
class Demuxer{
    public:
        virtual bool open(const std::string &filename) = 0;
        virtual std::unique_ptr<Packet> readPacket() = 0;
        virtual void close() = 0;
        virtual ~Demuxer() = default;
};


class RawVideoDemuxer:public Demuxer{
    public:
        RawVideoDemuxer(int width, int height);
        bool open(const std::string &filename) override;
        std::unique_ptr<Packet> readPacket() override;
        void close() override;

    private:
        std::ifstream file;
        int width, height;
        size_t frameSize;
        Timestamp pts = 0;
};

class WavAudioDemuxer:public Demuxer{
    public:
        bool open(const std::string &filename) override;
        std::unique_ptr<Packet> readPacket() override;
        void close() override;

        int getSampleRate() const {return sampleRate;}
        int getNumChannels() const {return numChannels;}
        int getBitsPerSample() const {return bitsPerSample;}
        size_t getChunkSize() const {return dataChunkSize;}

        void printAudioInformation() const;

    private:
        std::ifstream file;
        int sampleRate = 0;
        int numChannels = 0;
        int bitsPerSample = 0;
        uint16_t audioFormat = 0;
        size_t dataChunkSize = 0;
        Timestamp currentPoint = 0;
};


//Muxers
class Muxer{
    public:
        virtual bool open(const std::string &filename) = 0;
        virtual void writePacket(const Packet &packet) = 0;
        virtual void close() = 0;
        virtual ~Muxer() = default;
};

class RawVideoMuxer:public Muxer{
    public:
        RawVideoMuxer(int width, int height);
        bool open(const std::string &filename) override;
        void writePacket(const Packet &packet) override;
        void close() override;
    private:
        std::ofstream file;
        int width, height;
        size_t frameSize;
};

class WavAudioMuxer:public Muxer{
    public:
        WavAudioMuxer(int sampleRate, int numChannels, int bitsPerSample);
        bool open(const std::string &filename) override;
        void writePacket(const Packet &packet) override;
        void close() override;
    private:
        void writeWavHeader();
        void finalizeWavHeader();

        std::ofstream file;
        std::string filename;

        int sampleRate = 0;
        int numChannels = 0;
        int bitsPerSample = 0;
        
        size_t dataSize = 0;
};