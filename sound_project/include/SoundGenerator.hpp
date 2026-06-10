#ifndef SOUND_GENERATOR_HPP
#define SOUND_GENERATOR_HPP

#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include "WaveformGenerator.hpp"
#include "PiezoDriver.hpp"

class SoundGenerator {
private:
    std::unique_ptr<PiezoDriver> piezo;
    std::unique_ptr<std::thread> audioThread;
    int sampleRate;
    
public:
    SoundGenerator(uint8_t pin, int sampleRate = 44100) 
        : sampleRate(sampleRate) {
        piezo = std::make_unique<PiezoDriver>(pin);
    }
    
    ~SoundGenerator() {
        stop();
    }
    
    void playWaveform(std::unique_ptr<WaveformGenerator> waveform, int durationMs) {
        stop();
        
        audioThread = std::make_unique<std::thread>([this, waveform = std::move(waveform), durationMs]() {
            auto samples = waveform->generateWaveform(durationMs, sampleRate);
            piezo->generateCustomWaveform(samples, sampleRate);
        });
    }
    
    void playArcadeBeep(int frequency, int durationMs) {
        playWaveform(std::make_unique<SquareWave>(frequency, 0.5), durationMs);
    }
    
    void playArcadeBlip() {
        playWaveform(std::make_unique<ArcadeBlip>(880, 440), 150);
    }
    
    void playExplosion() {
        playWaveform(std::make_unique<ExplosionSound>(), 500);
    }
    
    void playCoinSound() {
        auto combined = std::make_unique<SquareWave>(1200, 0.3);
        playWaveform(std::move(combined), 100);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        playWaveform(std::make_unique<SquareWave>(800, 0.3), 100);
    }
    
    void playVictoryFanfare() {
        const std::vector<int> notes = {523, 587, 659, 523, 659, 784};
        for (int note : notes) {
            playWaveform(std::make_unique<SquareWave>(note, 0.5), 200);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    
    void stop() {
        if (audioThread && audioThread->joinable()) {
            piezo->stop();
            audioThread->join();
        }
    }

void playTerminatorTheme();
void playTerminatorArpeggio();
void playTerminatorEpic();
// Sonidos de teléfono y ringtones clásicos
void playClassicPhoneRing();
void playOldDialUpModem();
void playDTMF(char digit);  // Tonos DTMF (teclado telefónico)
void playNokiaTune();
void playMotorolaRingtone();
void playBusyTone();
void playDialTone();
void playRingBackTone();
void playSMSNotification();
void playRetroRingtone();

void playPhoneNumber(const std::string& number);

// Sonidos de Fax y Módem 56k
void playFaxHandshake();
void playFaxTransmission();
void playFaxError();
void playModem56kConnect();
void playModemHandshake();
void playModemNegotiation();
void playModemCarrierDetect();
void playModemTraining();
void playCompleteDialUpSequence();
void playFaxPickup();
void playModemHangup();

};

#endif
