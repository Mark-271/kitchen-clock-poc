Memory barriers
===============

(borrowed from Linux kernel memory-barriers.txt)

Compiler barrier
----------------

The Linux kernel has an explicit compiler barrier function that prevents the
compiler from moving the memory accesses either side of it to the other side::

    barrier();

This is a general barrier -- there are no read-read or write-write
variants of ``barrier()``.  However, ``READ_ONCE()`` and ``WRITE_ONCE()`` can be
thought of as weak forms of ``barrier()`` that affect only the specific
accesses flagged by the ``READ_ONCE()`` or ``WRITE_ONCE()``.

The ``barrier()`` function has the following effects:

* Prevents the compiler from reordering accesses following the
  ``barrier()`` to precede any accesses preceding the ``barrier()``.
  One example use for this property is to ease communication between
  interrupt-handler code and the code that was interrupted.
* Within a loop, forces the compiler to load the variables used
  in that loop's conditional on each pass through that loop.

The ``READ_ONCE()`` and ``WRITE_ONCE()`` functions can prevent any number of
optimizations that, while perfectly safe in single-threaded code, can
be fatal in concurrent code.  Here are some examples of these sorts
of optimizations:

* The compiler is within its rights to reorder loads and stores
  to the same variable, and in some cases, the CPU is within its
  rights to reorder loads to the same variable.  This means that
  the following code:

  .. code-block:: c

      a[0] = x;
      a[1] = x;

  Might result in an older value of x stored in a[1] than in a[0].
  Prevent both the compiler and the CPU from doing this as follows:

  .. code-block:: c

      a[0] = READ_ONCE(x);
      a[1] = READ_ONCE(x);

  In short, ``READ_ONCE()`` and ``WRITE_ONCE()`` provide cache coherence for
  accesses from multiple CPUs to a single variable.

* The compiler is within its rights to merge successive loads from
  the same variable.  Such merging can cause the compiler to "optimize"
  the following code:

  .. code-block:: c

      while (tmp = a)
      do_something_with(tmp);

  into the following code, which, although in some sense legitimate
  for single-threaded code, is almost certainly not what the developer
  intended:

  .. code-block:: c

      if (tmp = a)
          for (;;)
              do_something_with(tmp);

  Use ``READ_ONCE()`` to prevent the compiler from doing this to you:

  .. code-block:: c

      while (tmp = READ_ONCE(a))
          do_something_with(tmp);

* The compiler is within its rights to reload a variable, for example,
  in cases where high register pressure prevents the compiler from
  keeping all data of interest in registers.  The compiler might
  therefore optimize the variable ``tmp`` out of our previous example:

  .. code-block:: c

      while (tmp = a)
          do_something_with(tmp);

  This could result in the following code, which is perfectly safe in
  single-threaded code, but can be fatal in concurrent code:

  .. code-block:: c

      while (a)
          do_something_with(a);

  For example, the optimized version of this code could result in
  passing a zero to ``do_something_with()`` in the case where the variable
  a was modified by some other CPU between the ``while`` statement and
  the call to ``do_something_with()``.

  Again, use ``READ_ONCE()`` to prevent the compiler from doing this:

  .. code-block:: c

      while (tmp = READ_ONCE(a))
          do_something_with(tmp);

  Note that if the compiler runs short of registers, it might save
  ``tmp`` onto the stack.  The overhead of this saving and later restoring
  is why compilers reload variables.  Doing so is perfectly safe for
  single-threaded code, so you need to tell the compiler about cases
  where it is not safe.

* The compiler is within its rights to omit a load entirely if it knows
  what the value will be.  For example, if the compiler can prove that
  the value of variable ``a`` is always zero, it can optimize this code:

  .. code-block:: c

      while (tmp = a)
          do_something_with(tmp);

  Into this:

  .. code-block:: c

      do { } while (0);

  This transformation is a win for single-threaded code because it
  gets rid of a load and a branch.  The problem is that the compiler
  will carry out its proof assuming that the current CPU is the only
  one updating variable ``a``.  If variable ``a`` is shared, then the
  compiler's proof will be erroneous.  Use ``READ_ONCE()`` to tell the
  compiler that it doesn't know as much as it thinks it does:

  .. code-block:: c

      while (tmp = READ_ONCE(a))
          do_something_with(tmp);

  But please note that the compiler is also closely watching what you
  do with the value after the ``READ_ONCE()``.  For example, suppose you
  do the following and ``MAX`` is a preprocessor macro with the value 1:

  .. code-block:: c

      while ((tmp = READ_ONCE(a)) % MAX)
          do_something_with(tmp);

  Then the compiler knows that the result of the ``%`` operator applied
  to ``MAX`` will always be zero, again allowing the compiler to optimize
  the code into near-nonexistence.  (It will still load from the
  variable ``a``.)

* Similarly, the compiler is within its rights to omit a store entirely
  if it knows that the variable already has the value being stored.
  Again, the compiler assumes that the current CPU is the only one
  storing into the variable, which can cause the compiler to do the
  wrong thing for shared variables.  For example, suppose you have
  the following:

  .. code-block:: c

      a = 0;
      ... Code that does not store to variable a ...
      a = 0;

  The compiler sees that the value of variable ``a`` is already zero, so
  it might well omit the second store.  This would come as a fatal
  surprise if some other CPU might have stored to variable ``a`` in the
  meantime.

  Use ``WRITE_ONCE()`` to prevent the compiler from making this sort of
  wrong guess:

  .. code-block:: c

      WRITE_ONCE(a, 0);
      ... Code that does not store to variable a ...
      WRITE_ONCE(a, 0);

* The compiler is within its rights to reorder memory accesses unless
  you tell it not to.  For example, consider the following interaction
  between process-level code and an interrupt handler:

  .. code-block:: c

      void process_level(void)
      {
          msg = get_message();
          flag = true;
      }

       void interrupt_handler(void)
       {
           if (flag)
               process_message(msg);
       }

  There is nothing to prevent the compiler from transforming
  ``process_level()`` to the following, in fact, this might well be a
  win for single-threaded code:

  .. code-block:: c

      void process_level(void)
      {
          flag = true;
          msg = get_message();
      }

  If the interrupt occurs between these two statement, then
  ``interrupt_handler()`` might be passed a garbled ``msg``.  Use
  ``WRITE_ONCE()`` to prevent this as follows:

  .. code-block:: c

      void process_level(void)
      {
          WRITE_ONCE(msg, get_message());
          WRITE_ONCE(flag, true);
      }

      void interrupt_handler(void)
      {
          if (READ_ONCE(flag))
              process_message(READ_ONCE(msg));
      }

  Note that the ``READ_ONCE()`` and ``WRITE_ONCE()`` wrappers in
  ``interrupt_handler()`` are needed if this interrupt handler can itself
  be interrupted by something that also accesses ``flag`` and ``msg``,
  for example, a nested interrupt or an NMI.  Otherwise, ``READ_ONCE()``
  and ``WRITE_ONCE()`` are not needed in ``interrupt_handler()`` other than
  for documentation purposes.  (Note also that nested interrupts
  do not typically occur in modern Linux kernels, in fact, if an
  interrupt handler returns with interrupts enabled, you will get a
  ``WARN_ONCE()`` splat.)

  You should assume that the compiler can move ``READ_ONCE()`` and
  ``WRITE_ONCE()`` past code not containing ``READ_ONCE()``, ``WRITE_ONCE()``,
  ``barrier()``, or similar primitives.

  This effect could also be achieved using ``barrier()``, but ``READ_ONCE()``
  and ``WRITE_ONCE()`` are more selective:  With ``READ_ONCE()`` and
  ``WRITE_ONCE()``, the compiler need only forget the contents of the
  indicated memory locations, while with ``barrier()`` the compiler must
  discard the value of all memory locations that it has currented
  cached in any machine registers.  Of course, the compiler must also
  respect the order in which the ``READ_ONCE()``-s and ``WRITE_ONCE()``-s
  occur, though the CPU of course need not do so.

* The compiler is within its rights to invent stores to a variable,
  as in the following example:

  .. code-block:: c

      if (a)
          b = a;
      else
          b = 42;

  The compiler might save a branch by optimizing this as follows:

  .. code-block:: c

      b = 42;
      if (a)
          b = a;

  In single-threaded code, this is not only safe, but also saves
  a branch.  Unfortunately, in concurrent code, this optimization
  could cause some other CPU to see a spurious value of 42 -- even
  if variable ``a`` was never zero -- when loading variable ``b``.
  Use ``WRITE_ONCE()`` to prevent this as follows:

  .. code-block:: c

      if (a)
          WRITE_ONCE(b, a);
      else
          WRITE_ONCE(b, 42);

  The compiler can also invent loads.  These are usually less
  damaging, but they can result in cache-line bouncing and thus in
  poor performance and scalability.  Use ``READ_ONCE()`` to prevent
  invented loads.

* For aligned memory locations whose size allows them to be accessed
  with a single memory-reference instruction, prevents "load tearing"
  and "store tearing", in which a single large access is replaced by
  multiple smaller accesses.  For example, given an architecture having
  16-bit store instructions with 7-bit immediate fields, the compiler
  might be tempted to use two 16-bit store-immediate instructions to
  implement the following 32-bit store:

  .. code-block:: c

      p = 0x00010002;

  Please note that GCC really does use this sort of optimization,
  which is not surprising given that it would likely take more
  than two instructions to build the constant and then store it.
  This optimization can therefore be a win in single-threaded code.
  In fact, a recent bug (since fixed) caused GCC to incorrectly use
  this optimization in a volatile store.  In the absence of such bugs,
  use of ``WRITE_ONCE()`` prevents store tearing in the following example:

  .. code-block:: c

      WRITE_ONCE(p, 0x00010002);

  Use of packed structures can also result in load and store tearing,
  as in this example:

  .. code-block:: c

      struct __attribute__((__packed__)) foo {
          short a;
          int b;
          short c;
      };
      struct foo foo1, foo2;
      ...

      foo2.a = foo1.a;
      foo2.b = foo1.b;
      foo2.c = foo1.c;

  Because there are no ``READ_ONCE()`` or ``WRITE_ONCE()`` wrappers and no
  ``volatile`` markings, the compiler would be well within its rights to
  implement these three assignment statements as a pair of 32-bit
  loads followed by a pair of 32-bit stores.  This would result in
  load tearing on ``foo1.b`` and store tearing on ``foo2.b``.  ``READ_ONCE()``
  and ``WRITE_ONCE()`` again prevent tearing in this example:

  .. code-block:: c

      foo2.a = foo1.a;
      WRITE_ONCE(foo2.b, READ_ONCE(foo1.b));
      foo2.c = foo1.c;

All that aside, it is never necessary to use ``READ_ONCE()`` and
``WRITE_ONCE()`` on a variable that has been marked ``volatile``.  For example,
because ``jiffies`` is marked ``volatile``, it is never necessary to
say ``READ_ONCE(jiffies)``.  The reason for this is that ``READ_ONCE()`` and
``WRITE_ONCE()`` are implemented as ``volatile`` casts, which has no effect when
its argument is already marked ``volatile``.

Please note that these compiler barriers have no direct effect on the CPU,
which may then reorder things however it wishes (but Cortex-M doesn't do
reordering).
