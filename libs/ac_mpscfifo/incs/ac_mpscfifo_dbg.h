/**
 * Debug code for mpscfifo
 */

#ifndef SADIE_LIBS_AC_MPSCFIFO_AC_MPSCFIFO_DBG_H
#define SADIE_LIBS_AC_MPSCFIFO_AC_MPSCFIFO_DBG_H

#include <ac_mpscfifo.h>

/**
 * Print a ac_msg
 */
void printMsg(ac_msg *pMsg);

/**
 * Print a ac_mpscfifo
 */
void printMpscFifo(ac_mpscfifo *pQ);

#endif
