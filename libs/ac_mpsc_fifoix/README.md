Multiple Producer Single Consumer FIFO

A C implementation of a MPSC FIFO, this implementation
is wait free/thread safe. This algorithm is from Dimitry
Vyukov's non intrusive MPSC code here:
  http://www.1024cores.net/home/lock-free-algorithms/queues/intrusive-mpsc-node-based-queue

The fifo has a head and tail, the elements are added
to the head of the queue and removed from the tail.
