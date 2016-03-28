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

#include <ac_receptor.h>

#include <thread_x86.h>

#include <ac_assert.h>
#include <ac_inttypes.h>
#include <ac_printf.h>
#include <ac_test.h>
#include <ac_thread.h>
#include <ac_tsc.h>

#define round_up(x, y) ({               \
    ac_u64 rem = (x) % (y);             \
    ac_u64 result = ((x) + rem) / (y);  \
    result;                             \
})

typedef struct {
  ac_u64 scaler;
  char* units;
} tres_t;

tres_t tres[] = {
  { 1000000000ll, "ns" },
  { 1000000ll, "us" },
  { 1000ll, "ms" },
  { 1ll, "s" },
};

#define AC_U64_MAX 0xFFFFFFFFFFFFFFFFll

tres_t* calculate_tres(ac_u64 ticks) {
  //ac_printf("calculate_tres:+ticks=%ld\n", ticks);
  ac_uint i;
  for (i = 0; i < AC_ARRAY_COUNT(tres); i++) {
    //ac_printf("calculate_tres: i=%d\n", i);
    if (ticks < AC_U64_MAX / tres[i].scaler) {
      //ac_printf("calculate_tres:-ticks=%ld i=%d scaler=%ld units=%s\n",
      //  ticks, i, tres[i].scaler, tres[i].units);
      return &tres[i];
    }
  }
  //ac_printf("calculate_tres:-BAD AC_NULL\n");
  ac_assert(i < AC_ARRAY_COUNT(tres));
  return AC_NULL;
}

ac_u32 t1_count;

struct test_params {
  ac_receptor_t receptor;
  ac_uint loops;
  ac_uint counter;
  ac_receptor_t done_receptor;
};

void* t1(void *param) {
  struct test_params* params = (struct test_params*)param;
  //ac_printf("t1:+params->loops=%d\n", params->loops);
  for(ac_uint i = 0; i < params->loops; i++) {
    __atomic_add_fetch(&params->counter, 1, __ATOMIC_RELEASE);

    //ac_thread_yield();

    if ((i % 1) == 0) {
      ac_printf("t1: i=%d flags=%x isr_counter=%d receptor.thdl=%x",
          i, get_flags(), get_timer_reschedule_isr_counter(), params->receptor->thdl);
      print_ready_list(" - ");
    }

    ac_printf("t1: waiting    thdl=%x flags=%x\n", ac_thread_get_cur_hdl(), get_flags());
    tres_t* ptres;
    ac_u64 time;
    ac_u64 wait_start = ac_tscrd();
    ac_receptor_wait(params->receptor);
    ac_u64 ticks = ac_tscrd() - wait_start;
    ptres = calculate_tres(ticks);
    time = round_up(ticks * ptres->scaler, ac_tsc_freq());
    ac_printf("t1: continuing wait time = %ld%s\n", time, ptres->units);
  }

  ac_printf("t1: signal done_receptor\n");
  ac_receptor_signal(params->done_receptor, AC_FALSE);
  return AC_NULL;
}

ac_uint test_receptor(void) {
  ac_printf("test_receptor:+\n");

  ac_uint error = AC_FALSE;
#if defined(pc_x86_64)
  tres_t* ptres;
  ac_u64 time;
  ac_u64 time_per_op;
  struct test_params params;

  ac_printf("test_receptor: call ac_receptor_create\n");
  params.receptor = ac_receptor_create(AC_FALSE);
  params.done_receptor = ac_receptor_create(AC_FALSE);
  params.counter = 0;
  params.loops = 10;

  ac_thread_rslt_t rslt = ac_thread_create(0, t1, (void*)&params);
  error |= AC_TEST(rslt.status == 0);

  ac_u64 start = ac_tscrd();

  ac_printf("test_receptor: loops=%ld\n", params.loops);
  ac_uint x = 0;
  for (ac_uint i = 0; i < params.loops; i++) {
    while (__atomic_load_n(&params.counter, __ATOMIC_ACQUIRE) <= i) {
      if ((x++ % 20000000) == 0) {
        ac_printf("test_receptor: i=%d x=%d flags=%x isr_counter=%d receptor.thdl=%x",
            i, x, get_flags(), get_timer_reschedule_isr_counter(), params.receptor->thdl);
        print_ready_list(" - ");
      }

      //ac_printf("test_receptor: call ac_thread_yield\n");
      //ac_thread_yield();
    }
    ac_u64 wait_until = ac_tscrd() + (ac_tsc_freq() / 1);
    ac_printf("test_receptor: wait_until=%ld\n", wait_until);
    while (ac_tscrd() < wait_until) {
      if ((x++ % 20000000) == 0) {
        ac_printf("test_receptor: tsc=%ld i=%d x=%d flags=%x isr_counter=%d receptor.thdl=%x",
            ac_tscrd(), i, x, get_flags(), get_timer_reschedule_isr_counter(), params.receptor->thdl);
        print_ready_list(" - ");
      }
      //ac_thread_yield();
    }

    //ac_printf("test_receptor: signal\n");
    ac_u64 signal_start = ac_tscrd();
    ac_receptor_signal(params.receptor, AC_TRUE);
    ac_u64 ticks = ac_tscrd() - signal_start;
    ptres = calculate_tres(ticks);
    time = round_up(ticks * ptres->scaler, ac_tsc_freq());
    ac_printf("test_receptor: signal time = %ld%s\n", time, ptres->units);
  }
  ac_printf("test_receptor: wait done_receptor x=%d\n", x);
  ac_receptor_wait(params.done_receptor);
  ac_printf("test_receptor: continuing done_receptor\n");

  ac_u64 stop = ac_tscrd();
  ac_u64 ticks = stop - start;
  ac_printf("test_receptor: ticks=%ld %ld - %ld\n", ticks, stop, start);

  ac_u64 ticks_per_op = round_up(ticks, params.loops);
  ac_printf("test_receptor: ticks_per_op=%ld\n", ticks_per_op);

  ptres = calculate_tres(ticks);
  time = round_up(ticks * ptres->scaler, ac_tsc_freq());
  ac_printf("test_receptor: time = %ld%s\n", time, ptres->units);

  ptres = calculate_tres(ticks_per_op);
  time_per_op = round_up(ticks_per_op * ptres->scaler, ac_tsc_freq());
  ac_printf("test_receptor: time_per_op = %ld%s\n", time_per_op, ptres->units);

  ac_receptor_destroy(params.receptor);
#endif

  ac_printf("test_receptor:-error=%d\n", error);
  return error;
}

int main(void) {
  ac_uint error = AC_FALSE;

  ac_thread_init(8);
  ac_receptor_init(256);

  if (!error) {
    error |= test_receptor();
  }

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
