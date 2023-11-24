#include "chip8.h"
#include <cassert>

void CHIP8::reset() {
  address_i = 0;
  program_counter = 0x200;
  std::fill(registers.begin(), registers.end(), 0);

  // Reset the screen
  reset_screen();
}

void CHIP8::reset_screen() {
  // opcode = 0x00e0;
  for (auto &row : screen) {
    std::fill(row.begin(), row.end(), 0);
  }
}

void CHIP8::reset_keypad() { std::fill(keypad.begin(), keypad.end(), 0); }

void CHIP8::load_rom(std::string filename) {
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  int length = file.tellg();
  file.seekg(0, file.beg);

  std::vector<char> buffer(length);
  file.read(buffer.data(), length);
  file.close();

  std::copy(buffer.begin(), buffer.end(), memory.begin() + START_ADDRESS);
}

void CHIP8::cycle() {
  WORD ret = 0;
  ret = memory[program_counter];
  ret <<= 8;
  ret |= memory[program_counter + 1];
  program_counter += 2; // go to the next instruction
  opcode = ret;

  uint8_t leftmost_bit =
      opcode >> 12; // 4 (right half of left byte) + 8 (right byte)
  // execute the opcode
  (this->*table[leftmost_bit])();

  if (delay_timer > 0) {
    delay_timer--;
  }
  if (sound_timer > 0) {
    --sound_timer;
  }
}

void CHIP8::OP_NULL() {}

void CHIP8::OP_00E0() {
  assert(opcode & 0x00e0);
  reset_screen();
}

void CHIP8::OP_00EE() {
  assert(opcode & 0x00ee);
  program_counter = stack.back();
  stack.pop_back();
}

void CHIP8::OP_1NNN() {
  assert(opcode & 0x1000);
  program_counter = opcode & 0x0fff;
}

void CHIP8::OP_2NNN() {
  assert(opcode & 0x2000);
  WORD call_address = opcode & 0x0fff;
  stack.push_back(program_counter);
  program_counter = call_address;
}

void CHIP8::OP_3XKK() {
  assert(opcode & 0x3000);
  uint8_t reg_index = get_reg_x_index();
  uint8_t kk = opcode & 0x00ff;
  if (registers[reg_index] == kk) {
    program_counter += 2; // skip the instruction we WERE on
  }
}

void CHIP8::OP_4XKK() {
  assert(opcode & 0x4000);
  uint8_t reg_index = get_reg_x_index();
  uint8_t kk = opcode & 0x00ff;
  if (registers[reg_index] != kk) {
    program_counter += 2; // skip the instruction we WERE on
  }
}

void CHIP8::OP_5XY0() {
  assert(opcode & 0x5000);
  uint8_t reg_x_index = get_reg_x_index();
  uint8_t reg_y_index = get_reg_y_index();
  if (registers[reg_x_index] == registers[reg_y_index]) {
    program_counter += 2;
  }
}

void CHIP8::OP_6XKK() {
  assert(opcode & 0x6000);
  uint8_t reg_index = get_reg_x_index();
  uint8_t kk = get_kk();
  registers[reg_index] = kk;
}

void CHIP8::OP_7XKK() {
  assert(opcode & 0x7000);
  uint8_t reg_index = get_reg_x_index();
  uint8_t kk = get_kk();
  registers[reg_index] += kk;
}

void CHIP8::OP_8XY0() {
  assert(opcode & 0x8000);
  uint8_t reg_x_index = get_reg_x_index();
  uint8_t reg_y_index = get_reg_y_index();
  registers[reg_x_index] = registers[reg_y_index];
}

void CHIP8::OP_8XY1() {
  assert(opcode & 0x8001);
  uint8_t reg_x_index = get_reg_x_index();
  uint8_t reg_y_index = get_reg_y_index();
  registers[reg_x_index] |= registers[reg_y_index];
}

void CHIP8::OP_8XY2() {
  assert(opcode & 0x8002);
  uint8_t reg_x_index = get_reg_x_index();
  uint8_t reg_y_index = get_reg_y_index();
  registers[reg_x_index] &= registers[reg_y_index];
}

void CHIP8::OP_8XY3() {
  assert(opcode & 0x8003);
  uint8_t reg_x_index = get_reg_x_index();
  uint8_t reg_y_index = get_reg_y_index();
  registers[reg_x_index] ^= registers[reg_y_index];
}

void CHIP8::OP_8XY4() {
  assert(opcode & 0x8004);
  uint8_t reg_x_index = get_reg_x_index();
  uint8_t reg_y_index = get_reg_y_index();

  uint16_t sum = registers[reg_x_index] + registers[reg_y_index];
  if (sum > 255) {
    registers[0xf] = 1;
  } else {
    registers[0xf] = 0;
  }
  registers[reg_x_index] = sum & 0xff; // only take the lower byte
}

void CHIP8::OP_8XY5() {
  assert(opcode & 0x8005);
  uint8_t reg_x_index = get_reg_x_index();
  uint8_t reg_y_index = get_reg_y_index();

  uint8_t val_x = registers[reg_x_index];
  uint8_t val_y = registers[reg_y_index];
  if (val_x > val_y) {
    registers[0xf] = 1;
  } else {
    registers[0xf] = 0;
  }
  registers[reg_x_index] = val_x - val_y;
}

void CHIP8::OP_8XY6() {
  assert(opcode & 0x8006);
  uint8_t reg_x_index = get_reg_x_index();
  registers[0xf] = registers[reg_x_index] & 0x1;
  registers[reg_x_index] >>= 1;
}

void CHIP8::OP_8XY7() {
  assert(opcode & 0x8007);
  uint8_t reg_x_index = get_reg_x_index();
  uint8_t reg_y_index = get_reg_y_index();

  uint8_t val_x = registers[reg_x_index];
  uint8_t val_y = registers[reg_y_index];
  if (val_y > val_x) {
    registers[0xf] = 1;
  } else {
    registers[0xf] = 0;
  }
  registers[reg_x_index] = (BYTE)(val_y - val_x);
}

void CHIP8::OP_8XYE() {
  assert(opcode & 0x800e);
  uint8_t reg_x_index = get_reg_x_index();
  registers[0xf] = (registers[reg_x_index] & 0x80) >> 7; // get MSB
  registers[reg_x_index] <<= 1;
}

void CHIP8::OP_9XY0() {
  assert(opcode & 0x9000);
  uint8_t reg_x_index = get_reg_x_index();
  uint8_t reg_y_index = get_reg_y_index();
  if (registers[reg_x_index] != registers[reg_y_index]) {
    program_counter += 2;
  }
}

void CHIP8::OP_ANNN() {
  assert(opcode & 0xa000);
  address_i = opcode & 0x0fff;
}

void CHIP8::OP_BNNN() {
  assert(opcode & 0xb000);
  program_counter = registers[0] + get_nnn();
}

void CHIP8::OP_CXKK() {
  assert(opcode & 0xc000);
  uint8_t reg_x_index = get_reg_x_index();
  uint8_t kk = get_kk();
  registers[reg_x_index] = rand_byte(randGen) & kk;
}

void CHIP8::OP_DXYN() {
  assert(opcode & 0xd000);
  uint8_t reg_x_index = get_reg_x_index();
  uint8_t reg_y_index = get_reg_y_index();
  uint8_t height = opcode & 0x000f;

  uint8_t x_pos = registers[reg_x_index] % WIDTH;
  uint8_t y_pos = registers[reg_y_index] % HEIGHT;

  registers[0xf] = 0; // collision = false

  for (int y = 0; y < height; y++) {
    BYTE data = memory[address_i + y];
    for (int x = 0; x < 8; x++) {
      uint8_t sprite_pixel = data & (0b1 << (8 - x - 1));
      uint32_t screen_pixel = screen[y_pos + y][x_pos + x];
      if (sprite_pixel) {
        if (screen_pixel) {
          registers[0xf] = 1; // collision = true
        }
        screen[y_pos + y][x_pos + x] ^= 0xffffffff;
      }
    }
  }
}

void CHIP8::OP_EX9E() {
  assert(opcode & 0xe09e);
  uint8_t reg_x_index = get_reg_x_index();
  BYTE key = registers[reg_x_index];
  if (keypad[key]) {
    program_counter += 2;
  }
}

void CHIP8::OP_EXA1() {
  assert(opcode & 0xe0a1);
  uint8_t reg_x_index = get_reg_x_index();
  BYTE key = registers[reg_x_index];
  if (!keypad[key]) {
    program_counter += 2;
  }
}

void CHIP8::OP_FX07() {
  assert(opcode & 0xf007);
  uint8_t reg_x_index = get_reg_x_index();
  registers[reg_x_index] = delay_timer;
}

void CHIP8::OP_FX0A() {
  assert(opcode & 0xf00a);
  uint8_t reg_x_index = get_reg_x_index();
  if (std::any_of(keypad.begin(), keypad.end(),
                  [](bool keydown) { return keydown; })) {
    auto it = std::find(keypad.begin(), keypad.end(), 1);
    registers[reg_x_index] = std::distance(keypad.begin(), it);
  } else {
    program_counter -= 2;
  }
}

void CHIP8::OP_FX15() {
  assert(opcode & 0xf015);
  uint8_t reg_x_index = get_reg_x_index();
  delay_timer = registers[reg_x_index];
}

void CHIP8::OP_FX18() {
  assert(opcode & 0xf018);
  uint8_t reg_x_index = get_reg_x_index();
  sound_timer = registers[reg_x_index];
}

void CHIP8::OP_FX1E() {
  assert(opcode & 0xf01e);
  uint8_t reg_x_index = get_reg_x_index();
  address_i += registers[reg_x_index];
}

void CHIP8::OP_FX29() {
  assert(opcode & 0xf029);
  uint8_t reg_x_index = get_reg_x_index();
  address_i = FONTSET_START_ADDRESS + registers[reg_x_index] * 5;
}

void CHIP8::OP_FX33() {
  assert(opcode & 0xf033);
  uint8_t reg_x_index = get_reg_x_index();
  BYTE value = registers[reg_x_index];

  memory[address_i + 2] = value % 10;
  value /= 10;

  memory[address_i + 1] = value % 10;
  value /= 10;

  memory[address_i] = value % 10;
}

void CHIP8::OP_FX55() {
  assert(opcode & 0xf055);
  uint8_t reg_x_index = get_reg_x_index();

  std::copy(registers.begin(), registers.begin() + reg_x_index + 1,
            memory.begin() + address_i);
}

void CHIP8::OP_FX65() {
  assert(opcode & 0xf065);
  uint8_t reg_x_index = get_reg_x_index();

  std::copy(memory.begin() + address_i,
            memory.begin() + address_i + reg_x_index + 1, registers.begin());
}
