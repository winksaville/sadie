/**
 * Debug code for mpscfifo
 */
#include <ac_printf.h>

#include "ac_mpscfifo_dbg.h"

/**
 * @see ac_mpscfifo_dbg.h
 */
void ac_print_msg(ac_msg *pmsg) {
  if (pmsg != AC_NULL) {
    ac_printf("pmsg=%p pnext=%p pRspq=%p pExtra=%p cmd=%d\n", (void *)pmsg,
           (void *)(pmsg->pnext), pmsg->prspq, pmsg->pextra, pmsg->cmd);
  } else {
    ac_printf("pmsg == AC_NULL\n");
  }
}

/**
 * @see ac_mpscfifo_dbg.h
 */
void ac_print_mpscfifo(ac_mpscfifo *pq) {
  if (pq != AC_NULL) {
    ac_printf("pq->phead: ");
    ac_print_msg(pq->phead);
    ac_printf("pq->ptail: ");
    ac_print_msg(pq->ptail);
  } else {
    ac_printf("pq == AC_NULL");
  }
}
