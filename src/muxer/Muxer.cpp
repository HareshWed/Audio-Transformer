#include <iostream>
#include <cstring>
#include "mux.hpp"

RawVideoMuxer::RawVideoMuxer(int width, int height): width(width), height(height), frameSize(width * height * 3 / 2){}

bool RawVideoMuxer::open(const std::string &filename){
    file.open(filename, std::ios::binary);
    if(!file.is_open()){
        std::cerr << "Failed to open output file: " << filename << "\n";
        return false;
    }
    return true;
}

void RawVideoMuxer::writePacket(const Packet &packet){
    if(!file.is_open()) return;
    if(packet.media_type != MediaType::Video) return;
    if(packet.data.size() != frameSize){
        std::cerr << "Warning: unexpected frame size (expected " << frameSize << ", got " << packet.data.size() << ")\n";
    }
    file.write(reinterpret_cast<const char*>(packet.data.data()), packet.data.size());
}

void RawVideoMuxer::close(){
    if(file.is_open()) file.close();
}


WavAudioMuxer::WavAudioMuxer(int sampleRate, int numChannels, int bitsPerSample): sampleRate(sampleRate), numChannels(numChannels), bitsPerSample(bitsPerSample){}

bool WavAudioMuxer::open(const std::string &filename){
    this->filename = filename;
    file.open(filename, std::ios::binary);
    if(!file.is_open()){
        std::cerr << "Failed to open output file: " << filename << "\n";
        return false;
    }
    writeWavHeader();
    return true;
}

void WavAudioMuxer::writeWavHeader(){
    char header[44] = {};
    file.write(header, 44);
}
void WavAudioMuxer::writePacket(const Packet &packet){
    if(!file.is_open()) return;
    if(packet.media_type != MediaType::Audio) return;

    file.write(reinterpret_cast<const char*>(packet.data.data()), packet.data.size());
    dataSize += packet.data.size();
}

void WavAudioMuxer::finalizeWavHeader(){
    file.seekp(0, std::ios::beg);

    uint32_t byteRate = sampleRate * numChannels * bitsPerSample / 8;
    uint16_t blockAlign = numChannels * bitsPerSample / 8;
    uint32_t dataChunkSize = static_cast<uint32_t>(dataSize);
    uint32_t fmtChunkSize = 16;
    uint16_t audioFormat = 1; 
    uint32_t fileSize = 36 + dataChunkSize;

    file.write("RIFF", 4);
    file.write(reinterpret_cast<const char*>(&fileSize), 4);
    file.write("WAVE", 4);

    file.write("fmt ", 4);
    file.write(reinterpret_cast<const char*>(&fmtChunkSize), 4);
    file.write(reinterpret_cast<const char*>(&audioFormat), 2);
    file.write(reinterpret_cast<const char*>(&numChannels), 2);
    file.write(reinterpret_cast<const char*>(&sampleRate), 4);
    file.write(reinterpret_cast<const char*>(&byteRate), 4);
    file.write(reinterpret_cast<const char*>(&blockAlign), 2);
    file.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&dataChunkSize), 4);
}

void WavAudioMuxer::close(){
    if(file.is_open()){
        finalizeWavHeader();
        file.close();
    } 
}