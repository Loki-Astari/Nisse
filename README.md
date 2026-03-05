# Nisse

![Nisse](img/Nisse.jpg)

**Nisse** is a minimal, from-scratch server architecture built to demonstrate how real production servers work—without the complexity of large frameworks or opaque abstractions.

**Online documentation:**
- https://loki-astari.github.io/ThorsAnvil/NisseServer.html
- https://loki-astari.github.io/ThorsAnvil/NisseHTTP.html

It walks step-by-step from a basic TCP listener to a secure, multi-threaded HTTPS server, using modern C++ and standard system primitives. The goal is to make every layer understandable, debuggable, and hackable.

You can explore the full evolution of Nisse in this series:

1. [Nisse: Origins — Design Goals & Philosophy](https://lokiastari.com/posts/NisseOrigins)  
2. [Building a Basic Web Server](https://lokiastari.com/posts/NisseV1)  
3. [Low-Level C++ Socket Programming](https://lokiastari.com/posts/NisseV2)  
4. [Adding SSL/TLS Certificates](https://lokiastari.com/posts/NisseV3)  
5. [Introducing Multi-Threading](https://lokiastari.com/posts/NisseV4)

If you'd like to understand how servers work from the metal up—this is a good place to start.

If you don't want to handle the intricacies yourself you can use the [`mug`](https://github.com/Loki-Astari/Mug) an implementation of a simple server (like python flask) that allows you to quickly and simply build plugins that can be dynamically loaded.

## Nisse:
### [NisseServer](src/NisseServer/NisseServer.h):

The `ThorsAnvil::Nisse::Server::NisseServer` class provides a server that can listen simultaneously on multiple ports (via `listen(ServerInit, Pynt&)`). Accepted connections are dispatched onto a worker thread-pool for processing.

Underneath the hood the workers use non-blocking IO (via `ThorsSocket`) and cooperative multi-tasking (via `boost::coroutines2`) to asynchronously handle many connections with a small pool of threads. When an IO operation would block, the currently-running request yields back to the event loop; the worker thread immediately continues work on some other connection that is ready.

To the service writer, a connection is presented as a `ThorsAnvil::ThorsSocket::SocketStream` (a standard iostream-style interface). Your handler code can be written as normal “blocking” stream code; the server transparently yields/resumes when the underlying non-blocking socket is not ready.

Lifecycle:
- Call `listen(...)` one or more times to attach handlers (`Pynt`) to ports.
- Call `run()` to start the libevent loop.
- Call `stopSoft()` / `stopHard()` to stop the event loop.

### [Pynt](src/NisseServer/Pynt.h):

The Pynt interface is how service handlers are written and provides a simple interface that can be registered with `NisseServer::listen()`.

The implementer must implement:

`virtual PyntResult handleRequest(ThorsAnvil::ThorsSocket::SocketStream& stream, Context& context)`

The `stream` behaves like a normal iostream, but any read/write that would block will yield so the worker can make progress elsewhere.

The `context` allows advanced integrations (registering additional async `Socket`/`SocketStream` objects associated with the current request via the RAII helpers in [`Context.h`](src/NisseServer/Context.h)). This is used by examples like streaming a file (`WebServer`) or proxying to an upstream service (`ReverseProxy`) without blocking a worker thread.

## HTTP:
### [PyntHTTP](src/NisseHTTP/PyntHTTP.h):

Provides an implementation of `Pynt` that understands the `HTTP` protocol and provides an intuitive to use interface.

The implementer must implement:

`virtual void processRequest(Request& request, Response& response)`

The request object contains the parsed HTTP request and the response object is used to generate the reply.

Normal usage:
- Call `response.addHeaders(...)` (optional; must happen before starting the body)
- Write the body to `response.body(...)`

If you never start the body, the response defaults to `200 OK` with `content-length: 0`.

```C++
    class MyPyntHTTP: public PyntHTTP
    {
        public:
            virtual void processRequest(Request& request, Response& response) override
            {
                HeaderResponse      headers;
                headers.add("my-header", "header-value");
                response.addHeaders(headers);
                response.body(Encoding::Chunked)
                    << "<html><body>\n"
                    << "My Page: " << request.getUrl().href() << "<br>\n"
                    << "</body></html>\n";
            }
    };
```

### [HTTPHandler](src/NisseHTTP/HTTPHandler.h):

Extends the `PyntHTTP` interface. Provides an interface to register lambdas to specific request paths.

```C++
    HTTPHandler     handler;

    handler.addPath("/content/{file}.html", [](Request const& request, Response& response)
    {
        HeaderResponse      headers;
        headers.add("my-header", "header-value");
        response.addHeaders(headers);
        response.body(Encoding::Chunked)
            << "<html><body>\n"
            << "My Page: " << request.getUrl().href() << "<br>\n"
            << "File: " << request.variables()["file"] << "<br>\n"
            << "</body></html>\n";
        return true; // true = handled; false = let matcher continue
    });
```

Notes:
- `{name}` captures **everything up to the next literal part of the pattern** (it may include `/`), and the captured value is URL-decoded before being stored.
- Captures are exposed via `request.variables()["name"]`.
- `HTTPHandler` also populates `request.variables()` with request headers (lower-cased names) and query parameters. If `content-type` is `application/x-www-form-urlencoded`, it will also parse the body into variables.

### Control handlers
- [`PyntControl`](src/NisseServer/PyntControl.h): a very simple control handler that stops the server when connected to.
- [`PyntHTTPControl`](src/NisseHTTP/PyntHTTPControl.h): an HTTP control handler that supports `?command=ping|stopsoft|stophard`.

## Building:

### Dependencies:
You will need [boost](https://www.boost.org/) (coroutines2), [libevent](https://libevent.org/) and the Thors Anvil libraries used by Nisse (notably `ThorsSocket`, logging, and serialization).  
`ThorsMongo` is only required if you want to build/run the `MongoRest` example.  

To install boost libraries `brew install boost` (or use your platform package manager).  

To install libevent `brew install libevent` (or see the libevent website).  

To install the Thors Anvil libraries (including `ThorsSocket`) the easiest way is `brew install thors-anvil` (or build from source if you prefer).  

### Compile and Run:
```bash
> git clone https://github.com/Loki-Astari/Nisse.git
> cd Nisse
> ./configure
> make
> ./build/bin/HelloWorld
```

## Nisse

From the [Scandinavia folklore](https://en.wikipedia.org/wiki/Nisse_\(folklore\)). Small gnome like creatures that live in the farm barn and will help out with tasks in exchange for gifts.

Since `Nisse` like a small drink the interface to them is a `Pynt` (Pint).

## Example Applications:

### [HelloWorld](src/Examples/HelloWorld/HelloWorld.cpp)

Provides a trivial server that that will serve an HTML "Hello-World" page for the paths: `/HW{Who}.html` and `/CK{Who}.html` that are programmatically generated via the streams. Note: **ALL** the streams are async non-blocking. Any IO operations that would block release the thread to do processes other requests.

### [WebServer](src/Examples/WebServer/WebServer.cpp):

Provides a trivial web server that that will serve an HTML pages from files on the local filesystem. Note: **ALL** the streams are async non-blocking. Any IO operations that would block release the thread to do processes other requests.

### [ReverseProxy](src/Examples/ReverseProxy/ReverseProxy.cpp):

Provides a trivial reverse proxy that that will forward requests to an application server and return the result to the original connection. Note: **ALL** the streams are async non-blocking. Any IO operations that would block release the thread to do processes other requests.


