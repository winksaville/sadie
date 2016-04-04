/*
 * copyright 2016 wink saville
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

#include <waiting_tcbs.h>

#include <interrupts_x86.h>

#include <ac_assert.h>
#include <ac_inttypes.h>
#include <ac_memmgr.h>
#include <ac_printf.h>

/**
 * Implementing a priority queue using an array based binary binary heap.
 * See: http://pages.cs.wisc.edu/~vernon/cs367/notes/11.PRIORITY-Q.html
 *
 * The waiting_tcbs array must be able to hold every tcb in the system +1,
 * the waiting_tcbs[0] isn't used hence the +1. The highest priority element
 * is waiting_tcbs[1] and is the root node which is the only node without
 * a paraent. Every node always has a priority >= any of its children.
 * Also, the shape of the tree is compete, meaning the following three properties
 * hold:
 * 1) All leaves are either at depth d or d-1
 * 2) All leaves at depth d-1 are to the right of leaves at depth d
 * 3.a) There is at most 1 node with just one child
 * 3.b) that child is to the left of its parent
 * 3.c) and it's the rightmost leaf at depth d.
 *
 * The root/highest priority node is at waiting_tcbs[1] (K=0 is unused)
 *
 * A nodes left child is at waiting_tcbs[K*2] and its right child
 * at waiting_tcbs[(K*2) + 1].
 *
 * A node K parent is at waiting_tcbs[K/2] using integer division.
 */

/**
 * Assumption: Currently only a single CPU is being used and in the future
 * when I do support multiple CPU's these will be CPU local and only used
 * by a single CPU. Thus there is no need for atomic operations.
 */
static tcb_x86** waiting_tcbs;
static ac_uint max_waiting_tcbs;
static ac_uint num_waiting_tcbs;

/**
 * Allocate the array
 */
static tcb_x86** allocate_array(ac_uint count) {
  tcb_x86** pwt = ac_malloc(sizeof(tcb_x86*) * count);
  return pwt;
}

/**
 * swap nodes array[n1] and array[n2]
 */
void swap_nodes(ac_uint n1, ac_uint n2) {
  tcb_x86 *n2_tcb = waiting_tcbs[n2];
  
  waiting_tcbs[n2] = waiting_tcbs[n1];
  waiting_tcbs[n1] = n2_tcb;
}

/**
 * Return the next tcb that is waiting its turn to become ready.
 * The tcb is NOT removed.
 */
tcb_x86* waiting_tcb_peek_intr_disabled(void) {
  if (num_waiting_tcbs > 0) {
    return waiting_tcbs[1];
  } else {
    return AC_NULL;
  }
}

/**
 * Add tcb to waitlist.
 *
 * @param ptcb is the tcb to add
 * @param absolute_tsc is the absolute tsc to wait until
 */
void waiting_tcb_add_intr_disabled(tcb_x86* ptcb, ac_u64 absolute_tsc) {
  //ac_printf("waiting_tcb_add_intr_disabled:+\n");
  //print_waiting_tcbs();

  // New Node Index is number currently waiting + 1
  ac_uint nni = ++num_waiting_tcbs;
  ac_assert(nni <= max_waiting_tcbs);

  // Store it as the right most node and then find where its final position should be
  waiting_tcbs[nni] = ptcb;
  ptcb->waiting_deadline = absolute_tsc;

  // Now loop up through the new nodes substree comparing with its parent
  // to find its proper position in the heap.
  while (nni > 1) {
    ac_uint parent = nni / 2;
    tcb_x86* pchild_tcb = waiting_tcbs[nni];
    tcb_x86* pparent_tcb = waiting_tcbs[parent];
    ac_u64 child_waiting_deadline = pchild_tcb->waiting_deadline;
    ac_u64 parent_waiting_deadline = pparent_tcb->waiting_deadline;

    if (child_waiting_deadline < parent_waiting_deadline) {
      swap_nodes(nni, parent);
    } else {
      break;
    }
    nni = parent;
  }

  //ac_printf("waiting_tcb_add_intr_disabled:-\n");
  //print_waiting_tcbs();
}

/**
 * Remove the next tcb.
 */
void waiting_tcb_remove_intr_disabled(void) {
  //ac_printf("waiting_tcb_remove_intr_disabled:+\n");
  //print_waiting_tcbs();

  //  Node Index is number currently waiting + 1
  if (num_waiting_tcbs == 0) {
    return;
  }

  ac_uint old_num = num_waiting_tcbs--;

  if (waiting_tcbs == 0) {
    // No waiting tcbs to remove
    return;
  }

  // Place the right most element at the end of the heap as the root.
  //store_node(old_num, 1);
  waiting_tcbs[1] = waiting_tcbs[old_num];

  // Now loop up through the substree comparing parent and
  // its children to find its proper position in the heap.
  ac_uint parent = 1;
  while (parent < num_waiting_tcbs) {
    //tcb_x86* pparent_tcb = waiting_tcbs[parent];

    ac_uint left_child = parent * 2;
    ac_uint right_child = left_child + 1;

    if (left_child > num_waiting_tcbs) {
      // No children for this node, so we're done
      break;
    }
    
    // Parent and left child exist get their deadlines
    ac_u64 parent_waiting_deadline = waiting_tcbs[parent]->waiting_deadline;
    ac_u64 left_child_waiting_deadline = waiting_tcbs[left_child]->waiting_deadline;

    // Assume left child is smaller than right_child if the right child exists
    ac_uint smaller_node = left_child;
    ac_u64 smaller_child_waiting_deadline = left_child_waiting_deadline;

    if (right_child <= num_waiting_tcbs) {
      // Both left and right clildren exist, get the right childs waiting deadline
      ac_u64 right_child_waiting_deadline = waiting_tcbs[right_child]->waiting_deadline;

      // See if its smaller than the left child
      if (left_child_waiting_deadline > right_child_waiting_deadline) {
        // Guessed wrong right child is smaller
        smaller_child_waiting_deadline = right_child_waiting_deadline;
        smaller_node = right_child;
      }
    }

    // We know which child is samller check if we should swap it with parent
    if (parent_waiting_deadline > smaller_child_waiting_deadline) {
      swap_nodes(parent, smaller_node);
    } else {
      // Parent is smaller, we're done
      break;
    }
    // We're not done, continue down the tree
    parent = smaller_node;
  }

  //ac_printf("waiting_tcb_remove_intr_disabled:-\n");
  //print_waiting_tcbs();
}

/**
 * Print a waiting_tcb_node
 *
 * @return AC_TRUE if the node existed and was printed
 */
ac_bool print_waiting_tcb_node(ac_uint depth, ac_uint ni) {
  if (ni <= num_waiting_tcbs) {
    ac_printf("%d:", ni);
    for (ac_uint i = 0; i < depth; i++) {
      ac_printf(" ");
    }
    char* node_type;
    if (ni == 1) {
      node_type = "root";
    } else if ((ni & 1) == 0) {
      node_type = "left";
    } else {
      node_type = "right";
    }
    ac_printf(" %s tcb=%x waiting_deadline=%ld\n",
        node_type, waiting_tcbs[ni], waiting_tcbs[ni]->waiting_deadline);
    return AC_TRUE;
  } else {
    return AC_FALSE;
  }
}

/**
 * Print the waiting tcbs subtree
 *
 * @return AC_TRUE if there were nodes printed
 */
static ac_bool print_waiting_tcbs_subtree(ac_uint depth, ac_uint ni) {
  if (print_waiting_tcb_node(depth, ni)) {
    // Print the left subtree
    print_waiting_tcbs_subtree(depth + 1, ni * 2);
    // Print the right subtree
    print_waiting_tcbs_subtree(depth + 1, (ni * 2) + 1);
    return AC_TRUE;
  } else {
    return AC_FALSE;
  }
}

/**
 * Print the waiting tcbs
 */
void print_waiting_tcbs(void) {
  if (!print_waiting_tcbs_subtree(0, 1)) {
    ac_printf("No waiting tcbs\n");
  }
}

/**
 * Update the number of waiting tcbs. Note this is slow and
 * currently disables interrupts during the transition.
 */
void waiting_tcbs_update_max(ac_uint new_max) {
  ac_uint flags = disable_intr();
  {
    ac_uint cur_max = max_waiting_tcbs;
    if (new_max > cur_max) {
      tcb_x86** new = allocate_array(new_max);
      for (ac_uint i = 0; i < cur_max; i++) {
        new[i] = waiting_tcbs[i];
      }
      max_waiting_tcbs = new_max;
      *(void**)waiting_tcbs = (void**)new;
    }
  }
  restore_intr(flags);
}

/**
 * Initialize the waiting tcb data structures
 *
 * @param max is the maximum number of waiting tcbs
 */
void waiting_tcbs_init(ac_uint max) {
  max += 1; // There must be an extra one since waiting_tcbs[0] isn't used

  waiting_tcbs = allocate_array(max);
  ac_assert(waiting_tcbs != AC_NULL);

  max_waiting_tcbs = max;
  num_waiting_tcbs = 0;
}
