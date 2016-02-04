#qdht

qdht is a C++ distributed hash table implementation based on the design of the [Pastry](http://research.microsoft.com/en-us/um/people/antr/PAST/pastry.pdf) DHT.  qdht makes use of C++11 language features, leverages [boost::asio](http://www.boost.org/doc/libs/master/doc/html/boost_asio/overview.html) for aynchronous network communication, and leverages [capnproto](https://capnproto.org) for wire protocols.

This module contains the core qdht classes, which model nodes in the overlay network, node and message IDs, and the basic overlay messaging API.  Node discovery/join, inter-node message routing, and overlay network maintenance are implemented here.  Higher level services (e.g. qstore replicated storage) are built on top of the messaging API and infrastructure provided here.

## Metric

IDs in qdht are 160-bit entities that are used to uniquely identify overlay network nodes and message destinations.  qdht IDs are structured in a "ring" (note: "ring" used here in its geometric sense; not a formal algebraic ring) with a modulo distance metric.  The ring of IDs may be visualized like a unit-circle, with ID 0 at the zero degree position, proceeding around the circle counter-clockwise as IDs increase, and ending with ID (2^160)-1 just before returning to 0 again.

## State Tables

### Routing Table

### Leaf Set

## Message Routing

## Joining the Overlay

## Overlay Maintenance

