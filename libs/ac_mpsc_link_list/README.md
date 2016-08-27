Multiple Producer Single Consumer FIFO using a linked list.

A C implementation of a MPSC FIFO using a Link List, this
implementation is wait free/thread safe. This algorithm is
from Dimitry Vyukov's non-intrusive MPSC code here:
  http://www.1024cores.net/home/lock-free-algorithms/queues/non-intrusive-mpsc-node-based-queue

It's actually an instrusive implementation using the non-instrusive
algorithm. To manage the instrustive link list there is a
data structure, AcNextPtr, and there must be one AcNextPtr
for each msg on the link list. Since its in determinate the
number of msg's that could be on any given list, but each
message that's added to the list must have one it made sense
to have each message either contain a AcNextPtr or have a pointer
to one. I did test having an AcNextPtr embedded in each AcMessage
but the performance was better when I allocated them in an array.
So that why the current implementation uses a pointer.

This could be revisited in the future and maybe there  is an
efficient alogrithm for managing the AcNextPtr's but this
is working and has decent performance.
