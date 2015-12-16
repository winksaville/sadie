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

#include <ac_thread.h>

#include <ac_debug_printf.h>
#include <ac_test.h>

ac_u32 t1_count;

void* t1(void *param) {
  ac_u32 value = (ac_u32)(ac_uptr)param;
  ac_debug_printf("t1: param=%d\n", value);
  __atomic_add_fetch(&t1_count, value, __ATOMIC_RELEASE);
  return AC_NULL;
}

#if defined(VersatilePB)

#include <ac_timer.h>
#include <ac_exceptions.h>

typedef struct {
  ac_u32  timer;
  ac_bool  source;
} irq_param;

ac_u32 periodic_counter;

/**
 * Indentify and clear source of interrupt.
 * After returning interrupts will be enabled
 * so we use __atomic operations on source.
 */
void periodic_iacs(ac_uptr param) {
  irq_param* pirq_param = (irq_param*)param;
  ac_u32 timer_ris = ac_timer_rd_ris(pirq_param->timer);
  if ((timer_ris & 0x1) != 0) {
    ac_bool* psource = &pirq_param->source;
    __atomic_store_n(psource, AC_TRUE, __ATOMIC_RELEASE);
    ac_timer_wr_int_clr(pirq_param->timer);
    ac_debug_printf("\n\nperiodic: %d cleared\n", pirq_param->timer);
  }
}

/**
 * Handle the one_shot interrupt.
 *
 * NOTE: Interrupts are enabled so __atomic operations are used.
 */
void periodic_handler(ac_uptr param) {
  irq_param* pirq_param = (irq_param*)param;
  ac_bool ac_true = AC_TRUE;
  ac_bool* psource = &pirq_param->source;

  ac_bool ok = __atomic_compare_exchange_n(psource, &ac_true, AC_FALSE,
      AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
  if (ok) {
    __atomic_add_fetch(&periodic_counter, 1, __ATOMIC_RELEASE);
    ac_debug_printf("periodic: %d inc counter\n\n", pirq_param->timer);
  }
}

#endif

int main(void) {
  ac_bool error = AC_FALSE;
  ac_u32 t1_count_initial = -1;
  ac_u32 t1_count_increment = 2;
  ac_u32 result = t1_count_initial;

#if defined(VersatilePB)

  ac_debug_printf("test-ac_thread: VersatilePB\n");

  irq_param periodic_param = {
    .timer = 1,
    .source = AC_FALSE,
  };

  ac_exception_irq_register(&periodic_handler, &periodic_iacs,
      (ac_uptr)&periodic_param);
  ac_timer_periodic(periodic_param.timer, 1000);

#elif defined(Posix)

  ac_debug_printf("test-ac_thread: Posix\n");

  ac_thread_init(1);

#else

  ac_printf("Uknown platform\n");
  goto done;

#endif

  t1_count = t1_count_initial;
  ac_u32 created = ac_thread_create(0, t1, (void*)(ac_uptr)t1_count_increment);
  error |= AC_TEST(created == 0);

  if (created == 0) {
    const ac_u32 max_loops = 1000000000;
    ac_u32 loops;
    for (loops = 0; loops < max_loops; loops++) {
      result = __atomic_load_n(&t1_count, __ATOMIC_ACQUIRE);
      if (result != t1_count_initial) {
        break;
      }
    }
    ac_debug_printf("test-ac_thread: loops=%d result=%d\n", loops, result);
    error |= AC_TEST(loops < max_loops);
    error |= AC_TEST(result == t1_count_initial + t1_count_increment);

    // Since we don't have a 'ac_thread_join' we're racing, so
    // this delay give greater assurance the tests works.
    // TODO: Add ac_thread_join although that means blocking
    // which I don't like so we'll see.
    for (int i = 0; i < 1000000; i++) {
      __atomic_load_n(&t1_count, __ATOMIC_ACQUIRE);
    }

    created = ac_thread_create(0, t1, (void*)(ac_uptr)t1_count_increment);
    error |= AC_TEST(created == 0);

    if (created == 0) {
      const ac_u32 max_loops = 1000000000;
      ac_u32 loops;
      for (loops = 0; loops < max_loops; loops++) {
        result = __atomic_load_n(&t1_count, __ATOMIC_ACQUIRE);
        if (result != t1_count_initial + t1_count_increment) {
          break;
        }
      }
      ac_debug_printf("test-ac_thread: loops=%d result=%d\n", loops, result);
      error |= AC_TEST(loops < max_loops);
      error |= AC_TEST(result == t1_count_initial + (2 * t1_count_increment));
    }
  }

done:
  if (!error) {
    // Succeeded
    ac_printf("OK\n");
  }

  return error;
}

