/*
 * copyright 2016 wink saville
 *
 * licensed under the apache license, version 2.0 (the "license");
 * you may not use this file except in compliance with the license.
 * you may obtain a copy of the license at
 *
 *     http://www.apache.org/licenses/license-2.0
 *
 * unless required by applicable law or agreed to in writing, software
 * distributed under the license is distributed on an "as is" basis,
 * without warranties or conditions of any kind, either express or implied.
 * see the license for the specific language governing permissions and
 * limitations under the license.
 */

#ifndef SADIE_LIBS_AC_SWAP_BYTES_H
#define SADIE_LIBS_AC_SWAP_BYTES_H

#include <ac_inttypes.h>

#define AC_SWAP_BYTES_U16(val) ({ \
  ac_u16 result = (((ac_u16)val & (ac_u16)0x00FF) << 8)  \
                | (((ac_u16)val & (ac_u16)0xFF00) >> 8); \
  result; \
})

#define AC_SWAP_BYTES_U32(val) ({ \
  ac_u32 result = (((ac_u32)val & (ac_u32)0x000000FF) << 24)  \
                | (((ac_u32)val & (ac_u32)0x0000FF00) <<  8)  \
                | (((ac_u32)val & (ac_u32)0x00FF0000) >>  8)  \
                | (((ac_u32)val & (ac_u32)0xFF000000) >> 24); \
  result; \
})

#define AC_SWAP_BYTES_U64(val) ({ \
  ac_u64 result = (((ac_u64)val & (ac_u64)0x00000000000000FF) << 56)  \
                | (((ac_u64)val & (ac_u64)0x000000000000FF00) << 40)  \
                | (((ac_u64)val & (ac_u64)0x0000000000FF0000) << 24)  \
                | (((ac_u64)val & (ac_u64)0x00000000FF000000) <<  8)  \
                | (((ac_u64)val & (ac_u64)0x000000FF00000000) >>  8)  \
                | (((ac_u64)val & (ac_u64)0x0000FF0000000000) >> 24)  \
                | (((ac_u64)val & (ac_u64)0x00FF000000000000) >> 40)  \
                | (((ac_u64)val & (ac_u64)0xFF00000000000000) >> 56); \
  result; \
})

#endif
