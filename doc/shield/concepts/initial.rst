Initial conception model
------------------------

.. index::
   single: libhsield; principle

Initial considerations
^^^^^^^^^^^^^^^^^^^^^^


Libshield is an embedded implementation of the POSIX-PSE51 API.
Its goal is to deliver a well-known API support to users over various proprietary
or non-POSIX kernels for small embedded systems, in the same way newlib or
embedded-artistry libc does.

The slightly difference between others embedded libc and libshield is the
level of security targetted in this very library.

We aim, in that POSIX implementation, to go upto formal analysis and functional corrtectness
of as much as possible percentage of the library, which require a way to implement all
core functionalities with high semantic constraints.

Formal-proofness
^^^^^^^^^^^^^^^^

In order to achieve a high security level, formal correctness is done using
the FramaC framework, including both noRTE (Eva) and correctness (WP) analysis of
the source code.

For POSIX-PSE51 compliance, the FramaC MetaCSL plugin is also considered in order to
demonstrate the POSIX compliance of the implemented PSE51 APIs.
