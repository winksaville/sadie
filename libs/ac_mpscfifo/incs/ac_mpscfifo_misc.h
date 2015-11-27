/**
 * Miscelaneous header file
 */

#ifndef SADIE_LIBS_AC_MPSCFIFO_AC_MPSCFIFO_MISC_H
#define SADIE_LIBS_AC_MPSCFIFO_AC_MPSCFIFO_MISC_H

#include <ac_inttypes.h>
#include <ac_printf.h>

/**
 * Set "bool error" to "true" if cond is "false"
 */
#define TEST(cond, text)                                                       \
  do {                                                                         \
    ac_bool result = !(cond);                                                     \
    error |= result;                                                           \
    if (result) {                                                              \
      ac_printf("Error %s:%d FN %s: %s. Condition '" #cond "' failed.\n",         \
             __FILE__, __LINE__, __func__, text);                              \
    }                                                                          \
  } while (AC_FALSE);

#define UNUSED(x) (void)(x)

#endif
