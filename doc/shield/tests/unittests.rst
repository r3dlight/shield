Unit testing Shield
-------------------

.. _unittest:

.. index::
    single: unit test; model
    single: unit test; test suites
    single: gtest; tests design

Unit testing framework
""""""""""""""""""""""

Even if the libshield is targetting embedded systems, it is designed to also
easily support x86_64 target in order to allow unit testing.

The global test model is that any peace of code in Shield can be extracted, compiled for
x86_64, and linked to a test source that implement the potential missing blocs and
validate the behavior of the code under test in various cases. This peace of code,
when being tested, *system under test* (SUT).

.. note::
    SUT (System under test) is a test terminology defined by the
    `International Software Testing Qualifications Board <https://www.istqb.org/#welcome>`_,
    for software-related testing

To do that, the gtest framework delivers multiple useful components such as
mocks, to trigger execution of test fixtures when the source code calls external
symbols. In the same time, the usage of C++ allows templated testing, that
permit to forge a great amount of inputs and stimulii to various Shield modules.

The Shield tests suites are natively integrated into meson and are the following:

   * ut-managers: test suite targetting managers
   * ut-bsp: test suite targetting drivers
   * ut-utils: test suite targetting generic kernel utilities and core library


The Shield test support is integrated into the build system, and associated to the
Sentry static analyser to ensure coverage metrics.

.. index::
    single: unit test; development constraint
    single: unit test; architectural model

Building and testing modules on build host
""""""""""""""""""""""""""""""""""""""""""

Each subcomponent of Shield is made to use a controlled, clearly defined, external
API, so that it is easy to unit test each Shield subcomponent separately by
mocking the used external API symbols.

In order to easily support reference POSIX impleementation comparison, typically
with the GNU libc, all Shield POSIX symbols implementation are defined with the
``shield_`` prefix, so that they are aliased to the POSIX-PSE51 symbol at compile
time when not being under test.

Such a model allow an easy Shield versus glibc comparison and conformance in
the unit tests.

.. todo::
    to complete with examples
