Alarm
"""""

**API definition**:

.. code-block:: rust
   :caption: Rust UAPI for alarm syscall

   mod uapi {
      fn alarm(timeout_ms: u32) -> Status
   }

.. code-block:: c
   :caption: C UAPI for alarm syscall

   enum Status sys_alarm(uint32_t timeout_ms);

**Usage**:

Ask the kernel to emit a SIGNAL_ALARM signal in `timeout_ms` miliseconds to the current job.

This syscall is usefull when implementing userspace software-based timers. Although, there
are some restrictions:

   * requested alarms can't be removed before they expire
   * the system support a maximum number of concurrently configured alarms upto one alarm
     per task if all task requires an alarm in the same time


.. code-block:: C
   :linenos:
   :caption: sample bare usage of sys_alarm

   if (sys_alarm(200) != STATUS_OKAY) {
      // [...]
   }
   res = sys_wait_event([...]);
   /* alarm received 200ms later */

.. note::
   as SIGNAL_ALARM can also be emitted by other tasks through the signal syscall, the
   alarm emitted by the kernel due to a alarm() request hold the current task
   identifier as signal source

**Required capability**

   None.

**Return values**

   * STATUS_BUSY if the delay manager do not have any space for the requested alarm
   * STATUS_OK
