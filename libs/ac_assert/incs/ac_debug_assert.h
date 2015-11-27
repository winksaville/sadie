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
#ifndef SADIE_LIBS_INCS_AC_DEBUG_ASSERT_H
#define SADIE_LIBS_INCS_AC_DEBUG_ASSERT_H

#ifdef NDEBUG

/** NDEBUG is defined do nothing */
#define ac_debug_assert(expr) ((void)(0))

#define ac_debug_static_assert(expr) ((void)(0))

#else // NDEBUG is not defined

#include <ac_assert.h>

/**
 * NDEBUG is not defined invoke ac_assert(expr).
 */
#define ac_debug_assert(expr) ac_assert(expr)

#define ac_debug_static assert(expr) ac_static_assert(expr)

#endif

#endif
