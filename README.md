# Coactor

Actor framework with C++20 coroutines

## Design

An **actor** has an **inbox** that contains a list of **messages**.
An actor has a unique address.
An actor can send messages to actors that it has the adress to.
Actors have state, i.e. stored data.
Actors are isolated, in the sense that they can not access the data stored in other actors.
Actors process the messages in the inbox in the same order as they were received.
When a message is processed, the message and the actor state decides what the actor does.
Actors can spawn new actors.

### Actor adresses

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
