/*
 * Copyright 2015 Wink Saville
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ac_arg.h>
#include <ac_inttypes.h>
#include <ac_putchar.h>
#include <ac_string.h>

/**
 * Only assumes ac_putchar works
 */

/**
 * fmt is trivial single character format specification
 * s= char*   == "s"
 * 8= ac_u8   == 8
 * 6= ac_u16  == 16
 * 2= ac_u32  == 32
 * 4= ac_u64  == 64
 */
ac_u32 test1(const char* fmt, ...) {
    ac_va_list args;
    ac_u32 result = 0;

    ac_va_start(args, fmt);
    while(*fmt != 0) {
      switch(*fmt++) {
        case 's': {
          char* s = ac_va_arg(args, char*);
          if ((s[0] != 's') || (s[1] != 0)) {
            result |= 0x01;
          }
          break;
        }
        case '8': {
          /* ac_u8 are promoted to int when passed through '...' */
          int acU8 = ac_va_arg(args, int);
          if (acU8 != 8) {
            result |= 0x01;
          }
          break;
        }
        case '6': {
          /* ac_u16 are promoted to int when passed through '...' */
          int acU16 = ac_va_arg(args, int);
          if (acU16 != 16) {
            result |= 0x02;
          }
          break;
        }
        case '2': {
          ac_u8 acU32 = ac_va_arg(args, ac_u32);
          if (acU32 != 32) {
            result |= 0x04;
          }
          break;
        }
        case '4': {
          ac_u8 acU64 = ac_va_arg(args, ac_u64);
          if (acU64 != 64) {
            result |= 0x08;
          }
          break;
        }
        default: {
          result |= 0x80000000;
          break;
        }
      }
    }

    ac_va_end(args);

    return result;
}

int main(void) {
  const char* s = "s";
  const ac_u8 acU8 = 8;
  const ac_u16 acU16 = 16;
  const ac_u32 acU32 = 32; 
  const ac_u64 acU64 = 64;
  ac_u32 result = 0;
  ac_u32 r;


  r = test1("");
  result |= r;
  if (r != 0) {
    ac_putchar('0');
  }
  if (r != 0) {
    ac_putchar('1');
  }
  r = test1("s", s);
  result |= r;
  if (r != 0) {
    ac_putchar('2');
  }
  r = test1("8", acU8);
  result |= r;
  if (r != 0) {
    ac_putchar('3');
  }
  r = test1("6", acU16);
  result |= r;
  if (r != 0) {
    ac_putchar('4');
  }
  r = test1("2", acU32);
  result |= r;
  if (r != 0) {
    ac_putchar('5');
  }
  r = test1("4", acU64);
  result |= r;
  if (r != 0) {
    ac_putchar('6');
  }
  r = test1("8624s", acU8, acU16, acU32, acU64, s);
  result |= r;
  if (r != 0) {
    ac_putchar('7');
  }
  r = test1("s4268", s, acU64, acU32, acU16, acU8);
  result |= r;

  if (result == 0) {
    ac_putchar('O');
    ac_putchar('K');
    ac_putchar('\n');
  } else {
    ac_putchar('-');
    ac_putchar('E');
    ac_putchar('R');
    ac_putchar('R');
    ac_putchar('\n');
  }

  return result;
}
