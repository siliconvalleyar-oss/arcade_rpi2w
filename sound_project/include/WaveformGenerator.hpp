#ifndef WAVEFORM_GENERATOR_HPP
#define WAVEFORM_GENERATOR_HPP

#include <vector>
#include <cmath>
#include <memory>
#include <algorithm>

class WaveformGenerator {
public:
    virtual ~WaveformGenerator() = default;
    virtual std::vector<int> generateWaveform(int durationMs, int sampleRate) const = 0;
};

class SineWave : public WaveformGenerator {
private:
    int frequency;
    double amplitude;
    
public:
    SineWave(int freq, double amp = 0.5) : frequency(freq), amplitude(amp) {}
    
    std::vector<int> generateWaveform(int durationMs, int sampleRate) const override {
        std::vector<int> waveform;
        int samples = (durationMs * sampleRate) / 1000;
        waveform.reserve(samples);
        
        for (int i = 0; i < samples; ++i) {
            double t = static_cast<double>(i) / sampleRate;
            double value = amplitude * sin(2.0 * M_PI * frequency * t);
            waveform.push_back(value > 0 ? 1 : 0);
        }
        return waveform;
    }
};

class SquareWave : public WaveformGenerator {
private:
    int frequency;
    double dutyCycle;
    
public:
    SquareWave(int freq, double duty = 0.5) : frequency(freq), dutyCycle(duty) {}
    
    std::vector<int> generateWaveform(int durationMs, int sampleRate) const override {
        std::vector<int> waveform;
        int samples = (durationMs * sampleRate) / 1000;
        int periodSamples = sampleRate / frequency;
        waveform.reserve(samples);
        
        for (int i = 0; i < samples; ++i) {
            int pos = i % periodSamples;
            waveform.push_back(pos < periodSamples * dutyCycle ? 1 : 0);
        }
        return waveform;
    }
};

class ArcadeBlip : public WaveformGenerator {
private:
    int startFreq;
    int endFreq;
    
public:
    ArcadeBlip(int startFreq = 800, int endFreq = 400) 
        : startFreq(startFreq), endFreq(endFreq) {}
    
    std::vector<int> generateWaveform(int durationMs, int sampleRate) const override {
        std::vector<int> waveform;
        int samples = (durationMs * sampleRate) / 1000;
        waveform.reserve(samples);
        
        for (int i = 0; i < samples; ++i) {
            double t = static_cast<double>(i) / samples;
            int freq = startFreq + (endFreq - startFreq) * t;
            int periodSamples = sampleRate / freq;
            waveform.push_back((i / (periodSamples / 2)) % 2);
        }
        return waveform;
    }
};

class ExplosionSound : public WaveformGenerator {
public:
    std::vector<int> generateWaveform(int durationMs, int sampleRate) const override {
        std::vector<int> waveform;
        int samples = (durationMs * sampleRate) / 1000;
        waveform.reserve(samples);
        
        for (int i = 0; i < samples; ++i) {
            double noise = (rand() % 200 - 100) / 100.0;
            double envelope = 1.0 - (static_cast<double>(i) / samples);
            waveform.push_back((noise * envelope) > 0 ? 1 : 0);
        }
        return waveform;
    }
};

#endif
