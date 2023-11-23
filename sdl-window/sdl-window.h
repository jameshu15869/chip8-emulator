#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <memory>
#include <string>

const int PIXEL_WIDTH = 64;
const int PIXEL_HEIGHT = 32;

class SDLWindow {
  // std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window;
  SDL_Window *window;
  // std::unique_ptr<SDL_Surface> screen;
  SDL_Surface *screen;
  SDL_Renderer *renderer;
  SDL_Texture *texture;

public:
  SDLWindow(std::string title, int scale) : window(nullptr), screen(nullptr), renderer(nullptr), texture(nullptr) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      fprintf(stderr, "SDL could not initialize: %s\n", SDL_GetError());
      return;
    }
    // window.reset(SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED,
    //                           SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
    //                           SCREEN_HEIGHT, SDL_WINDOW_SHOWN));
    window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, PIXEL_WIDTH * scale,
                              PIXEL_HEIGHT * scale, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
      fprintf(stderr, "SDL could not create window: %s\n", SDL_GetError());
      return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
      fprintf(stderr, "SDL could not create renderer: %s\n", SDL_GetError());
      return;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, PIXEL_WIDTH, PIXEL_HEIGHT);
    if (texture == nullptr) {
      fprintf(stderr, "SDL could not create texture: %s\n", SDL_GetError());
      return;
    }

    // screen.reset(SDL_GetWindowSurface(window));
    // screen = SDL_GetWindowSurface(window);
    // SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0xFF, 0xff, 0xff));
    // SDL_UpdateWindowSurface(window);
    std::array<std::array<uint32_t, 64>, 32> temp_buffer;
    for (auto &row : temp_buffer) {
      std::fill(row.begin(), row.end(), 0);
    }
    // temp_buffer[0][0] = 0xffffffff;
    // temp_buffer[31][63] = 0xffffffff;
    // update(temp_buffer, sizeof(uint32_t) * 64);
    SDL_Event e; bool quit = false; while( quit == false ){ while(
    SDL_PollEvent( &e ) ){ if( e.type == SDL_QUIT ) quit = true; } }
  }

  ~SDLWindow() {
    // SDL_DestroyWindow(window.get());
    // SDL_FreeSurface(screen);
    // screen = nullptr;
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
  }

  template <std::size_t Width, std::size_t Height>
  void update(const std::array<std::array<uint32_t, Width>, Height>& buffer, int pitch) {
    SDL_UpdateTexture(texture, nullptr, buffer.data(), pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
  }
};
