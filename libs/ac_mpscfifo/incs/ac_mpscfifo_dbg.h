/**
 * Debug code for mpscfifo
 */

#ifndef SADIE_LIBS_AC_MPSCFIFO_AC_MPSCFIFO_DBG_H
#define SADIE_LIBS_AC_MPSCFIFO_AC_MPSCFIFO_DBG_H

#include <ac_mpscfifo.h>

/**
 * Print a Msg_t
 */
void printMsg(Msg_t *pMsg);

/**
 * Print a MpscFifo_t
 */
void printMpscFifo(MpscFifo_t *pQ);

#endif
