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

#define NDEBUG

#include <ac_string.h>
#include <ac_printf.h>
#include <ac_debug_printf.h>
#include <ac_test.h>

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
      ac_printf("ERROR:+: at func=%s file=%s:%d\n",
                  function, file, lineNumber);
    } else {
      ac_printf("ERROR:+: '%s'.  func=%s file=%s:%d\n",
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
  this->count = 0;
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
  this->count += 1;
}

static inline ac_bool test_printing_result(
    const char* expected, Buffer *pBuffer,
    const char* function, const char* file, const int lineNumber) {
  ac_bool failure = AC_FALSE;
  if (pBuffer->idx != ac_strlen(expected)) {
    ac_printf("buff:'%s' != expected:'%s' idx=%d\n",
        pBuffer->buff, expected, pBuffer->idx);
    failure |= test_failure(pBuffer->idx == ac_strlen(expected),
        "", function, file, lineNumber);
  } else if (ac_strncmp(expected, pBuffer->buff, pBuffer->idx) != 0) {
    ac_printf("buff:'%s' != expected:'%s' idx=%d\n",
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

#define TEST_PRINTING_2_PARAMS(formatting, param1, param2, expectedVal) ({ \
    ac_bool failure; \
    ac_printfw(&writer, formatting, param1, param2); \
    failure = test_printing_result(expectedVal, (Buffer*)(writer.data), \
        __FUNCTION__, __FILE__, __LINE__); \
    failure; \
})

#define TEST_PRINTING_3_PARAMS(formatting, param1, param2, param3, expectedVal) ({ \
    ac_bool failure; \
    ac_printfw(&writer, formatting, param1, param2, param3); \
    failure = test_printing_result(expectedVal, (Buffer*)(writer.data), \
        __FUNCTION__, __FILE__, __LINE__); \
    failure; \
})


static ac_u32 printf_ff_format_proc(ac_writer* writer, ac_u8 ch, ac_va_list args) {
  ac_printf_write_sval(writer, ac_va_arg(args, ac_u64), sizeof(ac_u64), 10);
  ac_u32 consumed = sizeof(ac_u64) / sizeof(ac_uint);
  ac_debug_printf("printf_ff_format_proc ch=%c count=%d consumed=%d\n",
      ch, writer->count, consumed);
  return consumed;
}

static ac_u32 printf_format_proc_s64(ac_writer* writer, const char* str, ac_va_list args) {
  ac_printf_write_sval(writer, ac_va_arg(args, ac_u64), sizeof(ac_u64), 10);
  ac_u32 consumed = sizeof(ac_u64) / sizeof(ac_uint);
  ac_debug_printf("printf_format_proc_s64 str=%s count=%d consumed=%d\n",
      str, writer->count, consumed);
  return consumed;
}

int main(void) {
  ac_bool failure = AC_FALSE;
  ac_uint count;
  Buffer buffer;

  // Test ac_printf
  count = ac_printf("Hello, World\n");
  failure |= AC_TEST(count == 13);
  ac_debug_printf("Hello, World: via ac_debug_printf\n");

  // Test ac_printfw with the default writer not initialized
  // So count should be 26
  count = ac_printfw(AcPrintf_get_writer(), "Hello, World\n");
  failure |= AC_TEST(count == 26);

  // Test ac_printfw with the default writer initialized
  count = ac_printfw(AcPrintf_get_writer_inited(), "Hello, World\n");
  failure |= AC_TEST(count == 13);

  // A very simple writer, all uninitialized fields will be 0
  // and for this code specifically simple_writer.count is 0.
  // Note since no write_beg it can be only used once and we
  // have to manully initialize buffer.idx. Also, since there
  // is no write_end there will be no terminating 0 of the buffer,
  // so this is BAD and only for testing that we can leave all
  // funtions empty except write_param.
  ac_writer simple_writer = {
          .write_param = write_char_to_buff,
          .data = &buffer,
  };
  buffer.idx = 0;
  count = ac_printfw(&simple_writer, "hi");
  failure |= AC_TEST(count == 2);
  failure |= AC_TEST(ac_strncmp(buffer.buff, "hi", 2) == 0);  

  // Test that with no get_buff an empty string is returned.
  // The wirter will be properly initialized and we'll have
  // a 0 at the end of the buffer so lie about the length
  // in the strncmp.
  ac_writer better_writer = {
          .write_beg = write_beg,
          .write_param = write_char_to_buff,
          .write_end = write_end,
          .data = &buffer,
  };
  const char* chars = ac_formatter(&better_writer, "bye");
  failure |= AC_TEST(better_writer.count == 3);
  failure |= AC_TEST(ac_strncmp(buffer.buff, "bye", 10) == 0);  
  failure |= AC_TEST(ac_strncmp(chars, "", 10) == 0);  

  // Define a buffer and a writer for testing printf
  ac_writer writer = {
          .write_beg = write_beg,
          .write_param = write_char_to_buff,
          .write_end = write_end,
          .get_buff = get_buff,
          .data = &buffer
  };

  failure |= TEST_PRINTING_NO_PARAM("Hello", "Hello");
  failure |= TEST_PRINTING_NO_PARAM("%", "");
  failure |= TEST_PRINTING_NO_PARAM("%a", "%a");
  failure |= TEST_PRINTING_NO_PARAM("%%", "%");

  failure |= TEST_PRINTING("%s", "string", "string");
  failure |= TEST_PRINTING("%b", 0, "0");
  failure |= TEST_PRINTING("%b", 0x87654321,
      "10000111011001010100001100100001");
  failure |= TEST_PRINTING("%b", 0xFFFFFFFF,
      "11111111111111111111111111111111");

  failure |= TEST_PRINTING("%d", 1, "1");
  failure |= TEST_PRINTING("%d", 2147483647, "2147483647");

  // Always print sign

  failure |= TEST_PRINTING("%d", 123, "123");
  failure |= TEST_PRINTING("%+d", 123, "+123");
  failure |= TEST_PRINTING("%+d", -123, "-123");


  // Alternate form does nothing
  failure |= TEST_PRINTING("%#d", 123, "123");

  // Width specifications
  failure |= TEST_PRINTING("%2d", 123, "123");
  failure |= TEST_PRINTING("%3d", 123, "123");
  failure |= TEST_PRINTING("%4d", 123, " 123");
  failure |= TEST_PRINTING("%5d", 123, "  123");

  failure |= TEST_PRINTING("0x%1x", 0x1, "0x1");
  failure |= TEST_PRINTING("0x%02x", 0x1, "0x01");
  failure |= TEST_PRINTING("0x%04x", 0x1, "0x0001");
  switch (sizeof(void*)) {
    case 4: failure |= TEST_PRINTING("0x%p", 0x1, "0x00000001"); break;
    case 8: failure |= TEST_PRINTING("0x%p", 0x1, "0x0000000000000001"); break;
    default: failure = AC_TEST(sizeof(void*) != 4 || sizeof(void*) != 8); break;
  }

  // Width & Leading zeros
  failure |= TEST_PRINTING("%02d", 123,  "123");
  failure |= TEST_PRINTING("%03d", 123,  "123");
  failure |= TEST_PRINTING("%04d", 123,  "0123");
  failure |= TEST_PRINTING("%05d", 123,  "00123");
  failure |= TEST_PRINTING("%05d", -123, "-0123");
  failure |= TEST_PRINTING("%06d", -123, "-00123");

  // Width & left justification
  failure |= TEST_PRINTING("%-3d", 123,  "123");
  failure |= TEST_PRINTING("%-3d", -123, "-123");
  failure |= TEST_PRINTING("%-4d", 123,  "123 ");
  failure |= TEST_PRINTING("%-4d", -123, "-123");
  failure |= TEST_PRINTING("%-5d", 123,  "123  ");
  failure |= TEST_PRINTING("%-5d", -123, "-123 ");

  // Width & left justificaton & leading 0
  failure |= TEST_PRINTING("%-03d", 123,  "123");
  failure |= TEST_PRINTING("%-03d", -123, "-123");
  failure |= TEST_PRINTING("%-04d", 123,  "0123");
  failure |= TEST_PRINTING("%-04d", -123, "-123");
  failure |= TEST_PRINTING("%-05d", 123,  "00123");
  failure |= TEST_PRINTING("%-05d", -123, "-0123");

  // Width with sign and left justification in both orders
  failure |= TEST_PRINTING("%+-3d", 123,  "+123");
  failure |= TEST_PRINTING("%+-3d", -123, "-123");
  failure |= TEST_PRINTING("%+-4d", 123,  "+123");
  failure |= TEST_PRINTING("%+-4d", -123, "-123");
  failure |= TEST_PRINTING("%+-5d", 123,  "+123 ");
  failure |= TEST_PRINTING("%+-5d", -123, "-123 ");
  failure |= TEST_PRINTING("%-+3d", 123,  "+123");
  failure |= TEST_PRINTING("%-+3d", -123, "-123");
  failure |= TEST_PRINTING("%-+4d", 123,  "+123");
  failure |= TEST_PRINTING("%-+4d", -123, "-123");
  failure |= TEST_PRINTING("%-+5d", 123,  "+123 ");
  failure |= TEST_PRINTING("%-+5d", -123, "-123 ");

  // Precison is a nop with integers
  failure |= TEST_PRINTING("%.d", 123, "123");
  failure |= TEST_PRINTING("%.4d", 123, "123");

  // Constant parameter width
  failure |= TEST_PRINTING_2_PARAMS("%*d", 3, 123,   "123");
  failure |= TEST_PRINTING_2_PARAMS("%*d", 3, -123,  "-123");
  failure |= TEST_PRINTING_2_PARAMS("%*d", 4, 123,   " 123");
  failure |= TEST_PRINTING_2_PARAMS("%+*d", 4, 123,  "+123");
  failure |= TEST_PRINTING_2_PARAMS("%+*d", 5, 123,  " +123");
  failure |= TEST_PRINTING_2_PARAMS("%-*d", 5, 123,  "123  ");
  failure |= TEST_PRINTING_2_PARAMS("%-*d", 5, -123, "-123 ");
  failure |= TEST_PRINTING_2_PARAMS("%-+*d", 5, 123, "+123 ");
  failure |= TEST_PRINTING_2_PARAMS("%-+*d", 5, -123,"-123 ");
  failure |= TEST_PRINTING_2_PARAMS("%+-*d", 5, -123,"-123 ");

  failure |= TEST_PRINTING_2_PARAMS("%0*d", 4, 123,  "0123");
  failure |= TEST_PRINTING_2_PARAMS("%0*d", 4, -123, "-123");
  failure |= TEST_PRINTING_2_PARAMS("%0*d", 5, -123, "-0123");
  failure |= TEST_PRINTING_2_PARAMS("%+0*d", 5, 123, "+0123");
  failure |= TEST_PRINTING_2_PARAMS("%0+*d", 5, -123,"-0123");

 
  // Constant parameter precision are nop's
  failure |= TEST_PRINTING_2_PARAMS("%.*d", 2, 123, "123");
  failure |= TEST_PRINTING_2_PARAMS("%.*d", 3, 123, "123");
  failure |= TEST_PRINTING_2_PARAMS("%.*d", 4, 123, "123");

  // Computed parameter width and precision
  ac_uint min_width = 6;
  ac_uint precision = 3;
  failure |= TEST_PRINTING_3_PARAMS("%*.*d", min_width, precision, 123,     "   123");
  failure |= TEST_PRINTING_3_PARAMS("%+*.*d", min_width, precision, 123,    "  +123");
  failure |= TEST_PRINTING_3_PARAMS("%+*.*d", min_width, precision, -123,   "  -123");
  failure |= TEST_PRINTING_3_PARAMS("%+-*.*d", min_width, precision, 123,   "+123  ");
  failure |= TEST_PRINTING_3_PARAMS("%+-*.*d", min_width, precision, -123,  "-123  ");

  failure |= TEST_PRINTING_3_PARAMS("%+0-*.*d", min_width, precision, 123,  "+00123");
  failure |= TEST_PRINTING_3_PARAMS("%0+-*.*d", min_width, precision, -123, "-00123");
  failure |= TEST_PRINTING_3_PARAMS("%+-0*.*d", min_width, precision, -123, "-00123");

  // Test a ac_printf_format_proc
  ac_u8 xx = 0xff;
  failure |= AC_TEST(ac_printf_register_format_proc(printf_ff_format_proc, xx) == 0);
  failure |= TEST_PRINTING_2_PARAMS("%\xff %s", 123ll, "me", "123 me");

  // Test a ac_printf_format_proc
  char* s64_str = "s64";
  failure |= AC_TEST(ac_printf_register_format_proc_str(printf_format_proc_s64, s64_str) == 0);
  failure |= TEST_PRINTING("%{abc} %s", "xx", "{abc} xx");
  failure |= TEST_PRINTING_2_PARAMS("%{s64} %s", (AcS64)123, "xx", "123 xx");
  failure |= TEST_PRINTING_2_PARAMS("%{s64 %s", (AcS64)123, "xx", "{s64 %s<<'}' is missing>>");
  failure |= TEST_PRINTING("%{s64456789012345678901234567890123} %s", "xx",
      "{s644567890123456789012345678901<<to long>>");

  // In printf statements constant negative numbers must be cast
  // so they work both 32 and 64 bit environments.
  failure |= TEST_PRINTING("%i", AC_SINT(-2147483647), "-2147483647");
  failure |= TEST_PRINTING("%i", (ac_sint)-2147483648, "-2147483648");
  failure |= TEST_PRINTING("%i", (ac_int)-1, "-1");

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

  // Test "l" and "ll" both are ac_u64/ll
  failure |= TEST_PRINTING("%lb", -1ll,
      "1111111111111111111111111111111111111111111111111111111111111111");
  failure |= TEST_PRINTING("%llb", 0ll, "0");
  failure |= TEST_PRINTING("%lb", 0x8765432187654321,
      "1000011101100101010000110010000110000111011001010100001100100001");
  failure |= TEST_PRINTING("%llb", (ac_u64)0x8765432187654321,
      "1000011101100101010000110010000110000111011001010100001100100001");
  failure |= TEST_PRINTING("%lb", (ac_u64)0xFFFFFFFFFFFFFFFF,
      "1111111111111111111111111111111111111111111111111111111111111111");

  failure |= TEST_PRINTING("%ld", -1ll, "-1");
  failure |= TEST_PRINTING("%lld", 0ll, "0");
  failure |= TEST_PRINTING("%ld",
      (ac_s64)0x7FFFFFFFFFFFFFFF, "9223372036854775807");
  failure |= TEST_PRINTING("%lld",
      0x7FFFFFFFFFFFFFFFll, "9223372036854775807");
  failure |= TEST_PRINTING("%ld",
      (ac_s64)0x8000000000000000, "-9223372036854775808");
  failure |= TEST_PRINTING("%ld",
      0x7FFFFFFFFFFFFFFF, "9223372036854775807");
  failure |= TEST_PRINTING("%ld",
      0x8000000000000000, "-9223372036854775808");

  failure |= TEST_PRINTING("%lu", -1ll, "18446744073709551615");
  failure |= TEST_PRINTING("%llu", 0ll, "0");
  failure |= TEST_PRINTING("%lu",
      (ac_u64)0x7FFFFFFFFFFFFFFF, "9223372036854775807");
  failure |= TEST_PRINTING("%lu",
      0x8000000000000000ll, "9223372036854775808");
  failure |= TEST_PRINTING("%llu",
      0xFFFFFFFFFFFFFFFF, "18446744073709551615");
  failure |= TEST_PRINTING("%lu",
      0x7FFFFFFFFFFFFFFF, "9223372036854775807");
  failure |= TEST_PRINTING("%llu",
      0x8000000000000000, "9223372036854775808");
  failure |= TEST_PRINTING("%lu",
      0xFFFFFFFFFFFFFFFF, "18446744073709551615");

  failure |= TEST_PRINTING("%lu", (ac_u64)-1, "18446744073709551615");
  failure |= TEST_PRINTING("%lu", -1ll, "18446744073709551615");
  failure |= TEST_PRINTING("%lx",
      (ac_u64)0x7FFFFFFFFFFFFFFF, "7fffffffffffffff");
  failure |= TEST_PRINTING("%llx",
      (ac_u64)0x8000000000000000, "8000000000000000");
  failure |= TEST_PRINTING("%lx",
      0xFFFFFFFFFFFFFFFFll, "ffffffffffffffff");
  failure |= TEST_PRINTING("%lx",
      0x7FFFFFFFFFFFFFFF, "7fffffffffffffff");
  failure |= TEST_PRINTING("%lx",
      0x8000000000000000, "8000000000000000");
  failure |= TEST_PRINTING("%lx",
      0xFFFFFFFFFFFFFFFF, "ffffffffffffffff");

  if (sizeof(ac_uint) == sizeof(ac_u64)) {
    // Test big positive and negative numbers on 64 bit systems
    // But casting isn't necessary, but works.
    failure |= TEST_PRINTING("%lb", 0x8765432187654321,
        "1000011101100101010000110010000110000111011001010100001100100001");
    failure |= TEST_PRINTING("%llb", 0xFFFFFFFFFFFFFFFF,
        "1111111111111111111111111111111111111111111111111111111111111111");

    failure |= TEST_PRINTING("%i",
        (ac_sint)0x7FFFFFFFFFFFFFFF, "9223372036854775807");
    failure |= TEST_PRINTING("%i",
        (ac_int)0x8000000000000000, "-9223372036854775808");
    failure |= TEST_PRINTING("%i",
        0x7FFFFFFFFFFFFFFF, "9223372036854775807");
    failure |= TEST_PRINTING("%i",
        0x8000000000000000, "-9223372036854775808");


    failure |= TEST_PRINTING("%v",
        (ac_uint)0x7FFFFFFFFFFFFFFF, "9223372036854775807");
    failure |= TEST_PRINTING("%v",
        AC_UINT(0x8000000000000000), "9223372036854775808");
    failure |= TEST_PRINTING("%v",
        (ac_uint)0xFFFFFFFFFFFFFFFF, "18446744073709551615");
    failure |= TEST_PRINTING("%v",
        0x7FFFFFFFFFFFFFFF, "9223372036854775807");
    failure |= TEST_PRINTING("%v",
        0x8000000000000000, "9223372036854775808");
    failure |= TEST_PRINTING("%v",
        0xFFFFFFFFFFFFFFFF, "18446744073709551615");

    failure |= TEST_PRINTING("%v", (ac_uint)-1, "18446744073709551615");
    failure |= TEST_PRINTING("%v", AC_UINT(-1), "18446744073709551615");

    failure |= TEST_PRINTING("%y",
        (ac_uint)0x7FFFFFFFFFFFFFFF, "7fffffffffffffff");
    failure |= TEST_PRINTING("%y",
        AC_UINT(0x8000000000000000), "8000000000000000");
    failure |= TEST_PRINTING("%y",
        (ac_uint)0xFFFFFFFFFFFFFFFF, "ffffffffffffffff");
    failure |= TEST_PRINTING("%y",
        0x7FFFFFFFFFFFFFFF, "7fffffffffffffff");
    failure |= TEST_PRINTING("%y",
        0x8000000000000000, "8000000000000000");
    failure |= TEST_PRINTING("%y",
        0xFFFFFFFFFFFFFFFF, "ffffffffffffffff");
  }


  failure |= TEST_PRINTING_NO_PARAM("%l", "%l");
  failure |= TEST_PRINTING_NO_PARAM("%la", "%la");
  failure |= TEST_PRINTING_NO_PARAM("%ll", "%ll");
  failure |= TEST_PRINTING_NO_PARAM("%llz1", "%llz1");

  failure |= TEST_PRINTING("%llx", 0ll, "0");
  failure |= TEST_PRINTING("%llx", 9ll, "9");
  failure |= TEST_PRINTING("%y", 10ll, "a");
  failure |= TEST_PRINTING("%y", 15ll, "f");
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
      failure |= TEST_PRINTING("%y", i, "f7654321f7654321");
  } else {
      failure |= FAIL(ac_formatter(
            &writer, "Unexptected sizeof(ac_uint)=%d", sizeof(ac_uint)));
  }

  /** Test ac_snprintf **/
  ac_u8 buff[10];

  count = ac_snprintf(AC_NULL, 10, "%d", 0);
  failure |= AC_TEST(count == 0);

  count = ac_snprintf(buff, 0, "%d", 0);
  failure |= AC_TEST(count == 0);

  count = ac_snprintf(buff, 1, "%d", 0);
  failure |= AC_TEST(count == 0);

  count = ac_snprintf(buff, 2, "%s", "12");
  ac_debug_printf("ac_snprintf(buff, 2, \"%%s\", \"12\")=\"%s\"\n", buff);
  failure |= AC_TEST(count == 1);
  failure |= AC_TEST(ac_strncmp("1", (char*)buff, 2) == 0);

  count = ac_snprintf(buff, AC_ARRAY_COUNT(buff), "%d", 0);
  ac_debug_printf("ac_snprintf(buff, AC_ARRAY_COUNT(buff), \"%%d\", 0)=\"%s\"\n", buff);
  failure |= AC_TEST(count == 1);
  failure |= AC_TEST(ac_strncmp("0", (char*)buff, 2) == 0);

  count = ac_snprintf(buff, AC_ARRAY_COUNT(buff), "%d", 1);
  ac_debug_printf("ac_snprintf(buff, AC_ARRAY_COUNT(buff), \"%%d\", 1)=\"%s\"\n", buff);
  failure |= AC_TEST(count == 1);
  failure |= AC_TEST(ac_strncmp("1", (char*)buff, 2) == 0);

  count = ac_snprintf(buff, AC_ARRAY_COUNT(buff), "%d", -1l);
  ac_debug_printf("ac_snprintf(buff, AC_ARRAY_COUNT(buff), \"%%d\", -1)=\"%s\"\n", buff);
  failure |= AC_TEST(count == 2);
  failure |= AC_TEST(ac_strncmp("-1", (char*)buff, 3) == 0);

  if (!failure) {
    ac_printf("OK\n");
  }
  return failure;
}
