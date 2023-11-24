#include <gtest/gtest.h>
#include <iostream>

#include "../chip/chip8.h"

TEST(TestSetup, BasicAssertions) { EXPECT_EQ(1, 1); }

class CHIP8Test : public testing::Test {
public:
  CHIP8 chip;

protected:
  void SetUp() override {
    chip = CHIP8();
    chip.reset();
  }
};

TEST_F(CHIP8Test, TestRegistersInit) {
  chip.reset();
  for (auto iter = chip.registers.begin(); iter != chip.registers.end();
       iter++) {
    ASSERT_EQ(*iter, 0);
  }
}

TEST_F(CHIP8Test, TestOP_00E0) {
  chip.opcode = 0x00e0;
  chip.OP_00E0();
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 64; x++) {
      ASSERT_EQ(chip.screen[y][x], 0);
    }
    std::cout << std::endl;
  }
}

TEST_F(CHIP8Test, TestOP_00EE) {
  chip.stack.push_back(0x200);
  chip.opcode = 0x00ee;
  chip.OP_00EE();
  ASSERT_EQ(chip.program_counter, 0x200);

  chip.stack.push_back(0x200);
  chip.stack.push_back(0x400);
  chip.program_counter = 0x200;
  chip.opcode = 0x00ee;
  chip.OP_00EE();
  ASSERT_EQ(chip.program_counter, 0x400);
  ASSERT_EQ(chip.stack.back(), 0x200);
}

TEST_F(CHIP8Test, TestOP_1NNN) {
  chip.opcode = 0x1200;
  chip.OP_1NNN();
  ASSERT_EQ(chip.program_counter, 0x200);

  chip.opcode = 0x1323;
  chip.OP_1NNN();
  ASSERT_EQ(chip.program_counter, 0x323);
}

TEST_F(CHIP8Test, TestOP_2NNN) {
  chip.program_counter = 0x200;
  chip.program_counter += 2; // cycle
  chip.opcode = 0x2200;
  chip.OP_2NNN();
  ASSERT_EQ(chip.program_counter, 0x200);
  ASSERT_EQ(chip.stack.back(), 0x202);

  chip.program_counter = 0x400;
  chip.program_counter += 2; // cycle
  chip.opcode = 0x2323;
  chip.OP_2NNN();
  ASSERT_EQ(chip.program_counter, 0x323);
  ASSERT_EQ(chip.stack.back(), 0x402);
}

TEST_F(CHIP8Test, TestOP_3XKK) {
  chip.program_counter = 0x300;
  chip.registers[0] = 1;
  chip.opcode = 0x3001;
  chip.OP_3XKK();
  ASSERT_EQ(chip.program_counter, 0x302);

  chip.program_counter = 0x300;
  chip.registers[0] = 1;
  chip.opcode = 0x3002;
  chip.OP_3XKK();
  ASSERT_EQ(chip.program_counter, 0x300);
}

TEST_F(CHIP8Test, TestOP_4XKK) {
  chip.program_counter = 0x300;
  chip.registers[0] = 1;
  chip.opcode = 0x4001;
  chip.OP_4XKK();
  ASSERT_EQ(chip.program_counter, 0x300);

  chip.program_counter = 0x300;
  chip.registers[0] = 1;
  chip.opcode = 0x4002;
  chip.OP_4XKK();
  ASSERT_EQ(chip.program_counter, 0x302);
}

TEST_F(CHIP8Test, TestOP_5XY0) {
  chip.program_counter = 0x300;
  chip.opcode = 0x5010;
  chip.registers[0] = 1;
  chip.registers[1] = 1;
  chip.OP_5XY0();
  ASSERT_EQ(chip.program_counter, 0x302);

  chip.program_counter = 0x300;
  chip.opcode = 0x5010;
  chip.registers[0] = 1;
  chip.registers[1] = 2;
  chip.OP_5XY0();
  ASSERT_EQ(chip.program_counter, 0x300);
}

TEST_F(CHIP8Test, TestOP_6XKK) {
  chip.opcode = 0x6133;
  chip.OP_6XKK();
  ASSERT_EQ(chip.registers[1], 0x33);
}

TEST_F(CHIP8Test, TestOP_7XKK) {
  chip.reset();
  chip.opcode = 0x71ff;
  chip.OP_7XKK();
  ASSERT_EQ(chip.registers[1], 0xff);

  chip.opcode = 0x71ff;
  chip.OP_7XKK();
  ASSERT_EQ(chip.registers[1], 0xfe);

  chip.reset();
  chip.registers[1] = 0xff;
  chip.opcode = 0x7101;
  chip.OP_7XKK();
  ASSERT_EQ(chip.registers[1], 0x00);
}

TEST_F(CHIP8Test, TestOP_8XY0) {
  chip.reset();
  chip.registers[1] = 0x20;
  chip.registers[3] = 0x03;

  chip.opcode = 0x8130;
  chip.OP_8XY0();
  ASSERT_EQ(chip.registers[1], 0x03);
  ASSERT_EQ(chip.registers[3], 0x03);
}

TEST_F(CHIP8Test, TestOP_8XY1) {
  chip.reset();
  chip.registers[1] = 0x20;
  chip.registers[3] = 0x03;

  chip.opcode = 0x8131;
  chip.OP_8XY1();
  ASSERT_EQ(chip.registers[1], 0x23);
  ASSERT_EQ(chip.registers[3], 0x03);

  chip.reset();
  chip.registers[1] = 0xf0;
  chip.registers[3] = 0x0f;

  chip.opcode = 0x8131;
  chip.OP_8XY1();
  ASSERT_EQ(chip.registers[1], 0xff);
  ASSERT_EQ(chip.registers[3], 0x0f);
}

TEST_F(CHIP8Test, TestOP_8XY2) {
  chip.reset();
  chip.registers[1] = 0x20;
  chip.registers[3] = 0x03;

  chip.opcode = 0x8132;
  chip.OP_8XY2();
  ASSERT_EQ(chip.registers[1], 0x00);
  ASSERT_EQ(chip.registers[3], 0x03);

  chip.reset();
  chip.registers[1] = 0x11;
  chip.registers[3] = 0x0f;

  chip.opcode = 0x8132;
  chip.OP_8XY2();
  ASSERT_EQ(chip.registers[1], 0x1);
  ASSERT_EQ(chip.registers[3], 0x0f);
}

TEST_F(CHIP8Test, TestOP_8XY3) {
  chip.reset();
  chip.registers[1] = 0x20;
  chip.registers[3] = 0x03;

  chip.opcode = 0x8133;
  chip.OP_8XY3();
  ASSERT_EQ(chip.registers[1], 0x23);
  ASSERT_EQ(chip.registers[3], 0x03);

  chip.reset();
  chip.registers[1] = 0x11;
  chip.registers[3] = 0x0f;

  chip.opcode = 0x8133;
  chip.OP_8XY3();
  ASSERT_EQ(chip.registers[1], 0x1e);
  ASSERT_EQ(chip.registers[3], 0x0f);
}

TEST_F(CHIP8Test, TestOP_8XY4) {
  chip.reset();
  chip.registers[0] = 0x1;
  chip.registers[1] = 0x2;
  chip.opcode = 0x8014;
  chip.OP_8XY4();
  ASSERT_EQ(chip.registers[0], 0x3);

  chip.registers[0] = 0xff;
  chip.registers[1] = 0x01;
  chip.opcode = 0x8014;
  chip.OP_8XY4();
  ASSERT_EQ(chip.registers[0], 0x00);
  ASSERT_EQ(chip.registers[0xf], 0x1);

  chip.registers[0] = 0xff;
  chip.registers[1] = 0xff;
  chip.opcode = 0x8014;
  chip.OP_8XY4();
  ASSERT_EQ(chip.registers[0], 0xfe);
  ASSERT_EQ(chip.registers[0xf], 0x1);
}

TEST_F(CHIP8Test, TestOP_8XY5) {
  chip.reset();
  chip.registers[0] = 0x2;
  chip.registers[1] = 0x1;
  chip.opcode = 0x8015;
  chip.OP_8XY5();
  ASSERT_EQ(chip.registers[0], 0x1);
  ASSERT_EQ(chip.registers[0xf], 0x1);

  chip.registers[0] = 0xff;
  chip.registers[1] = 0x01;
  chip.opcode = 0x8015;
  chip.OP_8XY5();
  ASSERT_EQ(chip.registers[0], 0xfe);
  ASSERT_EQ(chip.registers[0xf], 0x1);

  chip.registers[0] = 0xff;
  chip.registers[1] = 0xff;
  chip.opcode = 0x8015;
  chip.OP_8XY5();
  ASSERT_EQ(chip.registers[0], 0x00);
  ASSERT_EQ(chip.registers[0xf], 0x0);

  chip.registers[0] = 0x00;
  chip.registers[1] = 0x01;
  chip.opcode = 0x8015;
  chip.OP_8XY5();
  ASSERT_EQ(chip.registers[0], 0xff);
  ASSERT_EQ(chip.registers[0xf], 0x0);
}

TEST_F(CHIP8Test, TestOP_8XY6) {
  chip.reset();
  chip.registers[1] = 0xaf;
  chip.opcode = 0x8106;
  chip.OP_8XY6();
  ASSERT_EQ(chip.registers[1], 0x57);
  ASSERT_EQ(chip.registers[0xf], 0x1);

  chip.registers[1] = 0xf0;
  chip.OP_8XY6();
  ASSERT_EQ(chip.registers[1], 0x78);
  ASSERT_EQ(chip.registers[0xf], 0x0);
}

TEST_F(CHIP8Test, TestOP_8XY7) {
  chip.reset();
  chip.registers[0] = 0x2;
  chip.registers[1] = 0x1;
  chip.opcode = 0x8017;
  chip.OP_8XY7();
  ASSERT_EQ(chip.registers[0], 0xff);
  ASSERT_EQ(chip.registers[0xf], 0x0);

  chip.registers[0] = 0xff;
  chip.registers[1] = 0x01;
  chip.opcode = 0x8017;
  chip.OP_8XY7();
  ASSERT_EQ(chip.registers[0], 0b00000010);
  ASSERT_EQ(chip.registers[0xf], 0x0);

  chip.registers[0] = 0xff;
  chip.registers[1] = 0xff;
  chip.opcode = 0x8017;
  chip.OP_8XY7();
  ASSERT_EQ(chip.registers[0], 0x0);
  ASSERT_EQ(chip.registers[0xf], 0x0);

  chip.registers[0] = 0x00;
  chip.registers[1] = 0x01;
  chip.opcode = 0x8017;
  chip.OP_8XY7();
  ASSERT_EQ(chip.registers[0], 0x1);
  ASSERT_EQ(chip.registers[0xf], 0x1);
}

TEST_F(CHIP8Test, TestOP_8XYE) {
  chip.reset();
  chip.registers[3] = 0xff;
  chip.opcode = 0x830e;
  chip.OP_8XYE();
  ASSERT_EQ(chip.registers[3], 0xfe);
  ASSERT_EQ(chip.registers[0xf], 0x01);

  chip.registers[3] = 0x00;
  chip.opcode = 0x830e;
  chip.OP_8XYE();
  ASSERT_EQ(chip.registers[3], 0x00);
  ASSERT_EQ(chip.registers[0xf], 0x0);

  chip.registers[3] = 0xef;
  chip.opcode = 0x830e;
  chip.OP_8XYE();
  ASSERT_EQ(chip.registers[3], 0xde);
  ASSERT_EQ(chip.registers[0xf], 0x01);
}

TEST_F(CHIP8Test, TestOP_9XY0) {
  chip.reset();
  chip.registers[1] = 0x1;
  chip.registers[2] = 0x1;

  chip.opcode = 0x9120;

  chip.program_counter = 0x250;
  chip.OP_9XY0();
  ASSERT_EQ(chip.program_counter, 0x250);

  chip.registers[2] = 0x2;
  chip.OP_9XY0();
  ASSERT_EQ(chip.program_counter, 0x252);
}

TEST_F(CHIP8Test, TestOP_ANNN) {
  chip.opcode = 0xa532;
  chip.OP_ANNN();
  ASSERT_EQ(chip.address_i, 0x532);
}

TEST_F(CHIP8Test, TestOP_BNNN) {
  chip.opcode = 0xa532;
  chip.program_counter = 0x200;
  chip.registers[0] = 0x11;

  chip.OP_BNNN();
  ASSERT_EQ(chip.program_counter, 0x543);
}

TEST_F(CHIP8Test, TestOP_CXKK) {
  // no real way to test
  chip.registers[0] = 0;
  chip.opcode = 0xc000;
  chip.OP_CXKK();
  ASSERT_EQ(chip.registers[0], 0);
}

TEST_F(CHIP8Test, TestOP_DXYN) {
  // test char '0'
  chip.address_i = 0x50; // address of char '0'
  chip.opcode = 0xd005;
  chip.OP_DXYN();
  uint32_t screen_pixel;
  BYTE sprite_pixel;
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 8; x++) {
      screen_pixel = chip.screen[y][x];
      sprite_pixel = fontset[y] & (0b10000000 >> x);
      sprite_pixel >>= (8 - x - 1);
      // std::cout << "Screen: " << (int) screen_pixel << " sprite: " << (int)
      // sprite_pixel << std::endl;
      if (sprite_pixel) {
        ASSERT_EQ(screen_pixel, 0xffffffff);
      } else {
        ASSERT_EQ(screen_pixel, 0);
      }
    }
  }

  chip.reset_screen();
  // test char '1'
  chip.address_i = 0x55; // address of char '1'
  chip.opcode = 0xd005;
  chip.OP_DXYN();
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 8; x++) {
      screen_pixel = chip.screen[y][x];
      sprite_pixel = fontset[y + 5] & (0b10000000 >> x);
      sprite_pixel >>= (8 - x - 1);
      if (sprite_pixel) {
        ASSERT_EQ(screen_pixel, 0xffffffff);
      } else {
        ASSERT_EQ(screen_pixel, 0);
      }
    }
  }

  // overwrite char '1'
  chip.address_i = 0x55; // address of char '1'
  chip.opcode = 0xd005;
  chip.OP_DXYN();
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 8; x++) {
      screen_pixel = chip.screen[y][x];
      sprite_pixel = fontset[y + 5] & (0b10000000 >> x);
      sprite_pixel >>= (8 - x - 1);
      if (sprite_pixel) {
        ASSERT_EQ(screen_pixel, 0);
      }
    }
  }
  ASSERT_EQ(chip.registers[0xf], 0x1);
}

TEST_F(CHIP8Test, TestOP_EX9E) {
  chip.registers[0] = 0;
  chip.keypad[0] = 0;
  chip.opcode = 0xe09e;
  chip.program_counter = 0x230;
  chip.OP_EX9E();
  ASSERT_EQ(chip.program_counter, 0x230);

  chip.keypad[0] = 1;
  chip.OP_EX9E();
  ASSERT_EQ(chip.program_counter, 0x232);
}

TEST_F(CHIP8Test, TestOP_EXA1) {
  chip.registers[0] = 0;
  chip.keypad[0] = 0;
  chip.opcode = 0xe0a1;
  chip.program_counter = 0x230;
  chip.OP_EXA1();
  ASSERT_EQ(chip.program_counter, 0x232);

  chip.keypad[0] = 1;
  chip.OP_EXA1();
  ASSERT_EQ(chip.program_counter, 0x232);
}

TEST_F(CHIP8Test, TestOP_FX07) {
  chip.delay_timer = 0x03;
  chip.opcode = 0xf707;
  chip.OP_FX07();
  ASSERT_EQ(chip.registers[7], 0x03);
}

TEST_F(CHIP8Test, TestOP_FX0A) {
  chip.delay_timer = 0x03;
  chip.opcode = 0xf00a;
  chip.program_counter = 0x202;
  chip.OP_FX0A();
  ASSERT_EQ(chip.program_counter, 0x200);

  chip.keypad[0] = 1;
  chip.program_counter = 0x202;
  chip.OP_FX0A();
  ASSERT_EQ(chip.program_counter, 0x202);

  chip.reset_keypad();
  chip.keypad[10] = 1;
  chip.program_counter = 0x202;
  chip.OP_FX0A();
  ASSERT_EQ(chip.program_counter, 0x202);
}

TEST_F(CHIP8Test, TestOP_FX15) {
  chip.delay_timer = 0x03;
  chip.opcode = 0xf315;
  chip.registers[3] = 0x20;
  chip.OP_FX15();
  ASSERT_EQ(chip.delay_timer, 0x20);
}

TEST_F(CHIP8Test, TestOP_FX18) {
  chip.sound_timer = 0x03;
  chip.opcode = 0xf318;
  chip.registers[3] = 0x20;
  chip.OP_FX18();
  ASSERT_EQ(chip.sound_timer, 0x20);
}

TEST_F(CHIP8Test, TestOP_FX1E) {
  chip.opcode = 0xf31e;
  chip.registers[3] = 0x5;
  chip.address_i = 0x234;
  chip.OP_FX1E();
  ASSERT_EQ(chip.address_i, 0x239);
}

TEST_F(CHIP8Test, TestOP_FX29) {
  chip.registers[2] = 0xa;
  chip.opcode = 0xf229;
  chip.OP_FX29();
  ASSERT_EQ(chip.address_i, 0x82 /* 0x50 + 50 */);
}

TEST_F(CHIP8Test, TestOP_FX33) {
  chip.registers[5] = 123;
  chip.opcode = 0xf533;
  chip.address_i = 0x250;

  chip.OP_FX33();
  ASSERT_EQ(chip.memory[chip.address_i], 1);
  ASSERT_EQ(chip.memory[chip.address_i + 1], 2);
  ASSERT_EQ(chip.memory[chip.address_i + 2], 3);
}

TEST_F(CHIP8Test, TestOP_FX55) {
  chip.registers[0] = 0x12;
  chip.registers[1] = 0x34;
  chip.registers[2] = 0x56;
  chip.registers[3] = 0x78;
  chip.registers[4] = 0x90;
  chip.registers[5] = 0x12;
  chip.address_i = 0x250;
  chip.memory[chip.address_i + 6] = 0xff;
  chip.opcode = 0xf555;

  chip.OP_FX55();
  ASSERT_EQ(chip.memory[chip.address_i], 0x12);
  ASSERT_EQ(chip.memory[chip.address_i + 1], 0x34);
  ASSERT_EQ(chip.memory[chip.address_i + 2], 0x56);
  ASSERT_EQ(chip.memory[chip.address_i + 3], 0x78);
  ASSERT_EQ(chip.memory[chip.address_i + 4], 0x90);
  ASSERT_EQ(chip.memory[chip.address_i + 5], 0x12);
  ASSERT_EQ(chip.memory[chip.address_i + 6], 0xff);
}

TEST_F(CHIP8Test, TestOP_FX65) {
  chip.address_i = 0x250;
  chip.memory[chip.address_i] = 0x12;
  chip.memory[chip.address_i + 1] = 0x34;
  chip.memory[chip.address_i + 2] = 0x56;
  chip.memory[chip.address_i + 3] = 0x78;
  chip.memory[chip.address_i + 4] = 0x90;
  chip.memory[chip.address_i + 5] = 0x01;
  chip.memory[chip.address_i + 6] = 0x99;
  chip.registers[6] = 0xff;
  chip.opcode = 0xf565;

  chip.OP_FX65();
  ASSERT_EQ(chip.registers[0], 0x12);
  ASSERT_EQ(chip.registers[1], 0x34);
  ASSERT_EQ(chip.registers[2], 0x56);
  ASSERT_EQ(chip.registers[3], 0x78);
  ASSERT_EQ(chip.registers[4], 0x90);
  ASSERT_EQ(chip.registers[5], 0x01);
  ASSERT_EQ(chip.registers[6], 0xff);
}
