#include "filter.hpp"
#include <cstring>
#include <algorithm>

#include <iostream>

AudioFilter::AudioFilter(size_t totalSamples) : totalSamples(totalSamples){} 

std::unique_ptr<Frame> AudioFilter::applyVolume(const Frame& input, float gain){
    auto output = std::make_unique<Frame>();
    output->data = input.data;
    output->time_point = input.time_point;
    output->width = input.width;
    output->height = input.height;
    output->sampleRate = input.sampleRate;
    output->numChannels = input.numChannels;

    int16_t* samples = reinterpret_cast<int16_t*>(output->data.data());
    size_t numSamples = output->data.size() / sizeof(int16_t);

    for (size_t i = 0; i < numSamples; ++i){
        int32_t scaled = static_cast<int32_t>(samples[i] * gain);
        samples[i] = std::clamp(scaled, -32768, 32767);
    }
    return output;
}

std::unique_ptr<Frame> AudioFilter::applyFadeIn(const Frame& input, float durationSeconds, size_t& sampleIndex){
    auto output = std::make_unique<Frame>();
    output->data = input.data;
    output->time_point = input.time_point;
    output->width = input.width;
    output->height = input.height;
    output->sampleRate = input.sampleRate;
    output->numChannels = input.numChannels;

    int16_t* samples = reinterpret_cast<int16_t*>(output->data.data());
    size_t numSamples = output->data.size() / sizeof(int16_t);
    size_t totalFadeSamples = static_cast<size_t>(durationSeconds * output->sampleRate * output->numChannels);

    for (size_t i = 0; i < numSamples && sampleIndex < totalFadeSamples; ++i, ++sampleIndex){
        float scale = (static_cast<float>(sampleIndex) / output->numChannels) / totalFadeSamples;
        float fadedSample = samples[i] * scale;
        samples[i] = static_cast<int16_t>(std::clamp(fadedSample, -32768.0f, 32767.0f));
    }
    return output;
}

std::unique_ptr<Frame> AudioFilter::applyFadeOut(const Frame& input, float durationSeconds, size_t& sampleIndex){
    auto output = std::make_unique<Frame>();
    output->data = input.data;
    output->time_point = input.time_point;
    output->width = input.width;
    output->height = input.height;
    output->sampleRate = input.sampleRate;
    output->numChannels = input.numChannels;

    int16_t* samples = reinterpret_cast<int16_t*>(output->data.data());
    size_t numSamples = output->data.size() / sizeof(int16_t);
    size_t totalFadeSamples = static_cast<size_t>(durationSeconds * output->sampleRate * output->numChannels);
    size_t fadeIndexStart = totalSamples - totalFadeSamples;

    for (size_t i = 0; i < numSamples; ++i, ++sampleIndex){
        if (sampleIndex < fadeIndexStart){
            continue;
        }
        float scale = (static_cast<float>(totalSamples - sampleIndex - 1) / output->numChannels) / totalFadeSamples;
        float fadedSample = samples[i] * scale;
        samples[i] = static_cast<int16_t>(std::clamp(fadedSample, -32768.0f, 32767.0f));
    }
    return output;
}