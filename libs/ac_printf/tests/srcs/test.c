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

#include <ac_string.h>
#include <ac_printf.h>
#include <ac_debug_printf.h>

typedef struct {
    int idx;
    char buff[256];
} Buffer;

/**
 * Return AC_TRUE if the expr was not true signifying a failure
 */
static inline ac_bool test_failure(
    ac_bool expr, const char* exprStrg, const char* function,
    const char* file, const int lineNumber) {
  ac_bool failure = !expr;
  if (failure) {
    if (ac_strlen(exprStrg) == 0) {
      ac_printf("FAILURE at func=%s file=%s:%d\n",
                  function, file, lineNumber);
    } else {
      ac_printf("FAILURE '%s'.  func=%s file=%s:%d\n",
                  exprStrg, function, file, lineNumber);
    }
    failure = AC_TRUE;
  } else {
    failure = AC_FALSE;
  }
  return failure;
}

/**
 * Macro which returns AC_TRUE if the expr did NOT evaluate to true
 */
#define TEST(expr) test_failure((ac_bool)(expr), #expr, __FUNCTION__, __FILE__, __LINE__)

#define FAIL(str) test_failure(AC_FALSE, str, __FUNCTION__, __FILE__, __LINE__)

/**
 * get_buff called after printing
 */
static const char* get_buff(ac_writer* this) {
  Buffer *pBuffer = (Buffer*)this->data;
  return pBuffer->buff;
}

/**
 * write_beg called before first write_char_to_buff
 */
static void write_beg(ac_writer* this) {
  Buffer *pBuffer = (Buffer*)this->data;
  pBuffer->idx = 0;
}

/**
 * write_end called after the last write_char_to_buff
 */
static void write_end(ac_writer* this) {
  Buffer *pBuffer = (Buffer*)this->data;
  pBuffer->buff[pBuffer->idx] = 0;
}

/**
 * Write a character to the buffer
 */
static void write_char_to_buff(ac_writer* this, void* param) {
  Buffer *pBuffer = (Buffer*)this->data;
  char ch = ((char)(((ac_uptr)param) & 0xff));
  pBuffer->buff[pBuffer->idx++] = ch;
  if (pBuffer->idx >= sizeof(pBuffer->buff)) {
    pBuffer->idx = 0;
  }
}

static inline ac_bool test_printing_result(
    const char* expected, Buffer *pBuffer,
    const char* function, const char* file, const int lineNumber) {
  ac_bool failure = AC_FALSE;
  if (pBuffer->idx != ac_strlen(expected)) {
    ac_printf("buff:%s != expected:%s idx=%d\n",
        pBuffer->buff, expected, pBuffer->idx);
    failure |= test_failure(pBuffer->idx == ac_strlen(expected),
        "", function, file, lineNumber);
  } else if (ac_strncmp(expected, pBuffer->buff, pBuffer->idx) != 0) {
    ac_printf("buff:%s != expected:%s idx=%d\n",
        pBuffer->buff, expected, pBuffer->idx);
    failure |= test_failure(ac_strncmp(expected, pBuffer->buff,
          pBuffer->idx) == 0, "", function, file, lineNumber);
  }
  return failure;
}

#define TEST_PRINTING_NO_PARAM(formatting, expectedVal) ({ \
  ac_bool failure; \
  ac_printfw(&writer, formatting); \
  failure = test_printing_result(expectedVal, (Buffer*)(writer.data), \
      __FUNCTION__, __FILE__, __LINE__); \
  failure; \
})

#define TEST_PRINTING(formatting, param, expectedVal) ({ \
    ac_bool failure; \
    ac_printfw(&writer, formatting, param); \
    failure = test_printing_result(expectedVal, (Buffer*)(writer.data), \
        __FUNCTION__, __FILE__, __LINE__); \
    failure; \
})


int main(void) {
  ac_bool failure = AC_FALSE;
  ac_printf("Hello, World\n");
  ac_debug_printf("Hello, World: via ac_debug_printf\n");

  // Define a buffer and a writer for testing printf
  Buffer buffer;
  ac_writer writer = {
          .write_beg = write_beg,
          .write_param = write_char_to_buff,
          .write_end = write_end,
          .get_buff = get_buff,
          .data = &buffer
  };

  failure |= TEST_PRINTING_NO_PARAM("Hello", "Hello");
  failure |= TEST_PRINTING_NO_PARAM("%", "");
  failure |= TEST_PRINTING_NO_PARAM("%1", "%1");
  failure |= TEST_PRINTING_NO_PARAM("%%", "%");

  failure |= TEST_PRINTING("%s", "string", "string");
  failure |= TEST_PRINTING("%b", 0, "0");
  failure |= TEST_PRINTING("%b", 0x87654321,
      "10000111011001010100001100100001");
  failure |= TEST_PRINTING("%b", 0xFFFFFFFF,
      "11111111111111111111111111111111");

  failure |= TEST_PRINTING("%d", 1, "1");
  failure |= TEST_PRINTING("%d", 2147483647, "2147483647");

  // In printf statements constant negative numbers must be cast
  // so they work both 32 and 64 bit environments.
  failure |= TEST_PRINTING("%d", AC_SINT(-2147483647), "-2147483647");
  failure |= TEST_PRINTING("%d", (ac_sint)-2147483648, "-2147483648");
  failure |= TEST_PRINTING("%d", (ac_int)-1, "-1");

  failure |= TEST_PRINTING("%u", 2, "2");
  failure |= TEST_PRINTING("%u", 0x7FFFFFFF, "2147483647");
  failure |= TEST_PRINTING("%u", 0x80000000, "2147483648");
  failure |= TEST_PRINTING("%u", 0x80000001, "2147483649");
  failure |= TEST_PRINTING("%u", 0xFFFFFFFF, "4294967295");
  failure |= TEST_PRINTING("%u", -1, "4294967295");

  failure |= TEST_PRINTING("%x", 0, "0");
  failure |= TEST_PRINTING("%x", 9, "9");
  failure |= TEST_PRINTING("%x", 10, "a");
  failure |= TEST_PRINTING("%x", 15, "f");
  failure |= TEST_PRINTING("%x", 16, "10");
  failure |= TEST_PRINTING("0x%x", 0x12345678, "0x12345678");
  failure |= TEST_PRINTING("0x%x", 0x9abcdef0, "0x9abcdef0");

  if (sizeof(ac_uint) == sizeof(ac_u64)) {
    // Test big positive and negative numbers on 64 bit systems
    // But casting isn't necessary, but works.
    failure |= TEST_PRINTING("%b", 0x8765432187654321,
        "1000011101100101010000110010000110000111011001010100001100100001");
    failure |= TEST_PRINTING("%b", 0xFFFFFFFFFFFFFFFF,
        "1111111111111111111111111111111111111111111111111111111111111111");

    failure |= TEST_PRINTING("%d",
        (ac_sint)0x7FFFFFFFFFFFFFFF, "9223372036854775807");
    failure |= TEST_PRINTING("%d",
        (ac_int)0x8000000000000000, "-9223372036854775808");
    failure |= TEST_PRINTING("%d",
        0x7FFFFFFFFFFFFFFF, "9223372036854775807");
    failure |= TEST_PRINTING("%d",
        0x8000000000000000, "-9223372036854775808");


    failure |= TEST_PRINTING("%u",
        (ac_uint)0x7FFFFFFFFFFFFFFF, "9223372036854775807");
    failure |= TEST_PRINTING("%u",
        AC_UINT(0x8000000000000000), "9223372036854775808");
    failure |= TEST_PRINTING("%u",
        (ac_uint)0xFFFFFFFFFFFFFFFF, "18446744073709551615");
    failure |= TEST_PRINTING("%u",
        0x7FFFFFFFFFFFFFFF, "9223372036854775807");
    failure |= TEST_PRINTING("%u",
        0x8000000000000000, "9223372036854775808");
    failure |= TEST_PRINTING("%u",
        0xFFFFFFFFFFFFFFFF, "18446744073709551615");

    failure |= TEST_PRINTING("%u", (ac_uint)-1, "18446744073709551615");
    failure |= TEST_PRINTING("%u", AC_UINT(-1), "18446744073709551615");

    failure |= TEST_PRINTING("%x",
        (ac_uint)0x7FFFFFFFFFFFFFFF, "7fffffffffffffff");
    failure |= TEST_PRINTING("%x",
        AC_UINT(0x8000000000000000), "8000000000000000");
    failure |= TEST_PRINTING("%x",
        (ac_uint)0xFFFFFFFFFFFFFFFF, "ffffffffffffffff");
    failure |= TEST_PRINTING("%x",
        0x7FFFFFFFFFFFFFFF, "7fffffffffffffff");
    failure |= TEST_PRINTING("%x",
        0x8000000000000000, "8000000000000000");
    failure |= TEST_PRINTING("%x",
        0xFFFFFFFFFFFFFFFF, "ffffffffffffffff");
  }


  failure |= TEST_PRINTING_NO_PARAM("%l", "%l");
  failure |= TEST_PRINTING_NO_PARAM("%la", "%la");
  failure |= TEST_PRINTING_NO_PARAM("%ll", "%ll");
  failure |= TEST_PRINTING_NO_PARAM("%llz1", "%llz1");

  failure |= TEST_PRINTING("%llx", 0ll, "0");
  failure |= TEST_PRINTING("%llx", 9ll, "9");
  failure |= TEST_PRINTING("%x", 10ll, "a");
  failure |= TEST_PRINTING("%x", 15ll, "f");
  failure |= TEST_PRINTING("%llx", 16ll, "10");

  // Test we are "filling" zeros correctly as we print using write_u32 twice
  failure |= TEST_PRINTING("%llx", 0x80000000ll, "80000000");
  failure |= TEST_PRINTING("%llx", 0x800000000ll, "800000000");
  failure |= TEST_PRINTING("%llx", 0x800000000000ll, "800000000000");
  failure |= TEST_PRINTING("%llx", 0x80000000000000ll, "80000000000000");
  failure |= TEST_PRINTING("%llx", 0x8000000000000000ll,
      "8000000000000000");
  failure |= TEST_PRINTING("%llx", 0x8000000000000001ll,
      "8000000000000001");

  failure |= TEST_PRINTING("%llx", 0xFEDCBA9876543210ll,
      "fedcba9876543210");
  failure |= TEST_PRINTING("%llx", 0x7fffffffffffffffll,
      "7fffffffffffffff");
  failure |= TEST_PRINTING("%llx", -1ll, "ffffffffffffffff");

  if (sizeof(ac_uptr) == sizeof(ac_u32)) {
      void *pv = (void *)0xf7654321;
      failure |= TEST_PRINTING("%p", pv, "f7654321");
  } else if (sizeof(ac_uptr) == sizeof(ac_u64)) {
      void *pv = (void *)0xf7654321f7654321;
      failure |= TEST_PRINTING("%p", pv, "f7654321f7654321");
  } else {
      failure |= FAIL(ac_formatter(
            &writer, "Unexptected sizeof(ac_uptr)=%d", sizeof(ac_uptr)));
  }

  if (sizeof(ac_uint) == sizeof(ac_u32)) {
      ac_uint i = 0xf7654321;
      failure |= TEST_PRINTING("%x", i, "f7654321");
  } else if (sizeof(ac_uint) == sizeof(ac_u64)) {
      ac_uint i = (ac_uint)0xf7654321f7654321;
      failure |= TEST_PRINTING("%x", i, "f7654321f7654321");
  } else {
      failure |= FAIL(ac_formatter(
            &writer, "Unexptected sizeof(ac_uint)=%d", sizeof(ac_uint)));
  }

  if (!failure) {
    ac_printf("OK\n");
  }
  return failure;
}

