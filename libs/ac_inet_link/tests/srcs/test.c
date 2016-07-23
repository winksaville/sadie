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

ac_bool test_AcInetIpv4FragmentOffset(void) {
  ac_bool error = AC_FALSE;

  AcInetIpv4FragmentOffset frag;

  ac_memset(&frag, 0, sizeof(frag));
  error |= AC_TEST(frag.raw_u8[0] == 0);
  error |= AC_TEST(frag.raw_u8[1] == 0);
  error |= AC_TEST(frag.raw_u16 == 0);
  error |= AC_TEST(frag.offset_in_u64s == 0);
  error |= AC_TEST(frag.more_fragments == 0);
  error |= AC_TEST(frag.dont_fragment == 0);
  error |= AC_TEST(frag.zero == 0);

  return error;
}

ac_bool test_inet_link(void) {
  ac_bool error = AC_FALSE;

  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  error |= test_hton_ntoh_be();
  error |= test_hton_ntoh_le();
  error |= test_AcInetIpv4FragmentOffset();
  error |= test_inet_link();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
