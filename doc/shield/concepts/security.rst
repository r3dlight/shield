Security in libShield
---------------------

.. index::
  single: security-model; definition

Security requirements
^^^^^^^^^^^^^^^^^^^^^

.. todo::
  To be described


About stack smashing protection
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Stack smashing protection is a basic protection mechanism that include in each stack frame
a guardian that is forged at runtime during function preamble and checked at function postambule.

This guardian is denoted Canary and is manipulated by compiler's primitives that are embedded
in the software runtime, called during each stack frame creation and leave.
This primitives are activated with the usual `stack-protector`, `stack-protector-strong`, flags,
and need to be seeded at runtime for each thread in order to generate per-stack entropy, so that
each thread has its own canary sequence.

In order to seed each task thread, the userspace `_start` entrypoint implementation is included
in the libShield and take over the initial application boot sequence, starting with delivering
a per-job seed.

.. note::
  The usage of `_start` symbol in the application runtime allows to properly forge
  application environment at job boot time, and properly support application termination
  at job end time without requiring any single line of code from the application developer
