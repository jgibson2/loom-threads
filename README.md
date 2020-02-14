`loom` is a very easy-to-use actor framework for C++. Note that `loom` is still under development, with the following 
features yet to be implemented:

* ~Parallelizing `ActorSystemContext` data structures for concurrent access to the queue map (though the queues themselves are concurrent)~

* Adding publisher-subscriber, request-reply, and dealer patterns to the concurrent queues

* ~Return an iterator of objects to process instead of just one~

To get started, create an `Actor`:

```c++
#include <actor.h>

class MyActor: public Actor {
public:
    MyActor(std::shared_ptr<ActorSystemContext> ctx) : _ctx(ctx) { 
        queue = _ctx->getQueue<int>("my_queue");
        addPollable(queue->getPollable());
    }
    
private:
    std::shared_ptr<ActorQueue<int>> queue;

    void process() {
        auto opt = queue->receive();
        while(opt.has_value()) {
            std::cout << "Received " << opt.value() << std::endl;
            opt = queue->receive();
        }
        
    }
}
```

Now, we can simply do the following:
```c++
auto ctx = std::make_shared<ActorSystemContext>();
auto myActor = ctx->makeActor<MyActor>();
auto queue = ctx->getQueue<int>("my_queue");
for(int i = 0; i < 100; ++i) {
    queue->send(i);
}

```
And you will see the messages print in the terminal.

A few things to notice:

* The actors are created using the `ActorSystemContext`. You can create the `Actor`s yourself, but you are then responsible
for calling `run()` and `stop()`. If you create an actor that takes arguments, you can create an actor by calling `ctx->makeActor(bool flush, Args... args)`,
where `flush` indicates whether messages in the queue should be flushed or destroyed when the actor is stopped (default true).

* The queues must be registered with the actors using the `addPollable()` method. The library uses `epoll` for event-based triggers,
and each actor is triggered independently based on the queues it registers. Multiple queues can be registered with 
the same `Actor`.

* The `Actor` takes a `std::shared_ptr<ActorSystemContext>` and passes it to its parent constructor. In order to process events,
actors must implement the `process(void)` function, which will be called on the proper events. 

* Queues are accessed using the context by a unique identifying string (`"my_queue"`) and must be parametrized with the type. If the 
incorrect type is given, an exception will be thrown.

* Queues can be bulk-dequeued into a vector to operate on many elements after one receive call.