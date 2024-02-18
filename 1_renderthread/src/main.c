/*

NOTES:

The update loop seems to clock in at only around 80-85% of the set update frequency.
That doesn't mean it's throttling, i've seen it work with 100kHz, resulting in right about 80kHz actual frequency.
A possible fix for this would be to create a separate update thread.

*/

#include <stdio.h>
#include <time.h>

#include <SDL.h>
#include <SDL_thread.h>

#define WIDTH 64
#define HEIGHT 48
#define INITIAL_WIDTH 640
#define INITIAL_HEIGHT 480
#define PIXEL_SIZE 4

#define UPDATE_FREQUENCY 1000000
#define NS_PER_S 1000000000

static uint32_t pot(uint32_t n) {if (n == 0) return 1; uint32_t r = 10;for (uint32_t i = 0; i < n-1; i++) {r *= 10;}return r;}
static uint8_t dd(uint32_t num, uint32_t n) {uint32_t r;r = num/pot(n);r = r%10;return (uint8_t) r;}
#define FIVEDIGIT(NUM) dd(NUM, 4)+48,dd(NUM, 3)+48,dd(NUM, 2)+48,dd(NUM, 1)+48,dd(NUM, 0)+48
#define WINDOWTITLE(FPS, UPS) (char[]) {'R','e','n','d','e','r',':',' ',FIVEDIGIT(FPS),'H','z',',',' ','U','p','d','a','t','e',':',' ',FIVEDIGIT(UPS),'H','z',0}
 
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;
uint32_t* pixels;
uint8_t run;
uint32_t fps;
uint32_t ups;

void init() {

    // sdl
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) printf("error initializing SDL: %s\n", SDL_GetError());

    // rendering
    window = SDL_CreateWindow("A", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, INITIAL_WIDTH, INITIAL_HEIGHT, SDL_WINDOW_RESIZABLE);
    if(!window) printf("Window Error!! %s\n", SDL_GetError());
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(!renderer) printf("Renderer Error!! %s\n", SDL_GetError());
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    if(!texture) printf("Texture Error!! %s\n", SDL_GetError());

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    
    // pixel buffer
    pixels = malloc(WIDTH * HEIGHT * PIXEL_SIZE);
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            pixels[y * WIDTH + x] = 0xffffffff;
        }
    }

    fps = 0;
    ups = 0;

    // brake
    run = 1;
}

void render(void* data) {
    uint32_t render_count = 0;
    uint32_t render_ts = SDL_GetTicks();
    while(run) {
        SDL_RenderClear(renderer);
        SDL_UpdateTexture(texture, NULL, pixels, WIDTH * PIXEL_SIZE);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        render_count ++;
        if (SDL_GetTicks() - render_ts > 1000) {
            fps = render_count;
            render_ts = SDL_GetTicks();
            render_count = 0;
        }
    }
}

int main() {
    init();

    SDL_Thread* renderthread = SDL_CreateThread((SDL_ThreadFunction) render, "renderthread", NULL);

    uint32_t w = INITIAL_WIDTH;
    uint32_t h = INITIAL_HEIGHT;
    uint32_t update_count = 0;
    uint32_t update_ts = SDL_GetTicks();
    clock_t start, end;
    start = clock();
    while (run) {

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                run=0;
                break;
            }
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
                w=event.window.data1;
                h=event.window.data2;
            }
        }
        int x, y;
        uint32_t buttons = SDL_GetMouseState(&x, &y);
        if (x < 0) x = 0; if (x >= w) x = w-1;
        if (y < 0) y = 0; if (y >= h) y = h-1;

        // draw!
        if (buttons != 0) {
            int _x = (x*WIDTH)/w;
            int _y = (y*HEIGHT/h);
            uint32_t color = 0x000000ff;
            if (buttons == 1) color = 0xff0000ff;
            else if (buttons == 2) color = 0x00ff00ff;
            else if (buttons == 4) color = 0x0000ffff;

            pixels[_y*WIDTH + _x] = color;
        }

        // update fps/ups display
        update_count ++;
        if (SDL_GetTicks() - update_ts > 1000) {
            ups = update_count;
            update_ts = SDL_GetTicks();
            update_count = 0;
            SDL_SetWindowTitle(window, WINDOWTITLE(fps, ups));
        }

        // sleep
        uint32_t delta_ns = (clock()-start)*(NS_PER_S/CLOCKS_PER_SEC);
        nanosleep((const struct timespec[]){{0, NS_PER_S/UPDATE_FREQUENCY - delta_ns}}, NULL);
        // printf("delta_us: %i\n", clock()-start);
        start = clock();


    }
    SDL_WaitThread(renderthread, NULL);
}
