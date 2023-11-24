#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

const int PIXEL_WIDTH = 64;
const int PIXEL_HEIGHT = 32;
const int VIDEO_PITCH = sizeof(uint32_t) * PIXEL_WIDTH;
const std::unordered_map<SDL_Keycode, int> keymap = {
    {SDLK_1, 1},   {SDLK_2, 2},   {SDLK_3, 3},   {SDLK_4, 0xc},
    {SDLK_q, 4},   {SDLK_w, 5},   {SDLK_e, 0x6}, {SDLK_r, 0xd},
    {SDLK_a, 0x7}, {SDLK_s, 0x8}, {SDLK_d, 0x9}, {SDLK_f, 0xe},
    {SDLK_z, 0xa}, {SDLK_x, 0x0}, {SDLK_c, 0xb}, {SDLK_v, 0xf}};

class SDLWindow {
private:
  SDL_Window *window;
  SDL_Surface *screen;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  int scale;

  template <std::size_t Size>
  void handle_key(const SDL_Keycode key, std::array<uint8_t, Size> &keys,
                  const bool isKeyDown) {
    auto it = keymap.find(key);
    if (it != keymap.end()) {
      keys[it->second] = isKeyDown ? 1 : 0;
    }
  }

public:
  SDLWindow(std::string title, int scale)
      : window(nullptr), screen(nullptr), renderer(nullptr), texture(nullptr),
        scale(scale) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      std::cerr << "SDL could not initialize: %s\n" << SDL_GetError();
      return;
    }
    window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, PIXEL_WIDTH * scale,
                              PIXEL_HEIGHT * scale, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
      std::cerr << "SDL could not initialize: %s\n" << SDL_GetError();
      return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
      std::cerr << "SDL could not initialize: %s\n" << SDL_GetError();
      return;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_STREAMING, PIXEL_WIDTH,
                                PIXEL_HEIGHT);
    if (texture == nullptr) {
      std::cerr << "SDL could not initialize: %s\n" << SDL_GetError();
      return;
    }
  }

  ~SDLWindow() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
  }

  template <std::size_t Width, std::size_t Height>
  void update(const std::array<std::array<uint32_t, Width>, Height> &buffer) {
    SDL_UpdateTexture(texture, nullptr, buffer.data(), VIDEO_PITCH);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
  }

  template <std::size_t Size>
  bool process_input(std::array<uint8_t, Size> &keys) {
    bool quit = false;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
      case SDL_QUIT:
        quit = true;
        break;
      case SDL_KEYDOWN:
        if (e.key.keysym.sym == SDLK_ESCAPE) {
          quit = true;
          break;
        }
        handle_key(e.key.keysym.sym, keys, true);
        break;
      case SDL_KEYUP:
        handle_key(e.key.keysym.sym, keys, false);
        break;
      }
    }

    return quit;
  }
};
