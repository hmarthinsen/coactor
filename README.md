# Coactor

C++20 actor framework using coroutines.

## Design

### Actors

- An **actor** has an **inbox** that contains a list of **messages**.
- An actor has a unique **address**.
- An actor can send messages to actors that it has the address to.
- Actors have **state**, i.e. stored data.
- Actors are isolated, in the sense that they can not access the data stored in other actors.
- Actors process the messages in the inbox in the same order as they were received.
- When a message is processed, the message and the actor state decides what the actor does.
- Actors can spawn new actors.

### Coroutines

A **coroutine** is a function that can be suspended, and then resumed later.
A suspended coroutine saves its local variables, and restores them when it is resumed.

We can implement actors using coroutines by creating a mechanism for receiving and sending messages between coroutines.

### Actor addresses

TODO

### Sending messages

TODO

### Receiving messages

TODO

### Spawning actors

TODO

### Messages

A message can contain: TODO

### Processing messages

TODO
