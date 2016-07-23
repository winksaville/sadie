/*
 * Copyright 2016 Wink Saville
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

#include <ac_inet_link.h>

#include <ac_memset.h>
#include <ac_printf.h>
#include <ac_test.h>

#pragma push_macro("AC_ARCH_BYTE_ORDER")
#undef AC_ARCH_BYTE_ORDER
#define AC_ARCH_BYTE_ORDER AC_ARCH_BYTE_ORDER_BIG_ENDIAN

#undef SADIE_LIBS_AC_INET_LINK_INCS_AC_INET_HTON_H
#undef AC_HTON_U16
#undef AC_HTON_U32
#undef AC_HTON_U64
#include <ac_inet_hton.h>

#undef SADIE_LIBS_AC_INET_LINK_INCS_AC_INET_NTOH_H
#undef AC_NTOH_U16
#undef AC_NTOH_U32
#undef AC_NTOH_U64
#include <ac_inet_ntoh.h>

ac_bool test_hton_ntoh_be(void) {
  ac_bool error = AC_FALSE;

  ac_u16 a_u16 = 0x0123;
  ac_u32 a_u32 = 0x01234567;
  ac_u64 a_u64 = 0x0123456789ABCDEF;

  // Big endian should be unmodified
  error |= AC_TEST(AC_HTON_U16(a_u16) == a_u16);
  error |= AC_TEST(AC_HTON_U32(a_u32) == a_u32);
  error |= AC_TEST(AC_HTON_U64(a_u64) == a_u64);
  error |= AC_TEST(AC_NTOH_U16(a_u16) == a_u16);
  error |= AC_TEST(AC_NTOH_U32(a_u32) == a_u32);
  error |= AC_TEST(AC_NTOH_U64(a_u64) == a_u64);

  return error;
}
#pragma pop_macro("AC_ARCH_BYTE_ORDER")

#pragma push_macro("AC_ARCH_BYTE_ORDER")
#undef AC_ARCH_BYTE_ORDER
#define AC_ARCH_BYTE_ORDER AC_ARCH_BYTE_ORDER_LITTLE_ENDIAN

#undef SADIE_LIBS_AC_INET_LINK_INCS_AC_INET_HTON_H
#undef AC_HTON_U16
#undef AC_HTON_U32
#undef AC_HTON_U64
#include <ac_inet_hton.h>

#undef SADIE_LIBS_AC_INET_LINK_INCS_AC_INET_NTOH_H
#undef AC_NTOH_U16
#undef AC_NTOH_U32
#undef AC_NTOH_U64
#include <ac_inet_ntoh.h>

ac_bool test_hton_ntoh_le(void) {
  ac_bool error = AC_FALSE;

  ac_u16 a_u16 = 0x0123;
  ac_u32 a_u32 = 0x01234567;
  ac_u64 a_u64 = 0x0123456789ABCDEF;

  // Little endian should be swapped
  error |= AC_TEST(AC_HTON_U16(a_u16) == AC_SWAP_BYTES_U16(a_u16));
  error |= AC_TEST(AC_HTON_U32(a_u32) == AC_SWAP_BYTES_U32(a_u32));
  error |= AC_TEST(AC_HTON_U64(a_u64) == AC_SWAP_BYTES_U64(a_u64));

  error |= AC_TEST(AC_NTOH_U16(a_u16) == AC_SWAP_BYTES_U16(a_u16));
  error |= AC_TEST(AC_NTOH_U32(a_u32) == AC_SWAP_BYTES_U32(a_u32));
  error |= AC_TEST(AC_NTOH_U64(a_u64) == AC_SWAP_BYTES_U64(a_u64));

  return error;
}
#pragma pop_macro("AC_ARCH_BYTE_ORDER")

struct test_case_AcInetIpv4FragmentOffset {
  AcInetIpv4FragmentOffset val;
  ac_u16 offset_in_u64s;
  ac_bool more_fragments;
  ac_bool dont_fragment;
  ac_bool zero;
};

static struct test_case_AcInetIpv4FragmentOffset test_case_ipv4_frag[] = {
  { .val.raw_u16=0x0001, .offset_in_u64s=0x0001, },
  { .val.raw_u16=0x1000, .offset_in_u64s=0x1000, },
  { .val.raw_u16=0x2000, .more_fragments=1, },
  { .val.raw_u16=0x4000, .dont_fragment=1, },
  { .val.raw_u16=0x8000, .zero=1, },
};

static ac_bool test_ipv4_frag(struct test_case_AcInetIpv4FragmentOffset* test) {
  ac_bool error = AC_FALSE;

  error |= AC_TEST(test->val.offset_in_u64s == test->offset_in_u64s);
  error |= AC_TEST(test->val.more_fragments == test->more_fragments);
  error |= AC_TEST(test->val.dont_fragment == test->dont_fragment);
  error |= AC_TEST(test->val.zero == test->zero);

  if (error) {
    ac_printf("AcInetIpv4FragmentOffset.raw_u16=0x%x host order\n", test->val.raw_u16);
  }

  // Convert from to network order and back to host and all should still succeed
  test->val.raw_u16 = AC_HTON_U16(test->val.raw_u16);
  test->val.raw_u16 = AC_NTOH_U16(test->val.raw_u16);

  error |= AC_TEST(test->val.offset_in_u64s == test->offset_in_u64s);
  error |= AC_TEST(test->val.more_fragments == test->more_fragments);
  error |= AC_TEST(test->val.dont_fragment == test->dont_fragment);
  error |= AC_TEST(test->val.zero == test->zero);

  if (error) {
    ac_printf("AcInetIpv4FragmentOffset.raw_u16=0x%x after HTON NTOH\n",
        test->val.raw_u16);
  }

  return error;
}

ac_bool test_AcInetIpv4FragmentOffset(void) {
  ac_bool error = AC_FALSE;

  for (ac_u32 i = 0; i < AC_ARRAY_COUNT(test_case_ipv4_frag); i++) {
    error |= test_ipv4_frag(&test_case_ipv4_frag[i]);
  }

  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  error |= test_hton_ntoh_be();
  error |= test_hton_ntoh_le();
  error |= test_AcInetIpv4FragmentOffset();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
