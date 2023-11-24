#include <chrono>
#include <iostream>

#include "chip/chip8.h"
#include "sdl-window/sdl-window.h"

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " <scale> <delay> <ROM>" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  int video_scale = std::stoi(argv[1]);
  int cycle_delay = std::stoi(argv[2]);
  const std::string rom_filename = argv[3];

  CHIP8 chip = CHIP8();
  chip.load_rom(rom_filename);

  SDLWindow sdl_window = SDLWindow("CHIP8 Emulator", video_scale);
  sdl_window.update(chip.screen);

  auto last_cycle_time = std::chrono::high_resolution_clock::now();
  bool quit = false;
  while (!quit) {
    quit = sdl_window.process_input(chip.keypad);

    auto current_time = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(
                   current_time - last_cycle_time)
                   .count();

    if (dt > cycle_delay) {
      last_cycle_time = current_time;
      chip.cycle();
      sdl_window.update(chip.screen);
    }
  }
}