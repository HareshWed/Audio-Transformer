#include "mux.hpp"
#include <iostream>
#include <cstring> 
#include <fstream>

RawVideoDemuxer::RawVideoDemuxer(int width, int height): width(width), height(height), frameSize(width * height * 3 / 2), pts(0){}

bool RawVideoDemuxer::open(const std::string &filename){
    file.open(filename, std::ios::binary);
    return file.is_open();
}

std::unique_ptr <Packet> RawVideoDemuxer::readPacket(){
    if(!file or file.eof()) return nullptr;

    auto packet = std::make_unique<Packet>();
    packet->data.resize(frameSize);
    file.read(reinterpret_cast<char*>(packet->data.data()), frameSize);

    if(file.gcount() < frameSize) return nullptr;

    packet->media_type = MediaType::Video;
    packet->time_point = pts++;
    return packet;
}

void RawVideoDemuxer::close(){
    if (file.is_open()) file.close();
}

bool WavAudioDemuxer::open(const std::string &filename){
    file.open(filename, std::ios::binary);
    if (!file.is_open()){
        std::cerr << "Failed to open WAV file: " << filename << "\n";
        return false;
    }

    //Read RIFF header
    char riffHeader[4];
    file.read(riffHeader, 4);
    if(std::memcmp(riffHeader, "RIFF", 4) != 0){
        std::cerr << "Not a valid RIFF file\n";
        return false;
    }
    file.ignore(4);

    char waveHeader[4];
    file.read(waveHeader, 4);
    if(std::memcmp(waveHeader, "WAVE", 4) != 0){
        std::cerr << "Not a WAVE file\n";
        return false;
    }

    char chunkID[4];
    uint32_t chunkSize;
    while(file.read(chunkID, 4)){
        file.read(reinterpret_cast<char*>(&chunkSize), 4);
        if(std::memcmp(chunkID, "fmt ", 4) == 0){
            file.read(reinterpret_cast<char*>(&audioFormat), 2);
            file.read(reinterpret_cast<char*>(&numChannels), 2);
            file.read(reinterpret_cast<char*>(&sampleRate), 4);
            file.ignore(6);
            file.read(reinterpret_cast<char*>(&bitsPerSample), 2);

            if(audioFormat != 1){
             std::cerr << "Only PCM audio is supported (format = " << audioFormat << ")\n";
                return false;   
            }
            if (chunkSize > 16){
                file.ignore(chunkSize - 16);
            }
        }
        else if(std::memcmp(chunkID, "data", 4) == 0){
            dataChunkSize = chunkSize;
            break;
        }
        else{
            file.ignore(chunkSize);
        }
    }
    if (dataChunkSize == 0){
            std::cerr << "Failed to find 'data' chunk in WAV file.\n";
            return false;
        }
    return true;
}

std::unique_ptr <Packet> WavAudioDemuxer::readPacket(){
    if(!file or file.eof()) return nullptr;
    const size_t bytesPerRead = 4096;

    std::vector<uint8_t> buffer(bytesPerRead);
    file.read(reinterpret_cast<char*>(buffer.data()), bytesPerRead);
    std::streamsize bytesRead = file.gcount();

    if(bytesRead == 0) return nullptr;

    buffer.resize(bytesRead);

    auto packet = std::make_unique<Packet>();
    packet->data = std::move(buffer);
    packet->media_type = MediaType::Audio;
    packet->time_point = currentPoint;

    size_t bytesPerSample = bitsPerSample / 8;
    size_t frameSize = bytesPerSample * numChannels;
    size_t samplesRead = bytesRead / frameSize;
    currentPoint += samplesRead;
    return packet;
}

void WavAudioDemuxer::close(){
    if (file.is_open()) file.close();
}

void WavAudioDemuxer::printAudioInformation() const {
    std::cout << "------Audio Information------" << std::endl;
    std::cout << "Sample Rate: " << sampleRate << std::endl;
    std::cout << "Number of Channels: " << numChannels << std::endl;
    std::cout << "Bits per sample " << bitsPerSample << std::endl;
    std::cout << "Data chunk size: " << dataChunkSize << std::endl;
}