/**
 * Debug code for mpscfifo
 */

#ifndef SADIE_LIBS_AC_MPSCFIFO_AC_MPSCFIFO_DBG_H
#define SADIE_LIBS_AC_MPSCFIFO_AC_MPSCFIFO_DBG_H

#include <ac_mpscfifo.h>

/**
 * Print a ac_msg
 */
void ac_print_msg(ac_msg *pmsg);

/**
 * Print a ac_mpscfifo
 */
void ac_print_mpscfifo(ac_mpscfifo *pq);

#endif
