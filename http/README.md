This is a somewhat bare-bones C++ HTTP server, inspired by things like Express (http://expressjs.com/) and 
Martini (https://github.com/go-martini/martini).

The intended use-case is for embedding in C++ back-end services, to conveniently provide utility HTTP 
services (rest APIs and/or served status pages and small browser applications) for things like diagnostics 
and monitoring.  It was endeavored to keep the code size small and code complexity low.  Some design 
decisions toward that end:

* Asynchronous, but single threaded.  This leverages boost::asio's event proactor, and keeps complexity low 
by avoiding many synchronization requirements in handlers and dispatch.  Since the server is asynchronous, 
multiple simultaneous connections are supported robustly, but since it is also single-threaded throughput 
will likely not scale to hundreds or thousands of simultaneous connections.

* Express and Martini style "middlewares" were not implemented to keep complexity low.  This could be added 
at a later time if desired.  For now, perceived-to-be-commonly-used functionalities are wired directly into 
the server.

#### Features currently supported

* Express and Martini style Request and Response objects, and URL path specifiers with wildcarding and 
parameter capture for conveniently implementing REST services.

* Static content serving out of one-or-more file system directories, with file-extension-based automatic 
Content-Type detection.

* AJAX endpoint helpers: push a boost::property_tree to the server-side endpoint at any time and all 
currently pending clients will be updated with the corresponding JSON representation.

* HTTP 1.1 persistent connections.

* Can piggy-pack on existing asio::io_service instance from hosting application if desired.

#### Potential TODO

* Documentation, comments, unit tests (typical...)
* Cache control headers.
* Websocket support.
* HTTPS support.
* A reciprocal HTTP client library.
