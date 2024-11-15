#include <microkit.h>
#include <sel4/sel4.h>
#include <stdint.h>
#define PMU_CONTROL_CAP 8

#define MSR(reg, v)                                \
    do {                                           \
        uint64_t _v = v;                             \
        asm volatile("msr " reg ",%x0" ::  "r" (_v));\
    }while(0)

static char
hexchar(unsigned int v)
{
    return v < 10 ? '0' + v : ('a' - 10) + v;
}

static void
puthex64(uint64_t val)
{
    char buffer[16 + 3];
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[16 + 3 - 1] = 0;
    for (unsigned i = 16 + 1; i > 1; i--) {
        buffer[i] = hexchar(val & 0xf);
        val >>= 4;
    }
    microkit_dbg_puts(buffer);
}

void init() {
    microkit_dbg_puts("Attempting to use pmu invocations.\n");
    seL4_ARM_PMUControl_WriteEventCounter(PMU_CONTROL_CAP, 2, 5600, 42);

    seL4_ARM_PMUControl_ReadEventCounter_t ret = seL4_ARM_PMUControl_ReadEventCounter(PMU_CONTROL_CAP, 6);
    microkit_dbg_puts("This is the value of the cycle counter we read at the start: ");
    puthex64(ret.counter_value);
    microkit_dbg_puts("\n");
    seL4_ARM_PMUControl_CounterControl(PMU_CONTROL_CAP, 1);
    for (int i = 0; i < 77777; i++) {
        asm("nop");
    }
    ret = seL4_ARM_PMUControl_ReadEventCounter(PMU_CONTROL_CAP, 6);
    microkit_dbg_puts("This is the value of the cycle counter we read in the middle: ");
    puthex64(ret.counter_value);
    microkit_dbg_puts("\nStopping the PMU\n");
    seL4_ARM_PMUControl_CounterControl(PMU_CONTROL_CAP, 0);
    for (int i = 0; i < 9999999; i++) {
        asm("nop");
    }

    ret = seL4_ARM_PMUControl_ReadEventCounter(PMU_CONTROL_CAP, 6);
    microkit_dbg_puts("This is the value of the cycle counter we read at the end: ");
    puthex64(ret.counter_value);
    microkit_dbg_puts("\n");

    microkit_dbg_puts("Finished invocations.\n");

    microkit_dbg_puts("Attempting to directly access PMU register (this should fault)!\n");
    MSR("PMCNTENSET_EL0", 1 << 31);
}

void notified(microkit_channel ch) {
    return;
}