*********************************************************************
*                                                                   *
*   Beta-channels: Message Passing with Communication Structures.   *
*                                                                   *
*                       Gagarine Yaikhom                            *
*                                                                   *
*********************************************************************

INTRODUCTION

This is a prototype implementation of the beta-channel message passing
library based on the concepts of `Communication structures'. The
programming model removes the inherent ambiguity of `process group'
based message passing models, where a given communication pattern
could be interpreted in different ways in order to fit the pattern
into the progamming model. Beta-channels removes these concerns by
ensuring that any given communication pattern can be implemented
immediately into a valid, and unique, implemenation without any
modification to the communication pattern.


REFERENCES

[1] Gagarine Yaikhom, "Message Passing with Communication Structures",
	Ph.D. Thesis, School of Informatics, University of Edinburgh, 2006.

[2] Gagarine Yaikhom, "Shared Message Buffering without Intermediate
	Memory Copy", Submitted to High-level Parallel Programming and
	Applications (HLPP 2005).

[3] Gagarine Yaikhom, "Buffered Branching Channels with Rendezvous
	Message Passing", Proceedings of the 23rd IASTED International
	Conference on Parallel and Distributed Computing and Networks,
	February 15-17, Innsbruck, Austria, pp. 184-193, 2005.


