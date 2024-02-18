/*

NOTES:

Since we're clocking at 60FPS using vsync, some mouse data will get lost.
To increase the polling rate, rendering could be put in a separate thread.

*/

#include <stdio.h>
#include "SDL.h"

#define WIDTH 40
#define HEIGHT 30
#define INITIAL_WIDTH 640
#define INITIAL_HEIGHT 480

// string magic
static uint32_t pot(uint32_t n) {if (n == 0) return 1; uint32_t r = 10;for (uint32_t i = 0; i < n-1; i++) {r *= 10;}return r;}
static uint8_t dd(uint32_t num, uint32_t n) {uint32_t r;r = num/pot(n);r = r%10;return (uint8_t) r;}
#define BYTE "%c%c%c%c%c%c%c%c"
#define CONVERT(B) ((B) & 0x80 ? '1' : '0'), ((B) & 0x40 ? '1' : '0'), ((B) & 0x20 ? '1' : '0'), ((B) & 0x10 ? '1' : '0'), ((B) & 0x08 ? '1' : '0'), ((B) & 0x04 ? '1' : '0'), ((B) & 0x02 ? '1' : '0'), ((B) & 0x01 ? '1' : '0') 
#define WINDOWTITLE(NUM) (char[]) {dd(NUM, 4)+48, dd(NUM, 3)+48, dd(NUM, 2)+48, dd(NUM, 1)+48, dd(NUM, 0)+48, ' ', 'F', 'P', 'S', 0}
 
int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Init(SDL_INIT_TIMER);
    SDL_Window* window = SDL_CreateWindow("----- FPS", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, INITIAL_WIDTH, INITIAL_HEIGHT, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetWindowMinimumSize(window, WIDTH, HEIGHT);
    SDL_Texture* screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    uint32_t* pixels = malloc(WIDTH * HEIGHT * sizeof(uint32_t));

    // fill white
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            pixels[x + y * WIDTH] = 0xffffffff;
        }
    }

    uint32_t w = INITIAL_WIDTH;
    uint32_t h = INITIAL_HEIGHT;
    uint32_t timestamp = SDL_GetTicks();
    uint32_t framecount = 0;
    while (1) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) exit(0);
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
                w=event.window.data1;
                h=event.window.data2;
                printf(">>> RESIZE EVENT: [%i, %i]\n", w, h);
            }

        }

        int x, y;
        uint32_t buttons = SDL_GetMouseState(&x, &y);
        if (x < 0) x = 0; if (x >= w) x = w-1;
        if (y < 0) y = 0; if (y >= h) y = h-1;

        printf("["BYTE", %i, %i]\n", CONVERT(buttons), x, y);

        // draw!
        if (buttons != 0) {
            int _x = (x*WIDTH)/w;
            int _y = (y*HEIGHT/h);
            printf(">>> DRAWING ON SURFACE [%i, %i]\n", _x, _y);
            uint32_t color = 0x000000ff;
            if (buttons == 1) color = 0xff0000ff;
            else if (buttons == 2) color = 0x00ff00ff;
            else if (buttons == 4) color = 0x0000ffff;

            pixels[_y*WIDTH + _x] = color;
        }

        SDL_RenderClear(renderer);
        SDL_UpdateTexture(screen_texture, NULL, pixels, WIDTH * 4);
        SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        framecount ++;
        if (SDL_GetTicks() - timestamp > 1000) {
            SDL_SetWindowTitle(window, WINDOWTITLE(framecount));
            timestamp = SDL_GetTicks();
            framecount = 0;
        }
    }
}
