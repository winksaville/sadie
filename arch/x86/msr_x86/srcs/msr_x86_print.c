#include <msr_x86_print.h>

#include <msr_x86.h>
#include <cpuid_x86.h>

#include <ac_bits.h>
#include <ac_inttypes.h>
#include <ac_printf.h>

void print_msr(ac_u32 msr, ac_u64 value) {
  union msr_apic_base_u reg = { .raw = value };

  switch (msr) {
    case MSR_APIC_BASE: {
      ac_printf("msr_apic_base 0x%x: 0x%llx\n", MSR_APIC_BASE, value);
      ac_printf(" bsp=%b\n", reg.fields.bsp);
      ac_printf(" extd=%b\n", reg.fields.extd);
      ac_printf(" e=%b\n", reg.fields.e);
      ac_printf(" base=0x%llx\n", msr_apic_base_physical_addr(reg.raw));
      break;
    }

    default: {
      ac_printf("unknown msr 0x%x: 0x%llx\n", reg, value);
    }
  }
}
