# Beta-channels: Message Passing with Communication Structures

## Introduction

This is a prototype implementation of the beta-channel message passing
library based on the concepts of _Communication structures_. The
programming model removes the inherent ambiguity of _process group_
based message passing models, where a given communication pattern
could be interpreted in different ways in order to fit the pattern
into the progamming model. Beta-channels removes these concerns by
ensuring that any given communication pattern can be implemented
immediately into a valid, and unique, implemenation without any
modification to the communication pattern.

## Compiling

Some MPI implementations require applications to use an MPI wrapper
compiler, which automatically includes the necessary header files.
These are generally named, `mpicc`. We shall configure our project
accordingly.

The following are the steps to compile this project:

     $ CC=`which mpicc` ./configure
     $ make
     $ make install


## References

* Gagarine Yaikhom, _Message Passing with Communication Structures_,
  [Ph.D. Thesis](https://www.era.lib.ed.ac.uk/handle/1842/921),
  [School of Informatics](http://www.inf.ed.ac.uk),
  University of Edinburgh, 2006.

* Gagarine Yaikhom, _Shared Message Buffering without Intermediate
  Memory Copy_, Submitted to High-level Parallel Programming and
  Applications (HLPP 2005).

* Gagarine Yaikhom, _Buffered Branching Channels with Rendezvous
  Message Passing_, Proceedings of the 23rd IASTED International
  Conference on Parallel and Distributed Computing and Networks,
  February 15-17, Innsbruck, Austria, pp. 184-193, 2005.

## History

I wrote the `libdsys` library and accompanying message passing
applications as part of my doctoral research. A large portion of the
work was completed in 2004.
