/*
 * copyright 2015 wink saville
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

#include <thread_x86_debug_ctx_switch.h>

        // Have the current yield to the next ready thread.
        //
        // @param: remove_pready passed to thread_scheduler
        .GLOBAL thread_yield
thread_yield:
        // Setup interrupt stack frame
        pop     %rcx            # Get return address
        mov     %rsp, %rdx      # Save rsp to rdx
        mov     %ss, %ax        # Get ss to ax
        push    %rax            # Push ss from rax
        push    %rdx            # Push rsp from rdx
        pushfq                  # Push flags
        mov     %cs, %ax
        push    %rax            # Push cs
        push    %rcx            # Push rip aka return address

        push    %rbp
        mov     %rsp, %rbp

        // Save regsiters
        push    %r15
        push    %r14
        push    %r13
        push    %r12
        push    %r11
        push    %r10
        push    %r9
        push    %r8
        push    %rdi
        push    %rsi
        push    %rbx
        push    %rcx
        push    %rdx
        push    %rax

        // Be sure stack is aligned on 16 byte boundary
        and     $0xfffffffffffffff0, %rsp

        cld    

        // Disable interrupts
        cli

#ifdef THREAD_X86_DEBUG_CTX_SWITCH
        push    %rdi
        mov     $thread_yield_e_str, %rdi
        lea     -0x70(%rbp), %rsi
        callq   print_full_stack_frame
        pop     %rdi
#endif

        // Call thread scheduler, returns in rax the
        // tcb_x86 pointer of the thread to run
        // mov  %rdi, %rdi;        # rdi == remove_ready,
        //                         # passed in rdi to thread_yield
        lea     -0x70(%rbp), %rsi; # rsi == sp
        mov     %ss,%dx;
        movzwl  %dx,%edx;          # edx == ss
        call    thread_scheduler

        // Switch stacks
        mov    0x30(%rax), %rsp
        mov    0x38(%rax), %ss

#ifdef THREAD_X86_DEBUG_CTX_SWITCH
        mov     $thread_yield_x_str, %rdi
        mov     %rsp, %rsi
        callq   print_full_stack_frame
#endif

        // Restore registers and return
        pop     %rax
        pop     %rdx
        pop     %rcx
        pop     %rbx
        pop     %rsi
        pop     %rdi
        pop     %r8
        pop     %r9
        pop     %r10
        pop     %r11
        pop     %r12
        pop     %r13
        pop     %r14
        pop     %r15
        pop     %rbp

        iretq

thread_yield_e_str:
        .string "thread_yield+"

thread_yield_x_str:
        .string "thread_yield-"

/**
 * reschedule_isr
 */
        .GLOBAL reschedule_isr
reschedule_isr:
        // Save regsiters
        push    %rbp
        mov     %rsp, %rbp
        push    %r15
        push    %r14
        push    %r13
        push    %r12
        push    %r11
        push    %r10
        push    %r9
        push    %r8
        push    %rdi
        push    %rsi
        push    %rbx
        push    %rcx
        push    %rdx
        push    %rax

        // Be sure stack is aligned on 16 byte boundary
        and     $0xfffffffffffffff0, %rsp

        cld    

#ifdef THREAD_X86_DEBUG_CTX_SWITCH
        mov     $reschedule_isr_e_str, %rdi
        lea     -0x70(%rbp), %rsi
        callq   print_full_stack_frame
#endif

        // Call thread scheduler, returns in rax the
        // tcb_x86 pointer of the thread to run
        mov     %ss,%si
        movzwl  %si,%esi
        lea     -0x70(%rbp), %rdi
        call    thread_scheduler

        // Switch stacks
        mov    0x30(%rax), %rsp
        mov    0x38(%rax), %ss

#ifdef THREAD_X86_DEBUG_CTX_SWITCH
        mov     $reschedule_isr_ptcb_str, %rdi
        mov     %rax, %rsi
        mov     %rsp, %rdx
        callq   ac_printf
        mov     $reschedule_isr_x_str, %rdi
        mov     %rsp, %rsi
        callq   print_full_stack_frame
#endif

        // Restore registers and return
        pop     %rax
        pop     %rdx
        pop     %rcx
        pop     %rbx
        pop     %rsi
        pop     %rdi
        pop     %r8
        pop     %r9
        pop     %r10
        pop     %r11
        pop     %r12
        pop     %r13
        pop     %r14
        pop     %r15
        pop     %rbp

        iretq

reschedule_isr_ptcb_str:
        .string "reschedule_isr: ptcb=%x rsp=%x\n"

reschedule_isr_e_str:
        .string "reschedule_isr+"

reschedule_isr_x_str:
        .string "reschedule_isr-"

/**
 * timer_reschedule_isr
 */
        .GLOBAL timer_reschedule_isr
timer_reschedule_isr:
        // Save regsiters
        push    %rbp
        mov     %rsp, %rbp
        push    %r15
        push    %r14
        push    %r13
        push    %r12
        push    %r11
        push    %r10
        push    %r9
        push    %r8
        push    %rdi
        push    %rsi
        push    %rbx
        push    %rcx
        push    %rdx
        push    %rax

        // Be sure stack is aligned on 16 byte boundary
        and     $0xfffffffffffffff0, %rsp

        cld    

#ifdef THREAD_X86_DEBUG_CTX_SWITCH
        mov     $timer_reschedule_isr_e_str, %rdi
        lea     -0x70(%rbp), %rsi
        callq   print_full_stack_frame
#endif

        // Call thread scheduler for timer , returns in rax the
        // tcb_x86 pointer of the thread to run
        mov     %ss,%si
        movzwl  %si,%esi
        lea     -0x70(%rbp), %rdi
        call   timer_thread_scheduler

        // Switch stacks
        mov    0x30(%rax), %rsp
        mov    0x38(%rax), %ss

#ifdef THREAD_X86_DEBUG_CTX_SWITCH
        mov     $timer_reschedule_isr_x_str, %rdi
        mov     %rsp, %rsi
        callq   print_full_stack_frame
#endif

        // Restore registers and return
        pop     %rax
        pop     %rdx
        pop     %rcx
        pop     %rbx
        pop     %rsi
        pop     %rdi
        pop     %r8
        pop     %r9
        pop     %r10
        pop     %r11
        pop     %r12
        pop     %r13
        pop     %r14
        pop     %r15
        pop     %rbp

        iretq

timer_reschedule_isr_e_str:
        .string "timer_reschedule_isr+"

timer_reschedule_isr_x_str:
        .string "timer_reschedule_isr-"

