#pragma once
#include <memory>
#include "audiodata.hpp"

class AudioFilter{
    public:
        AudioFilter(size_t totalSamples);
        std::unique_ptr<Frame> applyVolume(const Frame& input, float gain);
        std::unique_ptr<Frame> applyFadeIn(const Frame& input, float durationSeconds, size_t& sampleIndex);
        std::unique_ptr<Frame> applyFadeOut(const Frame& input, float durationSeconds, size_t& sampleIndex);
        static std::unique_ptr<Frame> applyLowPass(const Frame& input, float alpha = 0.1);
    private:
        size_t totalSamples = 0;
};