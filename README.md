# Nisse

![Nisse](img/Nisse.jpg)

**Nisse** is a minimal, from-scratch server architecture built to demonstrate how real production servers work—without the complexity of large frameworks or opaque abstractions.

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

The `NisseService` class provides a server that can listen simultaneously on multiple ports (via `listen()` method). Accepted connections are passed to a thread-pool for processing.

Underneath the hood the thread-pool uses non blocking IO (via ThorsSockets) and cooperative multi-tasking (boost co-routines) to asynchronously handle all connections with only a small pool of working threads. Threads automatically yield IO blocked connections back to the Job-Queue and start work on an alternative non-blocked connections.

To the service writer a socket is materialized as a `std::iostream` that provides a standard blocking interface, thus allowing service to be implement as a standard sync code without needing to explicitly consider how/when to switch threads.

### [Pynt](src/NisseServer/Pynt.h):

The Pynt interface is how service handler are written and provides a simple interface that can be registered with `NisseService::listen()` method.

The implementer must implement the interface `virtual PyntResult handleRequest(TAS::SocketStream& stream)`. The stream implements the `std::iostream` interface but a read/write operation that blocks will result in the server thread executing the interface to yield for another connection.

## HTTP:
### [PyntHTTP](src/NisseHTTP/PyntHTTP.h):

Provides an implementation of `Pynt` that understands the `HTTP` protocol and provides an intuitive to use interface.

The implementer must implement the interface `virtual void processRequest(Request& request, Response& response)`. The request object contains all the information from the HTTP request and the response object can be used to generate the reply. No action will result in `200 OK` with no message body. Normal usage is simply to add the reply headers the stream the result to the returned stream:

```C++
    class MyPyntHTTP: public PyntHTTP
    {
        public:
            virtual void processRequest(Request& request, Response& response) override
            {
                HeaderResponse      headers;
                headers.add("my-header", "header-value');
                response.addHeaders(header, Encoding::Chunked)
                    << "<html><body>"
                    << "My Page: " << request.getUrl().href() << "<br>"
                    << "</body></html>";
            }
    };
```

### [HTTPHandler](src/NisseHTTP/HTTPHandler.h):

Extends the `PyntHTTP` interface. Provides an interface to register lambdas to specific request paths.

```C++
    HTTPHandler     handler;

    handler.addPath("/content/{file}.html", [](Request& request, Response& response)
    {
        HeaderResponse      headers;
        headers.add("my-header", "header-value');
        response.addHeaders(header, Encoding::Chunked)
            << "<html><body>"
            << "My Page: " << request.getUrl().href() << "<br>"
            << "File: " << request.variables()["file"] << "<br>"
            << "</body></html>";
    });
```

Note: `{file}` will match any valid URL characters (except '/'). The matched characters are available in the variable `file` that is part of the `request` object.

## Building:

### Dependencies:
You will need [boost](https://www.boost.org/), [libEvent](https://libevent.org/)  and [ThorsMongo](https://github.com/Loki-Astari/ThorsMongo).  

To install boost libraries `brew install boost` or (check google for you platform).  

To install libEvent `brew install libEvent` or (check the libEvent website).  

To install ThorsMongo the easiest way to install is `brew install thors-anvil` but checkout the above project to see alternative ways to install or build.  

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


