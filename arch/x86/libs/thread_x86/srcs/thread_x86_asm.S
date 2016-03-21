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

        .EXTERN thread_scheduler
        .GLOBAL thread_x86_yield

thread_x86_yield:
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

        // Save regsiters
        push    %r15
        push    %r14
        push    %r13
        push    %r12
        push    %r11
        push    %r10
        push    %r9
        push    %r8
        push    %rbp
        push    %rdi
        push    %rsi
        push    %rbx
        push    %rcx
        push    %rdx
        push    %rax
        cld    

        // Disable interrupts
        cli

        // Call thread scheduler, returns in rax the
        // tcb_x86 pointer of the thread to run
        mov    %ss,%si
        mov    %rsp,%rdi
        movzwl %si,%esi
        call   thread_scheduler

        // Switch stacks
        mov    0x30(%rax), %rsp
        mov    0x38(%rax), %ss

        // Restore registers and return
        pop     %rax
        pop     %rdx
        pop     %rcx
        pop     %rbx
        pop     %rsi
        pop     %rdi
        pop     %rbp
        pop     %r8
        pop     %r9
        pop     %r10
        pop     %r11
        pop     %r12
        pop     %r13
        pop     %r14
        pop     %r15

        iretq
