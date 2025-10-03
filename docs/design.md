# Design

The **runtime** owns the schedulers and their associated threads, and the actors.
The runtime has a set of thread-safe methods that are intended to be called by the actors for **sending messages** and **spawning actors**.

The **schedulers** orchestrate the execution of actors.
Each actor is assigned to a scheduler.
Each actor has a **status** that determines how the actor is scheduled.
When an actor is selected for execution, it is added to the **ready queue**.
The schedulers are *not* involved in messaging.

The **actors** are self-contained entities that have a message queue.
The actors have are given a callback from their schedulers that the actors call when they are Ready and want to be inserted into the ready queue.

## Sending a message

Two possible designs for how an actor notifies its scheduler when it is ready:

1. Directly by having the actor call a callback given to it by its scheduler.
2. Indirectly by letting the runtime call a method on the scheduler for adding the actor to the ready queue.

The direct method makes it so that the runtime doesn't have to do anything.
The indirect method makes it so that the actor doesn't need to have any references to its scheduler.
Let's go for the direct method.

An actor A sends a message M to the actor with address X in he following way:

1. A calls a thread-safe runtime method `R.send` to send M to the actor with address X.
2. `R.send` looks up the actor B that has address X in the address-actor table.
3. `R.send` then calls the thread-safe `B.append_msg` method on B.
4. `B.append_msg` does the following:
   1. If B has status Done, nothing happens and the method returns immediately.
   2. Else, M is appended to B's message queue.
   3. If B has status Blocked, the status is set to Ready and the `S.notify_ready` callback is called to inform the scheduler about the change in status.
   4. In `S.notify_ready`, X is added to the ready queue.
5. Control returns to A.

## Spawning an actor

An actor A spawns an actor B in the following way:

1. A calls a thread-safe runtime method `R.spawn<B>`.
2. `R.spawn<B>` does the following:
   1. It creates the new actor B and address Y and stores them in the address-actor table.
   2. It then reads and updates the next scheduler to use.
   3. It then inserts the actor into a scheduler T by calling `T.insert_actor`.
   4. `T.insert_actor` adds Y to T's address set and adds Y to T's ready queue.
3. Control returns to A.

## Receiving a message

Actor A wants to receive a message.
There are three cases.

### Message queue not empty

If there is a message already waiting in A's message queue, the front message in the queue will be popped and returned.

### Message queue empty, without timeout

If the message queue is empty and there is no timeout set, the following happens:

1. A's status is set to Blocked, its coroutine is suspended and control returns to its scheduler S, in the `S.run` method.
2. Execution continues with S's other ready actors.
3. Execution of A continues when another actor sends it a message, as described in [Sending a message](#sending-a-message).

### Message queue empty, with timeout

If the message queue is empty and a timeout T with message M is set, the following happens:

1. A's status is set to Blocked, its coroutine is suspended and control returns to its scheduler S, in the `S.run` method. T and M are passed back to S.
2. S associates T to A and M in its timeout-address-message table.
3. Execution continues with S's other ready actors.
4. If another actor sends A a message before the timeout, A's status is set to Ready, and it is added to the ready queue. The timeout is removed.
5. Else, if S is blocked, before a message is sent to A, the following happens:
   1. At the moment when S is blocked, i.e. when its ready queue is empty, the scheduler will wait for an actor becoming Ready or the timeout happening.
   2. Assuming that the timeout happened, the following happens:
      1. S calls `R.send` to send M to A, as described in [Sending a message](#sending-a-message).

## Data structures

### Runtime

| Name                | Mutex? |
| ------------------- | ------ |
| address-actor table | Y      |
| next scheduler      | Y      |

### Scheduler

| Name                          | Mutex? |
| ----------------------------- | ------ |
| ready queue                   | Y      |
| timeout-address-message table | N      |

### Actor

| Name          | Mutex? |
| ------------- | ------ |
| message queue | Y      |
| status        | Y      |
