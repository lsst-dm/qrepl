# qrepl

This repo hosts work-in-progress on a prototype data replication system for Qserv.  The current plan is to
build a storage management system in C++, based on distributed hash table technology, borrowing heavily in
design from the Pastry/PAST projects.

The following papers present the technical details of Pastry/PAST, and will be useful to establish a context
for understanding the present work:

* [Pastry: Scalable, decentralized object location and routing for large-scale peer-to-peer systems](http://research.microsoft.com/en-us/um/people/antr/PAST/pastry.pdf)
* [PAST: A large-scale, persistent peer-to-peer storage utility](http://research.microsoft.com/en-us/um/people/antr/past/hotos.pdf)
* [Storage management and caching in PAST, a large-scale, persistent peer-to-peer storage utility](http://research.microsoft.com/pubs/65213/past-sosp.pdf)
