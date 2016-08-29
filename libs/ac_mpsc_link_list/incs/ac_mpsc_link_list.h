/**
 * This software is released into the public domain.
 *
 * A MpscLinkList is a wait free/thread safe multi-producer
 * single consumer first in first out queue using a link list.
 * This algorithm is from Dimitry Vyukov's non intrusive MPSC code here:
 *   http://www.1024cores.net/home/lock-free-algorithms/queues/non-intrusive-mpsc-node-based-queue
 */

#ifndef SADIE_LIBS_AC_MPSC_LINK_LIST_H
#define SADIE_LIBS_AC_MPSC_LINK_LIST_H

#include <ac_mpsc_link_list_internal.h>

#include <ac_msg.h>
#include <ac_inttypes.h>
#include <ac_status.h>

typedef struct AcMpscLinkList AcMpscLinkList;

/**
 * Initialize an AcMpscLinkList. Don't forget to empty the fifo
 * and delete the stub before freeing AcMpscLinkList.
 */
extern AcStatus AcMpscLinkList_init(AcMpscLinkList* list);

/**
 * Deinitialize the AcMpscLinkList.
 *
 * @return number of messages removed.
 */
extern AcU64 AcMpscLinkList_deinit(AcMpscLinkList* list);

/**
 * Add a AcMsg to the head of the link list. This maybe used by multiple
 * entities on the same or different thread. This will never
 * block as it is a wait free algorithm.
 */
extern void AcMpscLinkList_add(AcMpscLinkList* list, AcMsg* msg);

/**
 * Remove a AcMsg from the tail of the link list. This maybe used only by
 * a single thread and returns NULL if empty. This may stall if a producer
 * calls add and was preempted before finishing.
 */
extern AcMsg* AcMpscLinkList_rmv(AcMpscLinkList* list);

#endif
