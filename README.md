# semaphores
Operating System: Linux

A program done for educational purposes that simulates a msg send/receive environment
with a limited size mailbox, using POSIX library.

Messages are sent in the form of threads that are generated randomly 
to increment a shared variable (counter) then die.
Another thread (monitor) that wakes up at random time intervals, reads the counter variable and clears it,
then places the messages in the buffer(mailbox) one by one, it cannot add messages to a full buffer.
A third thread (collector) reads the messages from the buffer, it cannot read from an empty buffer.
