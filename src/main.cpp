#include "main.hpp"
#include "cxxopts.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include "mux.hpp"
#include "encode.hpp"
#include "threading.hpp"
#include "filter.hpp"


ThreadSafeQueue<std::unique_ptr<Packet>> packetQueue;
ThreadSafeQueue<std::unique_ptr<Frame>> frameQueue;
ThreadSafeQueue<std::unique_ptr<Frame>> filteredQueue;

int parseCommand(int argc, char* argv[]){
    cxxopts::Options options("Audio Transformer", "My take on ffpmpeg");

    options.add_options()
        ("i,input", "Input file", cxxopts::value<std::string>())
        ("o,output", "Output file", cxxopts::value<std::string>())
        ("h,help", "Print usage");

    auto result = options.parse(argc, argv);
    if(result.count("help")){
        std::cout << options.help() << std::endl;
        return 0;
    }
    command userCmd;
    userCmd.inputFile = result["input"].as<std::string>();
    userCmd.outputFile = result["output"].as<std::string>();

    std::cout << "Input: " << userCmd.inputFile << std::endl;
    std::cout << "Output: " << userCmd.outputFile << std::endl;

    return 0;
}

int yuvTest(){
    int width = 176;
    int height = 144;
    RawVideoDemuxer demuxer(width, height);
    if(!demuxer.open("input.yuv")){
        std::cerr << "Failed to open input file.\n";
        return 1;
    }   

    RawVideoDecoder decoder(width, height);
    RawVideoEncoder encoder(width, height);

    RawVideoMuxer muxer(width, height);
    if (!muxer.open("output.yuv")) {
        std::cerr << "Failed to open output file\n";
        return 1;
    }


    while(auto packet = demuxer.readPacket()){
        auto frame = decoder.decode(*packet);
        if (frame){
            auto encodedPacket = encoder.encode(*frame);
            if (encodedPacket){
                muxer.writePacket(*encodedPacket);
                std::cout << "Wrote packet with pts: " << encodedPacket->time_point << "\n";
            }
        }
    }
    demuxer.close();
    muxer.close();
    return 0;
}

int main(int argc, char* argv[]){
    WavAudioDemuxer demuxer;
    if(!demuxer.open("input_2.wav")){
        std::cerr << "Failed to open input file.\n";
        return 1;
    }   
    std::cout << "Valid Wave File!" << std::endl;
    demuxer.printAudioInformation();

    int sampleRate = demuxer.getSampleRate();
    int numChannels = demuxer.getNumChannels();
    int bitsPerSample = demuxer.getBitsPerSample();
    size_t dataChunkSize = demuxer.getChunkSize();

    WavAudioMuxer muxer(sampleRate, numChannels, bitsPerSample);
    muxer.open("output_2.wav");

    PcmAudioDecoder decoder(sampleRate, numChannels, bitsPerSample);
    PcmAudioEncoder encoder(sampleRate, numChannels, bitsPerSample);

    size_t fadeInSampleIndex = 0;
    size_t fadeOutSampleIndex = 0;
    
    size_t totalSamples = dataChunkSize / (bitsPerSample / 8);
    AudioFilter filter(totalSamples);
    auto start = std::chrono::high_resolution_clock::now();


    std::thread demuxerThread([&]{
    while (auto packet = demuxer.readPacket()){
        if(packet){
            packetQueue.push(std::move(packet));
        }
    }
    packetQueue.setFinished();
    });

    std::thread decoderThread([&] {
        while (true) {
            auto pktOpt = packetQueue.pop();    
            if (!pktOpt) break;

            const Packet& packetRef = *(*pktOpt); 
            auto frame = decoder.decode(packetRef);
            frameQueue.push(std::move(frame));
        }
        frameQueue.setFinished();
    });

    std::thread filterThread([&]{
        while(true){
            auto frameOpt = frameQueue.pop();
            if (!frameOpt) break;

            const Frame& frameRef = *(*frameOpt);
            auto filtered = filter.applyFadeIn(frameRef, 10, fadeInSampleIndex);
            filtered = filter.applyFadeOut(*filtered, 10, fadeOutSampleIndex);
            filteredQueue.push(std::move(filtered));

        }
        filteredQueue.setFinished();
    });

    std::thread encoderThread([&] {
        while (true) {
            auto frameOpt = filteredQueue.pop();   
            if (!frameOpt) break;

            const Frame& frameRef = *(*frameOpt); 
            auto encoded = encoder.encode(frameRef);
            muxer.writePacket(*encoded);
        }
    });

    demuxerThread.join();
    decoderThread.join();
    filterThread.join();
    encoderThread.join();

  
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Total processing time: " << elapsed.count() << " seconds\n";

    demuxer.close();
    muxer.close();
    return 0;
}
