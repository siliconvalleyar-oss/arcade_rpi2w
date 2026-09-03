// ============================================================
//  Donkey Kong - Raspberry Pi Zero 2W + ST7789 240x240
//  main.cpp - Punto de entrada
// ============================================================
#include "../include/Game.h"
#include "../include/Hw.h"
#include <cstdio>
#include <csignal>

static Game* g_game = nullptr;

static void signalHandler(int sig)
{
    printf("\n[Main] Señal %d recibida, cerrando...\n", sig);
    if (g_game) {
        g_game->shutdown();
        g_game = nullptr;
    }
}

int main()
{
    printf("\n");
    printf("╔══════════════════════════════════════════╗\n");
    printf("║    DONKEY KONG - Raspberry Pi ST7789     ║\n");
    printf("║    240x240 - C++20 - ioctl SPI/GPIO      ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");

    signal(SIGINT,  signalHandler);
    signal(SIGTERM, signalHandler);

    if (hw_init() < 0) {
        fprintf(stderr, "[Main] hw_init fallido.\n");
        return 1;
    }

    Game game;
    g_game = &game;

    if (!game.init()) {
        fprintf(stderr, "[Main] Fallo al inicializar el juego.\n");
        hw_close();
        return 1;
    }

    printf("[Main] Iniciando loop. Ctrl+C para salir.\n\n");

    while (game.run()) {
        // run() controla el framerate internamente
    }

    game.shutdown();
    hw_close();
    printf("[Main] Fin.\n");
    return 0;
}
