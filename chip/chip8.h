#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

typedef unsigned char BYTE;
typedef unsigned short int WORD;

const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;
const int WIDTH = 64;
const int HEIGHT = 32;

const std::array<int, FONTSET_SIZE> fontset{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

const unsigned int START_ADDRESS = 0x200;

class CHIP8 {
private:
  typedef void (CHIP8::*CHIP8Func)();

  /**
   * Indexes by the unique part of the opcode
   */
  template <std::size_t Size>
  void call_table_by_opcode(std::array<CHIP8Func, Size> &t, size_t index) {
    if (index < t.size() && t[index]) {
      (this->*t[index])();
    } else {
      fprintf(stderr, "Index out of bounds or t[index] = nullptr");
    }
  }

  inline void init_main_table() {
    std::fill(table.begin(), table.end(), &CHIP8::OP_NULL);
    table[0x0] = &CHIP8::Table0;
    table[0x1] = &CHIP8::OP_1NNN;
    table[0x2] = &CHIP8::OP_2NNN;
    table[0x3] = &CHIP8::OP_3XKK;
    table[0x4] = &CHIP8::OP_4XKK;
    table[0x5] = &CHIP8::OP_5XY0;
    table[0x6] = &CHIP8::OP_6XKK;
    table[0x7] = &CHIP8::OP_7XKK;
    table[0x8] = &CHIP8::Table8;
    table[0x9] = &CHIP8::OP_9XY0;
    table[0xa] = &CHIP8::OP_ANNN;
    table[0xb] = &CHIP8::OP_BNNN;
    table[0xc] = &CHIP8::OP_CXKK;
    table[0xd] = &CHIP8::OP_DXYN;
    table[0xe] = &CHIP8::TableE;
    table[0xf] = &CHIP8::TableF;
  }

  inline void init_table0() {
    std::fill(table0.begin(), table0.end(), &CHIP8::OP_NULL);
    table0[0x0] = &CHIP8::OP_00E0;
    table0[0xe] = &CHIP8::OP_00EE;
  }

  inline void init_table8() {
    std::fill(table8.begin(), table8.end(), &CHIP8::OP_NULL);
    table8[0x0] = &CHIP8::OP_8XY0;
    table8[0x1] = &CHIP8::OP_8XY1;
    table8[0x2] = &CHIP8::OP_8XY2;
    table8[0x3] = &CHIP8::OP_8XY3;
    table8[0x4] = &CHIP8::OP_8XY4;
    table8[0x5] = &CHIP8::OP_8XY5;
    table8[0x6] = &CHIP8::OP_8XY6;
    table8[0x7] = &CHIP8::OP_8XY7;
    table8[0xe] = &CHIP8::OP_8XYE;
  }

  inline void init_tableE() {
    std::fill(tableE.begin(), tableE.end(), &CHIP8::OP_NULL);
    tableE[0x1] = &CHIP8::OP_EXA1;
    tableE[0xe] = &CHIP8::OP_EX9E;
  }

  inline void init_tableF() {
    std::fill(tableF.begin(), tableF.end(), &CHIP8::OP_NULL);
    tableF[0x07] = &CHIP8::OP_FX07;
    tableF[0x0a] = &CHIP8::OP_FX0A;
    tableF[0x15] = &CHIP8::OP_FX15;
    tableF[0x18] = &CHIP8::OP_FX18;
    tableF[0x1e] = &CHIP8::OP_FX1E;
    tableF[0x29] = &CHIP8::OP_FX29;
    tableF[0x33] = &CHIP8::OP_FX33;
    tableF[0x55] = &CHIP8::OP_FX55;
    tableF[0x65] = &CHIP8::OP_FX65;
  }

  inline void Table0() {
    call_table_by_opcode(table0, opcode & 0x000f); // 00E0 vs. 00EE
  }

  inline void Table8() {
    call_table_by_opcode(table8, opcode & 0x000f); // 8XY0, 8XY1, etc
  }

  inline void TableE() {
    call_table_by_opcode(tableE, opcode & 0x000f); // EXA1, EX9E
  }

  inline void TableF() {
    call_table_by_opcode(tableF, opcode & 0x00ff); // FX07, FX0A, etc
  }

  inline uint8_t get_reg_x_index() { return (opcode & 0x0f00) >> 8; }

  inline uint8_t get_reg_y_index() { return (opcode & 0x00f0) >> 4; }

  inline uint8_t get_kk() { return (opcode & 0x00ff); }

  inline WORD get_nnn() { return opcode & 0x0fff; }

public:
  std::array<BYTE, 4096> memory;
  std::array<BYTE, 16> registers;
  WORD address_i;
  WORD program_counter;
  std::vector<WORD> stack;
  BYTE delay_timer;
  BYTE sound_timer;
  WORD opcode;

  std::array<std::array<uint32_t, 64>, 32> screen;

  //  Keypad       Keyboard
  // +-+-+-+-+    +-+-+-+-+
  // |1|2|3|C|    |1|2|3|4|
  // +-+-+-+-+    +-+-+-+-+
  // |4|5|6|D|    |Q|W|E|R|
  // +-+-+-+-+ => +-+-+-+-+
  // |7|8|9|E|    |A|S|D|F|
  // +-+-+-+-+    +-+-+-+-+
  // |A|0|B|F|    |Z|X|C|V|
  // +-+-+-+-+    +-+-+-+-+
  // https://austinmorlan.com/posts/chip8_emulator/
  std::array<BYTE, 16> keypad;

  std::array<CHIP8Func, 0xf + 1> table; // indexes by leftmost digit
  std::array<CHIP8Func, 0xe + 1> table0;
  std::array<CHIP8Func, 0xe + 1> table8;
  std::array<CHIP8Func, 0xe + 1> tableE;
  std::array<CHIP8Func, 0x65 + 1> tableF;

  std::default_random_engine randGen;
  std::uniform_int_distribution<BYTE> rand_byte;

  CHIP8()
      : address_i(0), program_counter(0), delay_timer(0), sound_timer(0),
        opcode(0),
        randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
    program_counter = START_ADDRESS;

    std::copy(fontset.begin(), fontset.end(),
              memory.begin() + FONTSET_START_ADDRESS);

    rand_byte = std::uniform_int_distribution<BYTE>(0, 255U);

    init_main_table();
    init_table0();
    init_table8();
    init_tableE();
    init_tableF();

    reset();
    reset_screen();
    reset_keypad();
  }
  void reset();
  void reset_screen();
  void reset_keypad();
  void load_rom(const std::string filename);
  void cycle();
  // Do nothing
  void OP_NULL();
  // CLS
  void OP_00E0();
  // RET
  void OP_00EE();
  // JP nnn
  void OP_1NNN();
  // CALL nnn
  void OP_2NNN();
  // SE Vx, byte
  void OP_3XKK();
  // SNE Vx, byte
  void OP_4XKK();
  // SE Vx, Vy
  void OP_5XY0();
  // LD Vx, byte
  void OP_6XKK();
  // ADD Vx, byte
  void OP_7XKK();
  // LD Vx, Vy
  void OP_8XY0();
  // OR Vx, Vy
  void OP_8XY1();
  // AND Vx, Vy
  void OP_8XY2();
  // XOR Vx, Vy
  void OP_8XY3();
  // ADD Vx, Vy
  void OP_8XY4();
  // Sub Vx, Vy
  void OP_8XY5();
  /**
   * SHR Vx
   * We use the shift in place for Register X and move the shifted bit to Reg F
   */
  void OP_8XY6();
  // SUBN Vx, Vy
  void OP_8XY7();
  /**
   * SHL Vx {, Vy}
   * We use the shift in place for Register X and move the shifted bit to Reg F
   */
  void OP_8XYE();
  // SNE Vx, Vy
  void OP_9XY0();
  // LD I, addr
  void OP_ANNN();
  // JP V0, addr
  void OP_BNNN();
  // RND Vx, byte
  void OP_CXKK();
  // DRW Vx, Vy, nibble
  void OP_DXYN();
  // SKP Vx
  void OP_EX9E();
  // SKNP Vx
  void OP_EXA1();
  // Ld Vx, DT
  void OP_FX07();
  // LD Vx, K
  void OP_FX0A();
  // LD DT, Vx
  void OP_FX15();
  // LD ST, Vx
  void OP_FX18();
  // ADD I, Vx
  void OP_FX1E();
  // LD F, Vx
  void OP_FX29();
  // LD B, Vx
  void OP_FX33();
  // LD [I], Vx
  void OP_FX55();
  // LD Vx, I
  void OP_FX65();
};
