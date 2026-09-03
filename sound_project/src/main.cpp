#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <csignal>
#include <limits>
#include "SoundGenerator.hpp"

std::unique_ptr<SoundGenerator> soundGen;
static volatile std::sig_atomic_t g_stop = 0;

void signalHandler(int) {
    g_stop = 1;
}

void showMenu() {
    std::cout << "\n=== ARCADE SOUND SYSTEM ===\n";
    std::cout << "1. Basic Beep (440Hz)\n";
    std::cout << "2. Arcade Blip\n";
    std::cout << "3. Explosion Sound\n";
    std::cout << "4. Coin Insert\n";
    std::cout << "5. Victory Fanfare\n";
    std::cout << "6. Custom Frequency\n";
    std::cout << "7. Laser Shot\n";
    std::cout << "8. Power Up\n";
    std::cout << "9. Game Over\n";
    std::cout << "10. Terminator Theme\n";
    std::cout << "\n=== PHONE SOUNDS ===\n";
    std::cout << "11. Classic Phone Ring\n";
    std::cout << "12. Nokia Tune\n";
    std::cout << "13. Motorola Ringtone\n";
    std::cout << "14. SMS Notification\n";
    std::cout << "15. DTMF Test\n";
    std::cout << "\n=== FAX & MODEM 56k SOUNDS ===\n";
    std::cout << "16. Fax Handshake\n";
    std::cout << "17. Fax Transmission\n";
    std::cout << "18. Fax Error\n";
    std::cout << "19. Modem 56k Connect\n";
    std::cout << "20. Modem Handshake (Classic Screech)\n";
    std::cout << "21. Complete Dial-Up Sequence\n";
    std::cout << "22. Modem Training\n";
    std::cout << "23. Fax Pickup\n";
    std::cout << "24. Modem Hangup\n";
    std::cout << "25. Busy Tone\n";
    std::cout << "26. Dial Tone\n";
    std::cout << "0. Exit\n";
    std::cout << "Choose option: ";
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGPIPE, SIG_IGN);

    try {
        soundGen = std::make_unique<SoundGenerator>(21, 22050);
        std::cout << "Sound system initialized on GPIO21\n";
        std::cout << "Starting arcade sound demo...\n";

        soundGen->playArcadeBlip();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        soundGen->playCoinSound();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        soundGen->playVictoryFanfare();
        soundGen->stop();

        int choice = -1;
        while (!g_stop) {
            showMenu();
            if (!(std::cin >> choice)) {
                if (g_stop || std::cin.eof()) break;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            switch (choice) {
                case 1:
                    soundGen->playArcadeBeep(440, 300);
                    break;
                case 2:
                    soundGen->playArcadeBlip();
                    break;
                case 3:
                    soundGen->playExplosion();
                    break;
                case 4:
                    soundGen->playCoinSound();
                    break;
                case 5:
                    soundGen->playVictoryFanfare();
                    break;
                case 6: {
                    int freq;
                    std::cout << "Enter frequency (Hz): ";
                    if (!(std::cin >> freq)) {
                        std::cin.clear();
                        break;
                    }
                    soundGen->playArcadeBeep(freq, 300);
                    break;
                }
                case 7:
                    soundGen->playWaveform(std::make_unique<ArcadeBlip>(1200, 800), 80);
                    break;
                case 8:
                    for (int i = 200; i <= 1000; i += 100) {
                        soundGen->playArcadeBeep(i, 50);
                        std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    }
                    break;
                case 9:
                    for (int i = 400; i >= 200; i -= 50) {
                        soundGen->playArcadeBeep(i, 150);
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }
                    break;
                case 10:
                    soundGen->playTerminatorArpeggio();
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    soundGen->playTerminatorTheme();
                    break;
                case 11:
                    soundGen->playClassicPhoneRing();
                    break;
                case 12:
                    soundGen->playOldDialUpModem();
                    break;
                case 13:
                    soundGen->playNokiaTune();
                    break;
                case 14:
                    soundGen->playMotorolaRingtone();
                    break;
                case 15:
                    soundGen->playDTMF('5');
                    break;
                case 16:
                    soundGen->playFaxHandshake();
                    break;
                case 17:
                    soundGen->playFaxTransmission();
                    break;
                case 18:
                    soundGen->playFaxError();
                    break;
                case 19:
                    soundGen->playModem56kConnect();
                    break;
                case 20:
                    soundGen->playModemHandshake();
                    break;
                case 21:
                    soundGen->playCompleteDialUpSequence();
                    break;
                case 22:
                    soundGen->playModemTraining();
                    break;
                case 23:
                    soundGen->playFaxPickup();
                    break;
                case 24:
                    soundGen->playModemHangup();
                    break;
                case 25:
                    soundGen->playBusyTone();
                    break;
                case 26:
                    soundGen->playDialTone();
                    break;
                case 0:
                    std::cout << "Exiting...\n";
                    g_stop = 1;
                    break;
                default:
                    std::cout << "Invalid option\n";
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    if (soundGen) {
        soundGen->stop();
    }
    return 0;
}
