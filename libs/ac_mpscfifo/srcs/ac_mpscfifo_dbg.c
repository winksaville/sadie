/**
 * Debug code for mpscfifo
 */
#include <ac_printf.h>

#include "ac_mpscfifo_dbg.h"

/**
 * @see ac_mpscfifo_dbg.h
 */
void printMsg(ac_msg *pMsg) {
  if (pMsg != AC_NULL) {
    ac_printf("pMsg=%p pNext=%p pRspq=%p pExtra=%p cmd=%d\n", (void *)pMsg,
           (void *)(pMsg->pNext), pMsg->pRspq, pMsg->pExtra, pMsg->cmd);
  } else {
    ac_printf("pMsg == AC_NULL\n");
  }
}

/**
 * @see ac_mpscfifo_dbg.h
 */
void printMpscFifo(ac_mpscfifo *pQ) {
  if (pQ != AC_NULL) {
    ac_printf("pQ->pHead: ");
    printMsg(pQ->pHead);
    ac_printf("pQ->pTail: ");
    printMsg(pQ->pTail);
  } else {
    ac_printf("pQ == AC_NULL");
  }
}
