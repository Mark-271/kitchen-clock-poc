Do not use "volatile" modifier
==============================

:Author: Sam Protsenko

``volatile`` keyword purpose is to turn off the compiler optimizations for
caching the variable in the register. Very rarely its usage is justified. This
document discusses why its usage should be avoided at all in the firmware code.
It's also discouraged to use ``volatile`` in Linux kernel, see [1]_ for details.

As discussed below, ``volatile`` shouldn't be used at all. Using provided in
the firmware sync API (like ``READ_ONCE()``, ``WRITE_ONCE()``,
``enter_critical()``, ``exit_critical()`` and ``barrier()``) and libopencm3 API
for accessing the memory-mapped registers (like ``MMIO32()``) takes care of
disabling optimizations for us, so that we can avoid using ``volatile`` at all.


Summary
-------

Here is a short cheat sheet on when to use which API:

 * **Never** use ``volatile`` keyword explicitly in your code
 * Use existing API to access hardware registers (like ``MMIO32()``)
 * Use ``READ_ONCE()`` (in mainline code): for accessing global variables that
   change in ISR (to avoid compiler optimizations)
 * Use ``WRITE_ONCE()`` (in ISR code): when changing global variables
   (pairing it with ``READ_ONCE()`` helps compiler to understand variable usage)
 * Use local non-volatile variable to keep ``READ_ONCE()`` result
   (to avoid under-optimization)
 * Use critical sections ( ``enter_critical()``, ``exit_critical()`` ) to
   perform non-atomic operations (Read-Modify-Write, multi-word variable access,
   unaligned access, etc) in mutual-exclusive manner
 * There is no need to use ``READ_ONCE()`` if you are already inside of
   critical section
 * If you need atomic bit operations (bit-wise RMW per once CPU instruction),
   consider using bit-banding mechanism (see ``BBIO_SRAM()`` and
   ``BBIO_PERIPH()`` API in libopencm3)
 * Use ``barrier()`` to avoid compiler reordering of load/store instructions
   (e.g. to keep the order of two sequntial atomic operations)
 * For implementing delay loops: use either ``READ_ONCE()`` for checking
   the condition or just add ``barrier()`` inside the loop. Even better idea is
   to use existing API like ``mdelay()`` or software timers API to implement
   non-blocking sleep.

Details are discussed further.


Using ``volatile`` for ISR modifiable variable
----------------------------------------------

In Embedded world the ``volatile`` is often used for global variables being
modified from ISR. Compiler can't see the access to that variable from ISR, as
ISR is not executed from any other function. Here is an example:

.. code-block:: c

    static int a;

    int main(void)
    {
        while (a == 0) { }
    }

    void some_isr(void)
    {
        a++;
    }

In this case a compiler indeed throws the ``a == 0`` check away, which leads to
infinite loop. And a programmer might have a temptation to add ``volatile``
modifier to ``a`` variable declaration, like this:

.. code-block:: c

    static volatile int a;

    int main(void)
    {
        while (a == 0) { }
    }

    void some_isr(void)
    {
        a++;
    }

It disables optimizations for ``a`` variable, so compiler will produce correct
binary. But it's not recommended to use this approach because:

  * using ``volatile`` in more elaborate cases is very inefficient, as it
    disables optimizations for all accesses to the variable
  * when calling some functions on that variable, functions must also take
    ``volatile`` parameters, which gets ugly very fast; think about implementing
    queue struct and functions and using it on ``volatile`` variable
  * using ``volatile`` doesn't fix reordering of memory operations (which can be
    done by compiler and CPU); though it is guaranteed that accesses to
    ``volatile`` variable will be ordered, compiler still can generate
    out-of-order instructions between ``volatile`` variable and other variables.
    And of course the processor is free to do any reordering anyway.
  * using ``volatile`` doesn't make variable operations atomic

The C Standard mentions::

    If it is necessary to access a nonvolatile object using volatile semantics,
    the technique is to cast the address of the object to the appropriate
    pointer-to-qualified type, then dereference that pointer.

So here is a better way to disable optimizations using ``volatile`` "in place":

.. code-block:: c

    static int a;

    int main(void)
    {
        while (*(volatile int *)&a == 0) { }
    }

    void some_isr(void)
    {
        a++;
    }

Alternatively, you could place a ``barrier()`` call in the loop.

Even better to do the same using ``READ_ONCE()`` / ``WRITE_ONCE()`` macros
(borrowed from kernel), as compiler will order all memory accesses to ``a``
variable (as both accesses are now marked as ``volatile``):

.. code-block:: c

    static int a;

    int main(void)
    {
        while (READ_ONCE(a) == 0) { }
    }

    void some_isr(void)
    {
        WRITE_ONCE(a, a + 1);
    }

.. warning::

    ``READ_ONCE()`` OR ``WRITE_ONCE()`` DO NOT IMPLY A BARRIER!

So if you want to suppress compiler reordering between accesses to ``a`` and
other variables, additional ``barrier()`` call should be used. But be aware of
possible use-case implications! Refer to [2]_ for further details.

If there is a need to do RMW operation (which is not atomic), one should disable
interrupts to achieve the mutual exclusive behavior. ``enter_critical()`` /
``exit_critical()`` API can be used for this. There is an important thing about
calling external functions::

    Compiler is not allowed to reorder code across external function calls.

Mentioned critical section functions are actually macros, but they incorporate
compiler memory barrier code, which makes those macros conform to statement
above. So this takes care of possible optimizations, and ``volatile`` is not
needed in this case. Example:

.. code-block:: c

    static int a;

    int main(void)
    {
        int a_copy;

        do {
            unsigned long flags;

            enter_critical(flags);
            a_copy = a++;
            exit_critical(flags);
        } while (a_copy == 0);
    }

    void some_isr(void)
    {
       a = a + 1;
    }


Using ``volatile`` for accessing the memory-mapped registers
------------------------------------------------------------

This is another actual case where ``volatile`` is needed. When we want to write
some STM32 register, the naiive approach would be to do something like this:

.. code-block:: c

    int main(void)
    {
        unsigned long *reg = (unsigned long *)0x4ae00000;

        while (*reg == 0) { }
    }

Compiler will produce incorrect code, as compiler can't see where 0x4ae00000
changes, so it presumes it doesn't. Correct version of this code would be:

.. code-block:: c

    int main(void)
    {
        volatile unsigned long *reg = (volatile unsigned long *)0x4ae00000;

        while (*reg == 0) { }
    }

Now the correct binary will be produced. And ``volatile`` is probably the only
way to disable compiler's optimization in this case, so we really have to use
it here. But we already have ``MMIO32()`` macro defined in libopencm3 for this:

.. code-block:: c

    #define MMIO32(addr)    (*(volatile uint32_t *)(addr))

and all peripheral registers have corresponding access macros in libopencm3 too.
So again, we don't need to use ``volatile`` in the firmware code for this.


Atomicity and locking; reordering concerns
------------------------------------------

Some people have wrong idea that using ``volatile`` makes the access to the
variable atomic. Of course it's not true. But this is an important concept,
so let's review how to handle it.

Cortex-M3 (ARMv7-M) processor has next features:

  1. Load/store architecture (as it's RISC processor)
  2. Weakly-ordered memory model

Because of (1), variable modification is not atomic. Such operation requires 3
steps: loading from RAM to register, modification, and store from register to
RAM. Corresponding code in C for variable ``int a`` looks like this:

.. code-block:: c

    a++;

And generated assembler code would look something like this:

.. code-block:: none

    ldr r3, [sp, #4]
    add r3, r3, #1
    str r3, [sp, #4]

Compiler can reorder load/store instructions in order to improve code
efficiency, if it doesn't break the logic of code. But because compiler is not
aware of interrupts, it can reorder instructions incorrectly. Example:

.. code-block:: c

    volatile int ready;
    int message[100];

    void foo(int i)
    {
        message[i/10] = 42;
        ready = 1;
    }

    void some_isr(void)
    {
        while (ready != 1);
        // Read message here
    }

Even though ``volatile`` is used, compiler can re-order instructions so that
``ready`` flag is written before ``message``, and if ISR happens in between of
those writes, it'll try to read not written message. Such issue can be solved
using compiler memory barrier, like this:

.. code-block:: c

    volatile int ready;
    int message[100];

    void foo(int i)
    {
        message[i/10] = 42;
        barrier();
        ready = 1;
    }

    void some_isr(void)
    {
        while (ready != 1);
        // Read message here
    }

Of course, critical section (interrupts disabling) can be used too, but it would
be very inefficient comparing to just using ``barrier()``. Please notice that
even if we use ``READ_ONCE()`` / ``WRITE_ONCE()`` API, we still need to use a
barrier here:

.. code-block:: c

    int ready;
    int message[100];

    void foo(int i)
    {
        message[i/10] = 42;
        barrier();
        WRITE_ONCE(ready, 1);
    }

    void some_isr(void)
    {
        while (READ_ONCE(ready) != 1);
        // read message here
    }

Cortex-M doesn't reorder memory instructions internally. From [3]_::

    Implementation: In the Cortex-M processors data transfers are carried out in
    the programmed order.
    ...
    The Cortex-M processors never perform memory accesses out of order compared
    to instruction flow, however, the architecture does not prohibit this in
    future implementations.

So although ARMv7-M has weakly-ordered memory model and allows out-of-order
execution, Cortex-M implementation doesn't reorder instructions. And because
STM32F105 is single-core (Uniprocessor) chip with no cache, we don't need to
worry about processor barriers when working with regular memory.


References
----------

.. [1] https://www.kernel.org/doc/html/latest/process/volatile-considered-harmful.html
.. [2] :doc:`memory-barriers`
.. [3] ARM Application Note 321: "ARM Cortex-M Programming Guide to Memory
       Barrier Instructions"
