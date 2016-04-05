#qdht

qdht is a C++ distributed hash table implementation based on the design of the [Pastry]
(http://research.microsoft.com/en-us/um/people/antr/PAST/pastry.pdf) DHT.  qdht makes use of C++11 language
features, leverages [boost::asio](http://www.boost.org/doc/libs/master/doc/html/boost_asio/overview.html) for
aynchronous network communication, and leverages [capnproto](https://capnproto.org) for wire protocols.

This module contains the core qdht classes for nodes in the overlay network, IDs for nodes and messages,
and the basic overlay messaging API.  Overlay network join/quit, message routing, and maintenance are
implemented here.  Higher level services (e.g. qstore replicated storage) are built on top of the
infrastructure provided here.

##API classes

The principal class in the qdht API is `qdht::Node`, which represents a node in the overlay network -- a
potential source or a delivery destination for qdht client messages.  To participate in a qdht cluster, you
first instantiate a `qdht::Node`, then join that instance to a cluster.

Every overlay node has an associated `qdht::Id` -- a 160-bit entity that uniquely identifies the node
within the DHT.  IDs are structured in a ring (note "ring" is used here in its naive geometric sense; not a
formal algebraic ring) with a modulo distance metric.  The ring of IDs may be visualized like a unit-circle,
with ID 0 at the zero degree position, proceeding around the circle counter-clockwise as Ids increase, and
ending with ID (2^160)-1 just before returning to 0 again.

Messages in qdht are not routed to a particular destination, but rather toward a particular target ID.  A
message will be delivered to a participating node with the closest ID to the the target at the time that
the message is being routed.  Nodes conspire with one another behind the scenes to keep distributed routing
state updated dynamically.  Nodes are homogeneous and may join or leave the cluster at any time without
destabilizing message routing or delivery; all state is distributed and ephemeral.

qdht is designed to support multiple clients per node, where a client may be a higher level service
implemented above the qdht messaging API using its own dedicated message suite.  A client registers at a
`qdht::Node` by filling in a `qdht::Client` struct with a client identifier and a set of message
routing and delivery callback function objects.  Messages within qdht are encoded for wire transfer using
capnproto, and each client can extend the protocol with its own customized capnproto message class or
classes.

