
### Dealing with Unprompted Events (e.g. `Page.loadEventFired`)

The correlator map perfectly solves command-response matching using the `id`. 

However, `stdexec` pipelines still struggle with unprompted events because a pipeline represents a *single* logical sequence, whereas events like `Page.loadEventFired` happen out-of-band. 

**The standard idiom for unprompted events:**
Instead of trying to force unprompted browser events into the linear pipeline, you should have `handle_unprompted_event` resolve a separate promise or fire a lightweight callback. 

If you *must* wait for an event inside a pipeline (e.g., "Navigate, *then wait for loadEventFired*, then query DOM"), you use the exact same `async_initiate` trick. You write an `async_wait_for_event("Page.loadEventFired", use_sender)` function that stores a handler in a `std::multimap<std::string, Handler>`. When the websocket receives an event with no ID, it looks at the `method` string, finds the matching handlers in the multimap, and completes them.
