#include "SoundGenerator.hpp"
#include <thread>
#include <chrono>
#include <iostream>
#include <map>
#include <unistd.h>  // ← Agregar esta línea para usleep
#include <cstdlib>   // ← Para rand() y srand()
#include <ctime>     // ← Para time()

// El resto de tu implementación...

void SoundGenerator::playTerminatorArpeggio() {
    // Arpegio siniestro estilo Terminator
    const std::vector<int> arpeggio = {110, 131, 165, 196, 220, 262, 330, 392};
    for (int note : arpeggio) {
        playWaveform(std::make_unique<SquareWave>(note, 0.3), 80);
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
    }
}

void SoundGenerator::playTerminatorTheme() {
    // Tema principal estilo Terminator (secuencia de notas)
    // Ritmo: negra = 120 BPM -> duracion base = 500ms
    
    struct Note {
        int frequency;
        int duration; // en ms
        bool isRest;
    };
    
    std::vector<Note> theme = {
        // Frase 1 - Bajo amenazante
        {110, 400, false},  // La1
        {0, 50, true},      // silencio
        {131, 400, false},  // Do#2
        {0, 50, true},
        {165, 400, false},  // Mi2
        {0, 50, true},
        {196, 400, false},  // Sol2
        
        // Frase 2 - Ascendente tenso
        {220, 300, false},  // La2
        {0, 30, true},
        {262, 300, false},  // Do3
        {0, 30, true},
        {330, 300, false},  // Mi3
        {0, 30, true},
        {392, 600, false},  // Sol3
        
        // Frase 3 - Patrón rítmico industrial
        {110, 150, false},
        {0, 20, true},
        {110, 150, false},
        {0, 20, true},
        {131, 300, false},
        {0, 50, true},
        {165, 150, false},
        {0, 20, true},
        {165, 150, false},
        {0, 20, true},
        {196, 400, false},
        
        // Frase 4 - Climax amenazante
        {220, 200, false},
        {0, 30, true},
        {262, 200, false},
        {0, 30, true},
        {330, 200, false},
        {0, 30, true},
        {440, 800, false},  // La3 - nota sostenida
        
        // Frase 5 - Final siniestro
        {392, 300, false},
        {0, 50, true},
        {330, 300, false},
        {0, 50, true},
        {262, 300, false},
        {0, 50, true},
        {196, 600, false}
    };
    
    for (const auto& note : theme) {
        if (note.isRest) {
            std::this_thread::sleep_for(std::chrono::milliseconds(note.duration));
        } else {
            // Usar onda cuadrada para sonido agresivo
            playWaveform(std::make_unique<SquareWave>(note.frequency, 0.4), note.duration);
            std::this_thread::sleep_for(std::chrono::milliseconds(note.duration + 20));
        }
    }
}


void SoundGenerator::playTerminatorEpic() {
    // Versión más compleja con efectos de barrido de frecuencia
    // Intro: barrido descendente siniestro
    for (int freq = 800; freq >= 100; freq -= 20) {
        playWaveform(std::make_unique<SineWave>(freq, 0.3), 20);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Secuencia principal con acentos
    std::vector<std::pair<int, int>> epicTheme = {
        {110, 300}, {0, 50}, {131, 300}, {0, 50}, {165, 300}, {0, 50}, {196, 400},
        {220, 250}, {0, 30}, {262, 250}, {0, 30}, {330, 250}, {0, 30}, {440, 700},
        {392, 200}, {330, 200}, {262, 200}, {196, 200}, {165, 200}, {131, 200}, {110, 800}
    };
    
    for (const auto& [freq, duration] : epicTheme) {
        if (freq == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(duration));
        } else {
            // Mezclar ondas para sonido más rico
            playWaveform(std::make_unique<SquareWave>(freq, 0.5), duration);
            if (duration > 400) {
                // Agregar un eco en notas largas
                std::this_thread::sleep_for(std::chrono::milliseconds(duration - 100));
                playWaveform(std::make_unique<SineWave>(freq, 0.2), 100);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }
}


void SoundGenerator::playClassicPhoneRing() {
    // Ringtone clásico de teléfono fijo (EE.UU. - 2 segundos ring, 4 segundos silence)
    const int ringFreq = 440;  // 440 Hz
    const int ringDuration = 2000;  // 2 segundos
    const int silenceDuration = 4000;  // 4 segundos
    
    for (int i = 0; i < 3; i++) {  // 3 rings
        playWaveform(std::make_unique<SquareWave>(ringFreq, 0.5), ringDuration);
        std::this_thread::sleep_for(std::chrono::milliseconds(silenceDuration));
    }
}

void SoundGenerator::playOldDialUpModem() {
    // Sonido clásico de módem dial-up (años 90)
    const std::vector<std::pair<int, int>> modemSounds = {
        {1100, 800},   // Frecuencia inicial
        {1300, 400},
        {1500, 300},
        {1700, 200},
        {1900, 150},
        {2100, 100},
        {2300, 80},
        {2500, 60},
        {2700, 50},
        {2900, 40},
        {3100, 30},
        {3300, 20}
    };
    
    for (const auto& [freq, duration] : modemSounds) {
        playWaveform(std::make_unique<SineWave>(freq, 0.4), duration);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Ruido de handshake final
    for (int i = 0; i < 20; i++) {
        int noiseFreq = 1000 + (rand() % 2000);
        playWaveform(std::make_unique<SineWave>(noiseFreq, 0.2), 30);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void SoundGenerator::playDTMF(char digit) {
    // Tonos DTMF (Dual-Tone Multi-Frequency) para teléfono
    std::map<char, std::pair<int, int>> dtmfFrequencies = {
        {'1', {697, 1209}}, {'2', {697, 1336}}, {'3', {697, 1477}},
        {'4', {770, 1209}}, {'5', {770, 1336}}, {'6', {770, 1477}},
        {'7', {852, 1209}}, {'8', {852, 1336}}, {'9', {852, 1477}},
        {'*', {941, 1209}}, {'0', {941, 1336}}, {'#', {941, 1477}}
    };
    
    auto it = dtmfFrequencies.find(digit);
    if (it != dtmfFrequencies.end()) {
        // Reproducir ambos tonos simultáneamente (mezcla)
        auto [freq1, freq2] = it->second;
        
        // Generar waveform combinado
        std::vector<int> waveform;
        int durationMs = 200;
        int samples = (durationMs * 22050) / 1000;
        waveform.reserve(samples);
        
        for (int i = 0; i < samples; ++i) {
            double t = static_cast<double>(i) / 22050;
            double value = (sin(2.0 * M_PI * freq1 * t) + sin(2.0 * M_PI * freq2 * t)) / 2.0;
            waveform.push_back(value > 0 ? 1 : 0);
        }
        
        auto combinedWave = std::make_unique<SineWave>(freq1, 0.5);
        playWaveform(std::move(combinedWave), durationMs);
    }
}

void SoundGenerator::playNokiaTune() {
    // Tono clásico Nokia (Gran Vals)
    struct Note {
        int frequency;
        int duration;
    };
    
    std::vector<Note> nokiaTune = {
        {392, 300}, {392, 300}, {440, 300}, {440, 300}, {392, 300}, {392, 300},
        {330, 600}, {392, 300}, {392, 300}, {330, 300}, {330, 300}, {294, 600},
        {392, 300}, {392, 300}, {440, 300}, {440, 300}, {392, 300}, {392, 300},
        {330, 600}, {392, 300}, {330, 300}, {294, 300}, {330, 300}, {262, 600}
    };
    
    for (const auto& note : nokiaTune) {
        playWaveform(std::make_unique<SineWave>(note.frequency, 0.6), note.duration);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

void SoundGenerator::playMotorolaRingtone() {
    // Ringtone clásico Motorola (Hello Moto)
    const std::vector<std::pair<int, int>> motorolaTune = {
        {523, 200}, {0, 50}, {587, 200}, {0, 50}, {659, 200}, {0, 50},
        {784, 400}, {0, 100}, {659, 200}, {0, 50}, {587, 200}, {0, 50},
        {523, 600}
    };
    
    for (const auto& [freq, duration] : motorolaTune) {
        if (freq == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(duration));
        } else {
            playWaveform(std::make_unique<SquareWave>(freq, 0.4), duration);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }
}

void SoundGenerator::playBusyTone() {
    // Tono de ocupado (480Hz + 620Hz, 0.5s on, 0.5s off)
    for (int i = 0; i < 5; i++) {
        playWaveform(std::make_unique<SineWave>(480, 0.4), 500);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void SoundGenerator::playDialTone() {
    // Tono de marcación (350Hz + 440Hz)
    playWaveform(std::make_unique<SineWave>(350, 0.3), 3000);
}

void SoundGenerator::playRingBackTone() {
    // Tono de retorno de llamada (440Hz + 480Hz, 1s on, 3s off)
    for (int i = 0; i < 3; i++) {
        playWaveform(std::make_unique<SineWave>(440, 0.4), 1000);
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
}

void SoundGenerator::playSMSNotification() {
    // Notificación SMS clásica (tono corto ascendente)
    const std::vector<int> smsTone = {440, 880, 1320, 1760};
    for (int freq : smsTone) {
        playWaveform(std::make_unique<SineWave>(freq, 0.5), 80);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

void SoundGenerator::playRetroRingtone() {
    // Ringtone retro de teléfono de disco (sonido mecánico)
    for (int i = 0; i < 4; i++) {
        // Sonido de "rining" clásico
        playWaveform(std::make_unique<SquareWave>(350, 0.6), 400);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        playWaveform(std::make_unique<SquareWave>(350, 0.6), 400);
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
    }
}

// Función para reproducir secuencia de marcado (ej: número de teléfono)
void SoundGenerator::playPhoneNumber(const std::string& number) {
    for (char digit : number) {
        playDTMF(digit);
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}



void SoundGenerator::playFaxHandshake() {
    // Handshake inicial de fax (CNG tone - 1100 Hz)
    std::cout << "Fax handshake iniciado...\n";
    
    // Tono CNG (1100 Hz, 0.5s on, 3s off)
    for (int i = 0; i < 3; i++) {
        playWaveform(std::make_unique<SineWave>(1100, 0.5), 500);
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
    
    // Tono CED (2100 Hz)
    playWaveform(std::make_unique<SineWave>(2100, 0.6), 3000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void SoundGenerator::playFaxTransmission() {
    // Sonido de transmisión de fax (datos modulados)
    std::cout << "Transmitiendo fax...\n";
    
    // Secuencia de tonos V.27ter/V.29 (4800/9600 bps)
    const std::vector<std::pair<int, int>> faxTones = {
        {1650, 200}, {1650, 200}, {1850, 200}, {1850, 200},
        {1650, 150}, {1850, 150}, {1650, 150}, {1850, 150},
        {1750, 100}, {1750, 100}, {1750, 100}, {1750, 400}
    };
    
    for (const auto& [freq, duration] : faxTones) {
        playWaveform(std::make_unique<SquareWave>(freq, 0.4), duration);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    
    // Ruido de datos de fax (modulación de frecuencia)
    for (int i = 0; i < 50; i++) {
        int freq = 1650 + (i % 3) * 200;
        playWaveform(std::make_unique<SineWave>(freq, 0.3), 50);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void SoundGenerator::playFaxError() {
    // Tono de error de fax
    std::cout << "Error en transmisión de fax!\n";
    
    for (int i = 0; i < 5; i++) {
        playWaveform(std::make_unique<SineWave>(400, 0.8), 300);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        playWaveform(std::make_unique<SineWave>(300, 0.8), 300);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void SoundGenerator::playModemCarrierDetect() {
    // Tono de detección de portadora (1650 Hz)
    std::cout << "Detectando portadora...\n";
    playWaveform(std::make_unique<SineWave>(1650, 0.5), 1000);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void SoundGenerator::playModemTraining() {
    // Secuencia de entrenamiento del módem
    std::cout << "Entrenando módem...\n";
    
    // Barrido de frecuencias ascendente (entrenamiento)
    for (int freq = 800; freq <= 2000; freq += 50) {
        playWaveform(std::make_unique<SineWave>(freq, 0.3), 30);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    // Barrido descendente
    for (int freq = 2000; freq >= 800; freq -= 50) {
        playWaveform(std::make_unique<SineWave>(freq, 0.3), 30);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    // Tonos de ecualización
    const std::vector<int> eqTones = {1200, 1600, 1800, 2000, 2200};
    for (int freq : eqTones) {
        playWaveform(std::make_unique<SineWave>(freq, 0.4), 150);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void SoundGenerator::playModemNegotiation() {
    // Negociación de protocolo V.34/V.90
    std::cout << "Negociando protocolo...\n";
    
    // Tonos de respuesta ANSam
    playWaveform(std::make_unique<SineWave>(2100, 0.6), 3000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Secuencia de handshake
    const std::vector<int> handshake = {2400, 1200, 2400, 1200, 2400};
    for (int freq : handshake) {
        playWaveform(std::make_unique<SineWave>(freq, 0.5), 200);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void SoundGenerator::playModem56kConnect() {
    // Secuencia completa de conexión 56k (V.90/V.92)
    std::cout << "Conectando a 56k...\n";
    
    // 1. Tono de marcación
    playDialTone();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // 2. Marcado DTMF (número de ISP)
    std::string ispNumber = "1234567890";
    for (char digit : ispNumber) {
        playDTMF(digit);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // 3. Tono de ringback
    for (int i = 0; i < 2; i++) {
        playWaveform(std::make_unique<SineWave>(440, 0.5), 1000);
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
    
    // 4. Tono de respuesta del módem remoto (2100 Hz)
    playWaveform(std::make_unique<SineWave>(2100, 0.6), 3000);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // 5. Ruido de handshake inicial (modulación QAM)
    for (int i = 0; i < 30; i++) {
        int freq = 1000 + (rand() % 1000);
        playWaveform(std::make_unique<SineWave>(freq, 0.2), 40);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    // 6. Secuencia de entrenamiento V.34
    for (int freq = 800; freq <= 2000; freq += 30) {
        playWaveform(std::make_unique<SineWave>(freq, 0.4), 20);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    
    // 7. Tonos de negociación de velocidad
    const std::vector<int> speedTones = {1200, 2400, 4800, 9600, 14400, 19200, 28800, 33600, 38400, 44000, 48000, 52000, 56000};
    for (int freq : speedTones) {
        playWaveform(std::make_unique<SineWave>(freq/10, 0.3), 100);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    
    // 8. Ruido blanco de datos (simulación de conexión establecida)
    std::cout << "Conexión establecida! Sonido de datos...\n";
    for (int i = 0; i < 100; i++) {
        int freq = 1200 + (rand() % 800);
        int duration = 20 + (rand() % 60);
        playWaveform(std::make_unique<SineWave>(freq, 0.25), duration);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void SoundGenerator::playModemHandshake() {
    // Handshake básico de módem (el clásico screech)
    std::cout << "Handshake de módem...\n";
    
    // Tono inicial (2400 Hz)
    playWaveform(std::make_unique<SineWave>(2400, 0.5), 500);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Secuencia de handshake de Bell 103/V.21
    const std::vector<int> bell103 = {2025, 2225, 2025, 2225, 2025, 2225};
    for (int freq : bell103) {
        playWaveform(std::make_unique<SineWave>(freq, 0.6), 200);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    // Ruido de datos inicial
    for (int i = 0; i < 50; i++) {
        int freq = 1800 + (rand() % 400);
        playWaveform(std::make_unique<SquareWave>(freq, 0.3), 30);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void SoundGenerator::playCompleteDialUpSequence() {
    // Secuencia completa y realista de dial-up
    std::cout << "=== SIMULACIÓN COMPLETA DE DIAL-UP 56k ===\n";
    std::cout << "1. Marcando número...\n";
    
    // Marcado (números de ejemplo)
    std::string number = "5551234";
    for (char digit : number) {
        playDTMF(digit);
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
    }
    
    std::cout << "2. Esperando respuesta...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Ringback (el teléfono del ISP contestando)
    for (int i = 0; i < 2; i++) {
        playWaveform(std::make_unique<SineWave>(440, 0.5), 800);
        std::this_thread::sleep_for(std::chrono::milliseconds(2800));
    }
    
    std::cout << "3. Handshake inicial (el famoso screech)...\n";
    // Tono de respuesta del módem (2100 Hz con inversión de fase)
    playWaveform(std::make_unique<SineWave>(2100, 0.7), 2800);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    // Ruido de handshake (el característico sonido de módem)
    for (int i = 0; i < 40; i++) {
        int freq = 1600 + (rand() % 600);
        playWaveform(std::make_unique<SineWave>(freq, 0.35), 35);
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }
    
    std::cout << "4. Entrenamiento y ecualización...\n";
    // Secuencia de entrenamiento (tonos ascendentes rápidos)
    for (int freq = 1000; freq <= 3000; freq += 100) {
        playWaveform(std::make_unique<SineWave>(freq, 0.4), 25);
        usleep(5000);  // Microsegundos
    }
    
    std::cout << "5. Negociación de protocolo V.90...\n";
    // Tonos de negociación
    const std::vector<int> negotiation = {1200, 2400, 4800, 9600, 14400, 19200, 28800, 33600, 44000, 52000};
    for (int freq : negotiation) {
        playWaveform(std::make_unique<SineWave>(freq/10, 0.3), 80);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
    
    std::cout << "6. Estableciendo conexión...\n";
    // Ruido de datos (el sonido clásico de "conectando")
    for (int i = 0; i < 80; i++) {
        int freq = 1400 + (rand() % 1000);
        int duration = 15 + (rand() % 25);
        playWaveform(std::make_unique<SineWave>(freq, 0.3), duration);
        usleep(5000);
    }
    
    std::cout << "7. Conexión establecida a 52000 bps!\n";
    // Tono de confirmación final
    playWaveform(std::make_unique<SineWave>(1800, 0.5), 500);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    playWaveform(std::make_unique<SineWave>(2000, 0.5), 500);
    
    std::cout << "=== CONEXIÓN COMPLETADA ===\n";
}

void SoundGenerator::playFaxPickup() {
    // Sonido de fax contestando
    std::cout << "Fax contestando...\n";
    
    // Tono CED (2100 Hz) + V.21 handshake
    playWaveform(std::make_unique<SineWave>(2100, 0.6), 2800);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    // Handshake V.21
    const std::vector<int> v21Handshake = {1850, 1650, 1850, 1650, 1850, 1650};
    for (int freq : v21Handshake) {
        playWaveform(std::make_unique<SineWave>(freq, 0.5), 150);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

void SoundGenerator::playModemHangup() {
    // Tono de colgar del módem
    std::cout << "Desconectando módem...\n";
    
    // Tono de liberación
    playWaveform(std::make_unique<SineWave>(600, 0.5), 300);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    playWaveform(std::make_unique<SineWave>(300, 0.5), 300);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Ruido de desconexión
    for (int i = 0; i < 20; i++) {
        int freq = 1000 + (rand() % 500);
        playWaveform(std::make_unique<SineWave>(freq, 0.2), 20);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    // Tono de colgado final
    playWaveform(std::make_unique<SineWave>(440, 0.5), 1000);
}
