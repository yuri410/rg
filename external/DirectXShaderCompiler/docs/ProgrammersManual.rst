========================
LLVM Programmer's Manual
========================

.. contents::
   :local:

.. warning::
   This is always a work in progress.

.. _introduction:

Introduction
============

This document is meant to highlight some of the important classes and interfaces
available in the LLVM source-base.  This manual is not intended to explain what
LLVM is, how it works, and what LLVM code looks like.  It assumes that you know
the basics of LLVM and are interested in writing transformations or otherwise
analyzing or manipulating the code.

This document should get you oriented so that you can find your way in the
continuously growing source code that makes up the LLVM infrastructure.  Note
that this manual is not intended to serve as a replacement for reading the
source code, so if you think there should be a method in one of these classes to
do something, but it's not listed, check the source.  Links to the `doxygen
<http://llvm.org/doxygen/>`__ sources are provided to make this as easy as
possible.

The first section of this document describes general information that is useful
to know when working in the LLVM infrastructure, and the second describes the
Core LLVM classes.  In the future this manual will be extended with information
describing how to use extension libraries, such as dominator information, CFG
traversal routines, and useful utilities like the ``InstVisitor`` (`doxygen
<http://llvm.org/doxygen/InstVisitor_8h-source.html>`__) template.

.. _general:

General Information
===================

This section contains general information that is useful if you are working in
the LLVM source-base, but that isn't specific to any particular API.

.. _stl:

The C++ Standard Template Library
---------------------------------

LLVM makes heavy use of the C++ Standard Template Library (STL), perhaps much
more than you are used to, or have seen before.  Because of this, you might want
to do a little background reading in the techniques used and capabilities of the
library.  There are many good pages that discuss the STL, and several books on
the subject that you can get, so it will not be discussed in this document.

Here are some useful links:

#. `cppreference.com
   <http://en.cppreference.com/w/>`_ - an excellent
   reference for the STL and other parts of the standard C++ library.

#. `C++ In a Nutshell <http://www.tempest-sw.com/cpp/>`_ - This is an O'Reilly
   book in the making.  It has a decent Standard Library Reference that rivals
   Dinkumware's, and is unfortunately no longer free since the book has been
   published.

#. `C++ Frequently Asked Questions <http://www.parashift.com/c++-faq-lite/>`_.

#. `SGI's STL Programmer's Guide <http://www.sgi.com/tech/stl/>`_ - Contains a
   useful `Introduction to the STL
   <http://www.sgi.com/tech/stl/stl_introduction.html>`_.

#. `Bjarne Stroustrup's C++ Page
   <http://www.research.att.com/%7Ebs/C++.html>`_.

#. `Bruce Eckel's Thinking in C++, 2nd ed. Volume 2 Revision 4.0
   (even better, get the book)
   <http://www.mindview.net/Books/TICPP/ThinkingInCPP2e.html>`_.

You are also encouraged to take a look at the :doc:`LLVM Coding Standards
<CodingStandards>` guide which focuses on how to write maintainable code more
than where to put your curly braces.

.. _resources:

Other useful references
-----------------------

#. `Using static and shared libraries across platforms
   <http://www.fortran-2000.com/ArnaudRecipes/sharedlib.html>`_

.. _apis:

Important and useful LLVM APIs
==============================

Here we highlight some LLVM APIs that are generally useful and good to know
about when writing transformations.

.. _isa:

The ``isa<>``, ``cast<>`` and ``dyn_cast<>`` templates
------------------------------------------------------

The LLVM source-base makes extensive use of a custom form of RTTI.  These
templates have many similarities to the C++ ``dynamic_cast<>`` operator, but
they don't have some drawbacks (primarily stemming from the fact that
``dynamic_cast<>`` only works on classes that have a v-table).  Because they are
used so often, you must know what they do and how they work.  All of these
templates are defined in the ``llvm/Support/Casting.h`` (`doxygen
<http://llvm.org/doxygen/Casting_8h-source.html>`__) file (note that you very
rarely have to include this file directly).

``isa<>``:
  The ``isa<>`` operator works exactly like the Java "``instanceof``" operator.
  It returns true or false depending on whether a reference or pointer points to
  an instance of the specified class.  This can be very useful for constraint
  checking of various sorts (example below).

``cast<>``:
  The ``cast<>`` operator is a "checked cast" operation.  It converts a pointer
  or reference from a base class to a derived class, causing an assertion
  failure if it is not really an instance of the right type.  This should be
  used in cases where you have some information that makes you believe that
  something is of the right type.  An example of the ``isa<>`` and ``cast<>``
  template is:

  .. code-block:: c++

    static bool isLoopInvariant(const Value *V, const Loop *L) {
      if (isa<Constant>(V) || isa<Argument>(V) || isa<GlobalValue>(V))
        return true;

      // Otherwise, it must be an instruction...
      return !L->contains(cast<Instruction>(V)->getParent());
    }

  Note that you should **not** use an ``isa<>`` test followed by a ``cast<>``,
  for that use the ``dyn_cast<>`` operator.

``dyn_cast<>``:
  The ``dyn_cast<>`` operator is a "checking cast" operation.  It checks to see
  if the operand is of the specified type, and if so, returns a pointer to it
  (this operator does not work with references).  If the operand is not of the
  correct type, a null pointer is returned.  Thus, this works very much like
  the ``dynamic_cast<>`` operator in C++, and should be used in the same
  circumstances.  Typically, the ``dyn_cast<>`` operator is used in an ``if``
  statement or some other flow control statement like this:

  .. code-block:: c++

    if (AllocationInst *AI = dyn_cast<AllocationInst>(Val)) {
      // ...
    }

  This form of the ``if`` statement effectively combines together a call to
  ``isa<>`` and a call to ``cast<>`` into one statement, which is very
  convenient.

  Note that the ``dyn_cast<>`` operator, like C++'s ``dynamic_cast<>`` or Java's
  ``instanceof`` operator, can be abused.  In particular, you should not use big
  chained ``if/then/else`` blocks to check for lots of different variants of
  classes.  If you find yourself wanting to do this, it is much cleaner and more
  efficient to use the ``InstVisitor`` class to dispatch over the instruction
  type directly.

``cast_or_null<>``:
  The ``cast_or_null<>`` operator works just like the ``cast<>`` operator,
  except that it allows for a null pointer as an argument (which it then
  propagates).  This can sometimes be useful, allowing you to combine several
  null checks into one.

``dyn_cast_or_null<>``:
  The ``dyn_cast_or_null<>`` operator works just like the ``dyn_cast<>``
  operator, except that it allows for a null pointer as an argument (which it
  then propagates).  This can sometimes be useful, allowing you to combine
  several null checks into one.

These five templates can be used with any classes, whether they have a v-table
or not.  If you want to add support for these templates, see the document
:doc:`How to set up LLVM-style RTTI for your class hierarchy
<HowToSetUpLLVMStyleRTTI>`

.. _string_apis:

Passing strings (the ``StringRef`` and ``Twine`` classes)
---------------------------------------------------------

Although LLVM generally does not do much string manipulation, we do have several
important APIs which take strings.  Two important examples are the Value class
-- which has names for instructions, functions, etc. -- and the ``StringMap``
class which is used extensively in LLVM and Clang.

These are generic classes, and they need to be able to accept strings which may
have embedded null characters.  Therefore, they cannot simply take a ``const
char *``, and taking a ``const std::string&`` requires clients to perform a heap
allocation which is usually unnecessary.  Instead, many LLVM APIs use a
``StringRef`` or a ``const Twine&`` for passing strings efficiently.

.. _StringRef:

The ``StringRef`` class
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The ``StringRef`` data type represents a reference to a constant string (a
character array and a length) and supports the common operations available on
``std::string``, but does not require heap allocation.

It can be implicitly constructed using a C style null-terminated string, an
``std::string``, or explicitly with a character pointer and length.  For
example, the ``StringRef`` find function is declared as:

.. code-block:: c++

  iterator find(StringRef Key);

and clients can call it using any one of:

.. code-block:: c++

  Map.find("foo");                 // Lookup "foo"
  Map.find(std::string("bar"));    // Lookup "bar"
  Map.find(StringRef("\0baz", 4)); // Lookup "\0baz"

Similarly, APIs which need to return a string may return a ``StringRef``
instance, which can be used directly or converted to an ``std::string`` using
the ``str`` member function.  See ``llvm/ADT/StringRef.h`` (`doxygen
<http://llvm.org/doxygen/classllvm_1_1StringRef_8h-source.html>`__) for more
information.

You should rarely use the ``StringRef`` class directly, because it contains
pointers to external memory it is not generally safe to store an instance of the
class (unless you know that the external storage will not be freed).
``StringRef`` is small and pervasive enough in LLVM that it should always be
passed by value.

The ``Twine`` class
^^^^^^^^^^^^^^^^^^^

The ``Twine`` (`doxygen <http://llvm.org/doxygen/classllvm_1_1Twine.html>`__)
class is an efficient way for APIs to accept concatenated strings.  For example,
a common LLVM paradigm is to name one instruction based on the name of another
instruction with a suffix, for example:

.. code-block:: c++

    New = CmpInst::Create(..., SO->getName() + ".cmp");

The ``Twine`` class is effectively a lightweight `rope
<http://en.wikipedia.org/wiki/Rope_(computer_science)>`_ which points to
temporary (stack allocated) objects.  Twines can be implicitly constructed as
the result of the plus operator applied to strings (i.e., a C strings, an
``std::string``, or a ``StringRef``).  The twine delays the actual concatenation
of strings until it is actually required, at which point it can be efficiently
rendered directly into a character array.  This avoids unnecessary heap
allocation involved in constructing the temporary results of string
concatenation.  See ``llvm/ADT/Twine.h`` (`doxygen
<http://llvm.org/doxygen/Twine_8h_source.html>`__) and :ref:`here <dss_twine>`
for more information.

As with a ``StringRef``, ``Twine`` objects point to external memory and should
almost never be stored or mentioned directly.  They are intended solely for use
when defining a function which should be able to efficiently accept concatenated
strings.

.. _function_apis:

Passing functions and other callable objects
--------------------------------------------

Sometimes you may want a function to be passed a callback object. In order to
support lambda expressions and other function objects, you should not use the
traditional C approach of taking a function pointer and an opaque cookie:

.. code-block:: c++

    void takeCallback(bool (*Callback)(Function *, void *), void *Cookie);

Instead, use one of the following approaches:

Function template
^^^^^^^^^^^^^^^^^

If you don't mind putting the definition of your function into a header file,
make it a function template that is templated on the callable type.

.. code-block:: c++

    template<typename Callable>
    void takeCallback(Callable Callback) {
      Callback(1, 2, 3);
    }

The ``function_ref`` class template
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The ``function_ref``
(`doxygen <http://llvm.org/doxygen/classllvm_1_1function_ref.html>`__) class
template represents a reference to a callable object, templated over the type
of the callable. This is a good choice for passing a callback to a function,
if you don't need to hold onto the callback after the function returns. In this
way, ``function_ref`` is to ``std::function`` as ``StringRef`` is to
``std::string``.

``function_ref<Ret(Param1, Param2, ...)>`` can be implicitly constructed from
any callable object that can be called with arguments of type ``Param1``,
``Param2``, ..., and returns a value that can be converted to type ``Ret``.
For example:

.. code-block:: c++

    void visitBasicBlocks(Function *F, function_ref<bool (BasicBlock*)> Callback) {
      for (BasicBlock &BB : *F)
        if (Callback(&BB))
          return;
    }

can be called using:

.. code-block:: c++

    visitBasicBlocks(F, [&](BasicBlock *BB) {
      if (process(BB))
        return isEmpty(BB);
      return false;
    });

Note that a ``function_ref`` object contains pointers to external memory, so it
is not generally safe to store an instance of the class (unless you know that
the external storage will not be freed). If you need this ability, consider
using ``std::function``. ``function_ref`` is small enough that it should always
be passed by value.

.. _DEBUG:

The ``DEBUG()`` macro and ``-debug`` option
-------------------------------------------

Often when working on your pass you will put a bunch of debugging printouts and
other code into your pass.  After you get it working, you want to remove it, but
you may need it again in the future (to work out new bugs that you run across).

Naturally, because of this, you don't want to delete the debug printouts, but
you don't want them to always be noisy.  A standard compromise is to comment
them out, allowing you to enable them if you need them in the future.

The ``llvm/Support/Debug.h`` (`doxygen
<http://llvm.org/doxygen/Debug_8h-source.html>`__) file provides a macro named
``DEBUG()`` that is a much nicer solution to this problem.  Basically, you can
put arbitrary code into the argument of the ``DEBUG`` macro, and it is only
executed if '``opt``' (or any other tool) is run with the '``-debug``' command
line argument:

.. code-block:: c++

  DEBUG(errs() << "I am here!\n");

Then you can run your pass like this:

.. code-block:: none

  $ opt < a.bc > /dev/null -mypass
  <no output>
  $ opt < a.bc > /dev/null -mypass -debug
  I am here!

Using the ``DEBUG()`` macro instead of a home-brewed solution allows you to not
have to create "yet another" command line option for the debug output for your
pass.  Note that ``DEBUG()`` macros are disabled for optimized builds, so they
do not cause a performance impact at all (for the same reason, they should also
not contain side-effects!).

One additional nice thing about the ``DEBUG()`` macro is that you can enable or
disable it directly in gdb.  Just use "``set DebugFlag=0``" or "``set
DebugFlag=1``" from the gdb if the program is running.  If the program hasn't
been started yet, you can always just run it with ``-debug``.

.. _DEBUG_TYPE:

Fine grained debug info with ``DEBUG_TYPE`` and the ``-debug-only`` option
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Sometimes you may find yourself in a situation where enabling ``-debug`` just
turns on **too much** information (such as when working on the code generator).
If you want to enable debug information with more fine-grained control, you
can define the ``DEBUG_TYPE`` macro and use the ``-debug-only`` option as
follows:

.. code-block:: c++

  #undef  DEBUG_TYPE
  DEBUG(errs() << "No debug type\n");
  #define DEBUG_TYPE "foo"
  DEBUG(errs() << "'foo' debug type\n");
  #undef  DEBUG_TYPE
  #define DEBUG_TYPE "bar"
  DEBUG(errs() << "'bar' debug type\n"));
  #undef  DEBUG_TYPE
  #define DEBUG_TYPE ""
  DEBUG(errs() << "No debug type (2)\n");

Then you can run your pass like this:

.. code-block:: none

  $ opt < a.bc > /dev/null -mypass
  <no output>
  $ opt < a.bc > /dev/null -mypass -debug
  No debug type
  'foo' debug type
  'bar' debug type
  No debug type (2)
  $ opt < a.bc > /dev/null -mypass -debug-only=foo
  'foo' debug type
  $ opt < a.bc > /dev/null -mypass -debug-only=bar
  'bar' debug type

Of course, in practice, you should only set ``DEBUG_TYPE`` at the top of a file,
to specify the debug type for the entire module (if you do this before you
``#include "llvm/Support/Debug.h"``, you don't have to insert the ugly
``#undef``'s).  Also, you should use names more meaningful than "foo" and "bar",
because there is no system in place to ensure that names do not conflict.  If
two different modules use the same string, they will all be turned on when the
name is specified.  This allows, for example, all debug information for
instruction scheduling to be enabled with ``-debug-only=InstrSched``, even if
the source lives in multiple files.

For performance reasons, -debug-only is not available in optimized build
(``--enable-optimized``) of LLVM.

The ``DEBUG_WITH_TYPE`` macro is also available for situations where you would
like to set ``DEBUG_TYPE``, but only for one specific ``DEBUG`` statement.  It
takes an additional first parameter, which is the type to use.  For example, the
preceding example could be written as:

.. code-block:: c++

  DEBUG_WITH_TYPE("", errs() << "No debug type\n");
  DEBUG_WITH_TYPE("foo", errs() << "'foo' debug type\n");
  DEBUG_WITH_TYPE("bar", errs() << "'bar' debug type\n"));
  DEBUG_WITH_TYPE("", errs() << "No debug type (2)\n");

.. _Statistic:

The ``Statistic`` class & ``-stats`` option
-------------------------------------------

The ``llvm/ADT/Statistic.h`` (`doxygen
<http://llvm.org/doxygen/Statistic_8h-source.html>`__) file provides a class
named ``Statistic`` that is used as a unified way to keep track of what the LLVM
compiler is doing and how effective various optimizations are.  It is useful to
see what optimizations are contributing to making a particular program run
faster.

Often you may run your pass on some big program, and you're interested to see
how many times it makes a certain transformation.  Although you can do this with
hand inspection, or some ad-hoc method, this is a real pain and not very useful
for big programs.  Using the ``Statistic`` class makes it very easy to keep
track of this information, and the calculated information is presented in a
uniform manner with the rest of the passes being executed.

There are many examples of ``Statistic`` uses, but the basics of using it are as
follows:

#. Define your statistic like this:

  .. code-block:: c++

    #define DEBUG_TYPE "mypassname"   // This goes before any #includes.
    STATISTIC(NumXForms, "The # of times I did stuff");

  The ``STATISTIC`` macro defines a static variable, whose name is specified by
  the first argument.  The pass name is taken from the ``DEBUG_TYPE`` macro, and
  the description is taken from the second argument.  The variable defined
  ("NumXForms" in this case) acts like an unsigned integer.

#. Whenever you make a transformation, bump the counter:

  .. code-block:: c++

    ++NumXForms;   // I did stuff!

That's all you have to do.  To get '``opt``' to print out the statistics
gathered, use the '``-stats``' option:

.. code-block:: none

  $ opt -stats -mypassname < program.bc > /dev/null
  ... statistics output ...

Note that in order to use the '``-stats``' option, LLVM must be
compiled with assertions enabled.

When running ``opt`` on a C file from the SPEC benchmark suite, it gives a
report that looks like this:

.. code-block:: none

   7646 bitcodewriter   - Number of normal instructions
    725 bitcodewriter   - Number of oversized instructions
 129996 bitcodewriter   - Number of bitcode bytes written
   2817 raise           - Number of insts DCEd or constprop'd
   3213 raise           - Number of cast-of-self removed
   5046 raise           - Number of expression trees converted
     75 raise           - Number of other getelementptr's formed
    138 raise           - Number of load/store peepholes
     42 deadtypeelim    - Number of unused typenames removed from symtab
    392 funcresolve     - Number of varargs functions resolved
     27 globaldce       - Number of global variables removed
      2 adce            - Number of basic blocks removed
    134 cee             - Number of branches revectored
     49 cee             - Number of setcc instruction eliminated
    532 gcse            - Number of loads removed
   2919 gcse            - Number of instructions removed
     86 indvars         - Number of canonical indvars added
     87 indvars         - Number of aux indvars removed
     25 instcombine     - Number of dead inst eliminate
    434 instcombine     - Number of insts combined
    248 licm            - Number of load insts hoisted
   1298 licm            - Number of insts hoisted to a loop pre-header
      3 licm            - Number of insts hoisted to multiple loop preds (bad, no loop pre-header)
     75 mem2reg         - Number of alloca's promoted
   1444 cfgsimplify     - Number of blocks simplified

Obviously, with so many optimizations, having a unified framework for this stuff
is very nice.  Making your pass fit well into the framework makes it more
maintainable and useful.

.. _ViewGraph:

Viewing graphs while debugging code
-----------------------------------

Several of the important data structures in LLVM are graphs: for example CFGs
made out of LLVM :ref:`BasicBlocks <BasicBlock>`, CFGs made out of LLVM
:ref:`MachineBasicBlocks <MachineBasicBlock>`, and :ref:`Instruction Selection
DAGs <SelectionDAG>`.  In many cases, while debugging various parts of the
compiler, it is nice to instantly visualize these graphs.

LLVM provides several callbacks that are available in a debug build to do
exactly that.  If you call the ``Function::viewCFG()`` method, for example, the
current LLVM tool will pop up a window containing the CFG for the function where
each basic block is a node in the graph, and each node contains the instructions
in the block.  Similarly, there also exists ``Function::viewCFGOnly()`` (does
not include the instructions), the ``MachineFunction::viewCFG()`` and
``MachineFunction::viewCFGOnly()``, and the ``SelectionDAG::viewGraph()``
methods.  Within GDB, for example, you can usually use something like ``call
DAG.viewGraph()`` to pop up a window.  Alternatively, you can sprinkle calls to
these functions in your code in places you want to debug.

Getting this to work requires a small amount of setup.  On Unix systems
with X11, install the `graphviz <http://www.graphviz.org>`_ toolkit, and make
sure 'dot' and 'gv' are in your path.  If you are running on Mac OS X, download
and install the Mac OS X `Graphviz program
<http://www.pixelglow.com/graphviz/>`_ and add
``/Applications/Graphviz.app/Contents/MacOS/`` (or wherever you install it) to
your path. The programs need not be present when configuring, building or
running LLVM and can simply be installed when needed during an active debug
session.

``SelectionDAG`` has been extended to make it easier to locate *interesting*
nodes in large complex graphs.  From gdb, if you ``call DAG.setGraphColor(node,
"color")``, then the next ``call DAG.viewGraph()`` would highlight the node in
the specified color (choices of colors can be found at `colors
<http://www.graphviz.org/doc/info/colors.html>`_.) More complex node attributes
can be provided with ``call DAG.setGraphAttrs(node, "attributes")`` (choices can
be found at `Graph attributes <http://www.graphviz.org/doc/info/attrs.html>`_.)
If you want to restart and clear all the current graph attributes, then you can
``call DAG.clearGraphAttrs()``.

Note that graph visualization features are compiled out of Release builds to
reduce file size.  This means that you need a Debug+Asserts or Release+Asserts
build to use these features.

.. _datastructure:

Picking the Right Data Structure for a Task
===========================================

LLVM has a plethora of data structures in the ``llvm/ADT/`` directory, and we
commonly use STL data structures.  This section describes the trade-offs you
should consider when you pick one.

The first step is a choose your own adventure: do you want a sequential
container, a set-like container, or a map-like container?  The most important
thing when choosing a container is the algorithmic properties of how you plan to
access the container.  Based on that, you should use:


* a :ref:`map-like <ds_map>` container if you need efficient look-up of a
  value based on another value.  Map-like containers also support efficient
  queries for containment (whether a key is in the map).  Map-like containers
  generally do not support efficient reverse mapping (values to keys).  If you
  need that, use two maps.  Some map-like containers also support efficient
  iteration through the keys in sorted order.  Map-like containers are the most
  expensive sort, only use them if you need one of these capabilities.

* a :ref:`set-like <ds_set>` container if you need to put a bunch of stuff into
  a container that automatically eliminates duplicates.  Some set-like
  containers support efficient iteration through the elements in sorted order.
  Set-like containers are more expensive than sequential containers.

* a :ref:`sequential <ds_sequential>` container provides the most efficient way
  to add elements and keeps track of the order they are added to the collection.
  They permit duplicates and support efficient iteration, but do not support
  efficient look-up based on a key.

* a :ref:`string <ds_string>` container is a specialized sequential container or
  reference structure that is used for character or byte arrays.

* a :ref:`bit <ds_bit>` container provides an efficient way to store and
  perform set operations on sets of numeric id's, while automatically
  eliminating duplicates.  Bit containers require a maximum of 1 bit for each
  identifier you want to store.

Once the proper category of container is determined, you can fine tune the
memory use, constant factors, and cache behaviors of access by intelligently
picking a member of the category.  Note that constant factors and cache behavior
can be a big deal.  If you have a vector that usually only contains a few
elements (but could contain many), for example, it's much better to use
:ref:`SmallVector <dss_smallvector>` than :ref:`vector <dss_vector>`.  Doing so
avoids (relatively) expensive malloc/free calls, which dwarf the cost of adding
the elements to the container.

.. _ds_sequential:

Sequential Containers (std::vector, std::list, etc)
---------------------------------------------------

There are a variety of sequential containers available for you, based on your
needs.  Pick the first in this section that will do what you want.

.. _dss_arrayref:

llvm/ADT/ArrayRef.h
^^^^^^^^^^^^^^^^^^^

The ``llvm::ArrayRef`` class is the preferred class to use in an interface that
accepts a sequential list of elements in memory and just reads from them.  By
taking an ``ArrayRef``, the API can be passed a fixed size array, an
``std::vector``, an ``llvm::SmallVector`` and anything else that is contiguous
in memory.

.. _dss_fixedarrays:

Fixed Size Arrays
^^^^^^^^^^^^^^^^^

Fixed size arrays are very simple and very fast.  They are good if you know
exactly how many elements you have, or you have a (low) upper bound on how many
you have.

.. _dss_heaparrays:

Heap Allocated Arrays
^^^^^^^^^^^^^^^^^^^^^

Heap allocated arrays (``new[]`` + ``delete[]``) are also simple.  They are good
if the number of elements is variable, if you know how many elements you will
need before the array is allocated, and if the array is usually large (if not,
consider a :ref:`SmallVector <dss_smallvector>`).  The cost of a heap allocated
array is the cost of the new/delete (aka malloc/free).  Also note that if you
are allocating an array of a type with a constructor, the constructor and
destructors will be run for every element in the array (re-sizable vectors only
construct those elements actually used).

.. _dss_tinyptrvector:

llvm/ADT/TinyPtrVector.h
^^^^^^^^^^^^^^^^^^^^^^^^

``TinyPtrVector<Type>`` is a highly specialized collection class that is
optimized to avoid allocation in the case when a vector has zero or one
elements.  It has two major restrictions: 1) it can only hold values of pointer
type, and 2) it cannot hold a null pointer.

Since this container is highly specialized, it is rarely used.

.. _dss_smallvector:

llvm/ADT/SmallVector.h
^^^^^^^^^^^^^^^^^^^^^^

``SmallVector<Type, N>`` is a simple class that looks and smells just like
``vector<Type>``: it supports efficient iteration, lays out elements in memory
order (so you can do pointer arithmetic between elements), supports efficient
push_back/pop_back operations, supports efficient random access to its elements,
etc.

The advantage of SmallVector is that it allocates space for some number of
elements (N) **in the object itself**.  Because of this, if the SmallVector is
dynamically smaller than N, no malloc is performed.  This can be a big win in
cases where the malloc/free call is far more expensive than the code that
fiddles around with the elements.

This is good for vectors that are "usually small" (e.g. the number of
predecessors/successors of a block is usually less than 8).  On the other hand,
this makes the size of the SmallVector itself large, so you don't want to
allocate lots of them (doing so will waste a lot of space).  As such,
SmallVectors are most useful when on the stack.

SmallVector also provides a nice portable and efficient replacement for
``alloca``.

.. note::

   Prefer to use ``SmallVectorImpl<T>`` as a parameter type.

   In APIs that don't care about the "small size" (most?), prefer to use
   the ``SmallVectorImpl<T>`` class, which is basically just the "vector
   header" (and methods) without the elements allocated after it. Note that
   ``SmallVector<T, N>`` inherits from ``SmallVectorImpl<T>`` so the
   conversion is implicit and costs nothing. E.g.

   .. code-block:: c++

      // BAD: Clients cannot pass e.g. SmallVector<Foo, 4>.
      hardcodedSmallSize(SmallVector<Foo, 2> &Out);
      // GOOD: Clients can pass any SmallVector<Foo, N>.
      allowsAnySmallSize(SmallVectorImpl<Foo> &Out);

      void someFunc() {
        SmallVector<Foo, 8> Vec;
        hardcodedSmallSize(Vec); // Error.
        allowsAnySmallSize(Vec); // Works.
      }

   Even though it has "``Impl``" in the name, this is so widely used that
   it really isn't "private to the implementation" anymore. A name like
   ``SmallVectorHeader`` would be more appropriate.

.. _dss_vector:

<vector>
^^^^^^^^

``std::vector`` is well loved and respected.  It is useful when SmallVector
isn't: when the size of the vector is often large (thus the small optimization
will rarely be a benefit) or if you will be allocating many instances of the
vector itself (which would waste space for elements that aren't in the
container).  vector is also useful when interfacing with code that expects
vectors :).

One worthwhile note about std::vector: avoid code like this:

.. code-block:: c++

  for ( ... ) {
     std::vector<foo> V;
     // make use of V.
  }

Instead, write this as:

.. code-block:: c++

  std::vector<foo> V;
  for ( ... ) {
     // make use of V.
     V.clear();
  }

Doing so will save (at least) one heap allocation and free per iteration of the
loop.

.. _dss_deque:

<deque>
^^^^^^^

``std::deque`` is, in some senses, a generalized version of ``std::vector``.
Like ``std::vector``, it provides constant time random access and other similar
properties, but it also provides efficient access to the front of the list.  It
does not guarantee continuity of elements within memory.

In exchange for this extra flexibility, ``std::deque`` has significantly higher
constant factor costs than ``std::vector``.  If possible, use ``std::vector`` or
something cheaper.

.. _dss_list:

<list>
^^^^^^

``std::list`` is an extremely inefficient class that is rarely useful.  It
performs a heap allocation for every element inserted into it, thus having an
extremely high constant factor, particularly for small data types.
``std::list`` also only supports bidirectional iteration, not random access
iteration.

In exchange for this high cost, std::list supports efficient access to both ends
of the list (like ``std::deque``, but unlike ``std::vector`` or
``SmallVector``).  In addition, the iterator invalidation characteristics of
std::list are stronger than that of a vector class: inserting or removing an
element into the list does not invalidate iterator or pointers to other elements
in the list.

.. _dss_ilist:

llvm/ADT/ilist.h
^^^^^^^^^^^^^^^^

``ilist<T>`` implements an 'intrusive' doubly-linked list.  It is intrusive,
because it requires the element to store and provide access to the prev/next
pointers for the list.

``ilist`` has the same drawbacks as ``std::list``, and additionally requires an
``ilist_traits`` implementation for the element type, but it provides some novel
characteristics.  In particular, it can efficiently store polymorphic objects,
the traits class is informed when an element is inserted or removed from the
list, and ``ilist``\ s are guaranteed to support a constant-time splice
operation.

These properties are exactly what we want for things like ``Instruction``\ s and
basic blocks, which is why these are implemented with ``ilist``\ s.

Related classes of interest are explained in the following subsections:

* :ref:`ilist_traits <dss_ilist_traits>`

* :ref:`iplist <dss_iplist>`

* :ref:`llvm/ADT/ilist_node.h <dss_ilist_node>`

* :ref:`Sentinels <dss_ilist_sentinel>`

.. _dss_packedvector:

llvm/ADT/PackedVector.h
^^^^^^^^^^^^^^^^^^^^^^^

Useful for storing a vector of values using only a few number of bits for each
value.  Apart from the standard operations of a vector-like container, it can
also perform an 'or' set operation.

For example:

.. code-block:: c++

  enum State {
      None = 0x0,
      FirstCondition = 0x1,
      SecondCondition = 0x2,
      Both = 0x3
  };

  State get() {
      PackedVector<State, 2> Vec1;
      Vec1.push_back(FirstCondition);

      PackedVector<State, 2> Vec2;
      Vec2.push_back(SecondCondition);

      Vec1 |= Vec2;
      return Vec1[0]; // returns 'Both'.
  }

.. _dss_ilist_traits:

ilist_traits
^^^^^^^^^^^^

``ilist_traits<T>`` is ``ilist<T>``'s customization mechanism. ``iplist<T>``
(and consequently ``ilist<T>``) publicly derive from this traits class.

.. _dss_iplist:

iplist
^^^^^^

``iplist<T>`` is ``ilist<T>``'s base and as such supports a slightly narrower
interface.  Notably, inserters from ``T&`` are absent.

``ilist_traits<T>`` is a public base of this class and can be used for a wide
variety of customizations.

.. _dss_ilist_node:

llvm/ADT/ilist_node.h
^^^^^^^^^^^^^^^^^^^^^

``ilist_node<T>`` implements the forward and backward links that are expected
by the ``ilist<T>`` (and analogous containers) in the default manner.

``ilist_node<T>``\ s are meant to be embedded in the node type ``T``, usually
``T`` publicly derives from ``ilist_node<T>``.

.. _dss_ilist_sentinel:

Sentinels
^^^^^^^^^

``ilist``\ s have another specialty that must be considered.  To be a good
citizen in the C++ ecosystem, it needs to support the standard container
operations, such as ``begin`` and ``end`` iterators, etc.  Also, the
``operator--`` must work correctly on the ``end`` iterator in the case of
non-empty ``ilist``\ s.

The only sensible solution to this problem is to allocate a so-called *sentinel*
along with the intrusive list, which serves as the ``end`` iterator, providing
the back-link to the last element.  However conforming to the C++ convention it
is illegal to ``operator++`` beyond the sentinel and it also must not be
dereferenced.

These constraints allow for some implementation freedom to the ``ilist`` how to
allocate and store the sentinel.  The corresponding policy is dictated by
``ilist_traits<T>``.  By default a ``T`` gets heap-allocated whenever the need
for a sentinel arises.

While the default policy is sufficient in most cases, it may break down when
``T`` does not provide a default constructor.  Also, in the case of many
instances of ``ilist``\ s, the memory overhead of the associated sentinels is
wasted.  To alleviate the situation with numerous and voluminous
``T``-sentinels, sometimes a trick is employed, leading to *ghostly sentinels*.

Ghostly sentinels are obtained by specially-crafted ``ilist_traits<T>`` which
superpose the sentinel with the ``ilist`` instance in memory.  Pointer
arithmetic is used to obtain the sentinel, which is relative to the ``ilist``'s
``this`` pointer.  The ``ilist`` is augmented by an extra pointer, which serves
as the back-link of the sentinel.  This is the only field in the ghostly
sentinel which can be legally accessed.

.. _dss_other:

Other Sequential Container options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Other STL containers are available, such as ``std::string``.

There are also various STL adapter classes such as ``std::queue``,
``std::priority_queue``, ``std::stack``, etc.  These provide simplified access
to an underlying container but don't affect the cost of the container itself.

.. _ds_string:

String-like containers
----------------------

There are a variety of ways to pass around and use strings in C and C++, and
LLVM adds a few new options to choose from.  Pick the first option on this list
that will do what you need, they are ordered according to their relative cost.

Note that it is generally preferred to *not* pass strings around as ``const
char*``'s.  These have a number of problems, including the fact that they
cannot represent embedded nul ("\0") characters, and do not have a length
available efficiently.  The general replacement for '``const char*``' is
StringRef.

For more information on choosing string containers for APIs, please see
:ref:`Passing Strings <string_apis>`.

.. _dss_stringref:

llvm/ADT/StringRef.h
^^^^^^^^^^^^^^^^^^^^

The StringRef class is a simple value class that contains a pointer to a
character and a length, and is quite related to the :ref:`ArrayRef
<dss_arrayref>` class (but specialized for arrays of characters).  Because
StringRef carries a length with it, it safely handles strings with embedded nul
characters in it, getting the length does not require a strlen call, and it even
has very convenient APIs for slicing and dicing the character range that it
represents.

StringRef is ideal for passing simple strings around that are known to be live,
either because they are C string literals, std::string, a C array, or a
SmallVector.  Each of these cases has an efficient implicit conversion to
StringRef, which doesn't result in a dynamic strlen being executed.

StringRef has a few major limitations which make more powerful string containers
useful:

#. You cannot directly convert a StringRef to a 'const char*' because there is
   no way to add a trailing nul (unlike the .c_str() method on various stronger
   classes).

#. StringRef doesn't own or keep alive the underlying string bytes.
   As such it can easily lead to dangling pointers, and is not suitable for
   embedding in datastructures in most cases (instead, use an std::string or
   something like that).

#. For the same reason, StringRef cannot be used as the return value of a
   method if the method "computes" the result string.  Instead, use std::string.

#. StringRef's do not allow you to mutate the pointed-to string bytes and it
   doesn't allow you to insert or remove bytes from the range.  For editing
   operations like this, it interoperates with the :ref:`Twine <dss_twine>`
   class.

Because of its strengths and limitations, it is very common for a function to
take a StringRef and for a method on an object to return a StringRef that points
into some string that it owns.

.. _dss_twine:

llvm/ADT/Twine.h
^^^^^^^^^^^^^^^^

The Twine class is used as an intermediary datatype for APIs that want to take a
string that can be constructed inline with a series of concatenations.  Twine
works by forming recursive instances of the Twine datatype (a simple value
object) on the stack as temporary objects, linking them together into a tree
which is then linearized when the Twine is consumed.  Twine is only safe to use
as the argument to a function, and should always be a const reference, e.g.:

.. code-block:: c++

  void foo(const Twine &T);
  ...
  StringRef X = ...
  unsigned i = ...
  foo(X + "." + Twine(i));

This example forms a string like "blarg.42" by concatenating the values
together, and does not form intermediate strings containing "blarg" or "blarg.".

Because Twine is constructed with temporary objects on the stack, and because
these instances are destroyed at the end of the current statement, it is an
inherently dangerous API.  For example, this simple variant contains undefined
behavior and will probably crash:

.. code-block:: c++

  void foo(const Twine &T);
  ...
  StringRef X = ...
  unsigned i = ...
  const Twine &Tmp = X + "." + Twine(i);
  foo(Tmp);

... because the temporaries are destroyed before the call.  That said, Twine's
are much more efficient than intermediate std::string temporaries, and they work
really well with StringRef.  Just be aware of their limitations.

.. _dss_smallstring:

llvm/ADT/SmallString.h
^^^^^^^^^^^^^^^^^^^^^^

SmallString is a subclass of :ref:`SmallVector <dss_smallvector>` that adds some
convenience APIs like += that takes StringRef's.  SmallString avoids allocating
memory in the case when the preallocated space is enough to hold its data, and
it calls back to general heap allocation when required.  Since it owns its data,
it is very safe to use and supports full mutation of the string.

Like SmallVector's, the big downside to SmallString is their sizeof.  While they
are optimized for small strings, they themselves are not particularly small.
This means that they work great for temporary scratch buffers on the stack, but
should not generally be put into the heap: it is very rare to see a SmallString
as the member of a frequently-allocated heap data structure or returned
by-value.

.. _dss_stdstring:

std::string
^^^^^^^^^^^

The standard C++ std::string class is a very general class that (like
SmallString) owns its underlying data.  sizeof(std::string) is very reasonable
so it can be embedded into heap data structures and returned by-value.  On the
other hand, std::string is highly inefficient for inline editing (e.g.
concatenating a bunch of stuff together) and because it is provided by the
standard library, its performance characteristics depend a lot of the host
standard library (e.g. libc++ and MSVC provide a highly optimized string class,
GCC contains a really slow implementation).

The major disadvantage of std::string is that almost every operation that makes
them larger can allocate memory, which is slow.  As such, it is better to use
SmallVector or Twine as a scratch buffer, but then use std::string to persist
the result.

.. _ds_set:

Set-Like Containers (std::set, SmallSet, SetVector, etc)
--------------------------------------------------------

Set-like containers are useful when you need to canonicalize multiple values
into a single representation.  There are several different choices for how to do
this, providing various trade-offs.

.. _dss_sortedvectorset:

A sorted 'vector'
^^^^^^^^^^^^^^^^^

If you intend to insert a lot of elements, then do a lot of queries, a great
approach is to use a vector (or other sequential container) with
std::sort+std::unique to remove duplicates.  This approach works really well if
your usage pattern has these two distinct phases (insert then query), and can be
coupled with a good choice of :ref:`sequential container <ds_sequential>`.

This combination provides the several nice properties: the result data is
contiguous in memory (good for cache locality), has few allocations, is easy to
address (iterators in the final vector are just indices or pointers), and can be
efficiently queried with a standard binary search (e.g.
``std::lower_bound``; if you want the whole range of elements comparing
equal, use ``std::equal_range``).

.. _dss_smallset:

llvm/ADT/SmallSet.h
^^^^^^^^^^^^^^^^^^^

If you have a set-like data structure that is usually small and whose elements
are reasonably small, a ``SmallSet<Type, N>`` is a good choice.  This set has
space for N elements in place (thus, if the set is dynamically smaller than N,
no malloc traffic is required) and accesses them with a simple linear search.
When the set grows beyond N elements, it allocates a more expensive
representation that guarantees efficient access (for most types, it falls back
to :ref:`std::set <dss_set>`, but for pointers it uses something far better,
:ref:`SmallPtrSet <dss_smallptrset>`.

The magic of this class is that it handles small sets extremely efficiently, but
gracefully handles extremely large sets without loss of efficiency.  The
drawback is that the interface is quite small: it supports insertion, queries
and erasing, but does not support iteration.

.. _dss_smallptrset:

llvm/ADT/SmallPtrSet.h
^^^^^^^^^^^^^^^^^^^^^^

``SmallPtrSet`` has all the advantages of ``SmallSet`` (and a ``SmallSet`` of
pointers is transparently implemented with a ``SmallPtrSet``), but also supports
iterators.  If more than N insertions are performed, a single quadratically
probed hash table is allocated and grows as needed, providing extremely
efficient access (constant time insertion/deleting/queries with low constant
factors) and is very stingy with malloc traffic.

Note that, unlike :ref:`std::set <dss_set>`, the iterators of ``SmallPtrSet``
are invalidated whenever an insertion occurs.  Also, the values visited by the
iterators are not visited in sorted order.

.. _dss_stringset:

llvm/ADT/StringSet.h
^^^^^^^^^^^^^^^^^^^^

``StringSet`` is a thin wrapper around :ref:`StringMap\<char\> <dss_stringmap>`,
and it allows efficient storage and retrieval of unique strings.

Functionally analogous to ``SmallSet<StringRef>``, ``StringSet`` also suports
iteration. (The iterator dereferences to a ``StringMapEntry<char>``, so you
need to call ``i->getKey()`` to access the item of the StringSet.)  On the
other hand, ``StringSet`` doesn't support range-insertion and
copy-construction, which :ref:`SmallSet <dss_smallset>` and :ref:`SmallPtrSet
<dss_smallptrset>` do support.

.. _dss_denseset:

llvm/ADT/DenseSet.h
^^^^^^^^^^^^^^^^^^^

DenseSet is a simple quadratically probed hash table.  It excels at supporting
small values: it uses a single allocation to hold all of the pairs that are
currently inserted in the set.  DenseSet is a great way to unique small values
that are not simple pointers (use :ref:`SmallPtrSet <dss_smallptrset>` for
pointers).  Note that DenseSet has the same requirements for the value type that
:ref:`DenseMap <dss_densemap>` has.

.. _dss_sparseset:

llvm/ADT/SparseSet.h
^^^^^^^^^^^^^^^^^^^^

SparseSet holds a small number of objects identified by unsigned keys of
moderate size.  It uses a lot of memory, but provides operations that are almost
as fast as a vector.  Typical keys are physical registers, virtual registers, or
numbered basic blocks.

SparseSet is useful for algorithms that need very fast clear/find/insert/erase
and fast iteration over small sets.  It is not intended for building composite
data structures.

.. _dss_sparsemultiset:

llvm/ADT/SparseMultiSet.h
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

SparseMultiSet adds multiset behavior to SparseSet, while retaining SparseSet's
desirable attributes. Like SparseSet, it typically uses a lot of memory, but
provides operations that are almost as fast as a vector.  Typical keys are
physical registers, virtual registers, or numbered basic blocks.

SparseMultiSet is useful for algorithms that need very fast
clear/find/insert/erase of the entire collection, and iteration over sets of
elements sharing a key. It is often a more efficient choice than using composite
data structures (e.g. vector-of-vectors, map-of-vectors). It is not intended for
building composite data structures.

.. _dss_FoldingSet:

llvm/ADT/FoldingSet.h
^^^^^^^^^^^^^^^^^^^^^

FoldingSet is an aggregate class that is really good at uniquing
expensive-to-create or polymorphic objects.  It is a combination of a chained
hash table with intrusive links (uniqued objects are required to inherit from
FoldingSetNode) that uses :ref:`SmallVector <dss_smallvector>` as part of its ID
process.

Consider a case where you want to implement a "getOrCreateFoo" method for a
complex object (for example, a node in the code generator).  The client has a
description of **what** it wants to generate (it knows the opcode and all the
operands), but we don't want to 'new' a node, then try inserting it into a set
only to find out it already exists, at which point we would have to delete it
and return the node that already exists.

To support this style of client, FoldingSet perform a query with a
FoldingSetNodeID (which wraps SmallVector) that can be used to describe the
element that we want to query for.  The query either returns the element
matching the ID or it returns an opaque ID that indicates where insertion should
take place.  Construction of the ID usually does not require heap traffic.

Because FoldingSet uses intrusive links, it can support polymorphic objects in
the set (for example, you can have SDNode instances mixed with LoadSDNodes).
Because the elements are individually allocated, pointers to the elements are
stable: inserting or removing elements does not invalidate any pointers to other
elements.

.. _dss_set:

<set>
^^^^^

``std::set`` is a reasonable all-around set class, which is decent at many
things but great at nothing.  std::set allocates memory for each element
inserted (thus it is very malloc intensive) and typically stores three pointers
per element in the set (thus adding a large amount of per-element space
overhead).  It offers guaranteed log(n) performance, which is not particularly
fast from a complexity standpoint (particularly if the elements of the set are
expensive to compare, like strings), and has extremely high constant factors for
lookup, insertion and removal.

The advantages of std::set are that its iterators are stable (deleting or
inserting an element from the set does not affect iterators or pointers to other
elements) and that iteration over the set is guaranteed to be in sorted order.
If the elements in the set are large, then the relative overhead of the pointers
and malloc traffic is not a big deal, but if the elements of the set are small,
std::set is almost never a good choice.

.. _dss_setvector:

llvm/ADT/SetVector.h
^^^^^^^^^^^^^^^^^^^^

LLVM's ``SetVector<Type>`` is an adapter class that combines your choice of a
set-like container along with a :ref:`Sequential Container <ds_sequential>` The
important property that this provides is efficient insertion with uniquing
(duplicate elements are ignored) with iteration support.  It implements this by
inserting elements into both a set-like container and the sequential container,
using the set-like container for uniquing and the sequential container for
iteration.

The difference between SetVector and other sets is that the order of iteration
is guaranteed to match the order of insertion into the SetVector.  This property
is really important for things like sets of pointers.  Because pointer values
are non-deterministic (e.g. vary across runs of the program on different
machines), iterating over the pointers in the set will not be in a well-defined
order.

The drawback of SetVector is that it requires twice as much space as a normal
set and has the sum of constant factors from the set-like container and the
sequential container that it uses.  Use it **only** if you need to iterate over
the elements in a deterministic order.  SetVector is also expensive to delete
elements out of (linear time), unless you use its "pop_back" method, which is
faster.

``SetVector`` is an adapter class that defaults to using ``std::vector`` and a
size 16 ``SmallSet`` for the underlying containers, so it is quite expensive.
However, ``"llvm/ADT/SetVector.h"`` also provides a ``SmallSetVector`` class,
which defaults to using a ``SmallVector`` and ``SmallSet`` of a specified size.
If you use this, and if your sets are dynamically smaller than ``N``, you will
save a lot of heap traffic.

.. _dss_uniquevector:

llvm/ADT/UniqueVector.h
^^^^^^^^^^^^^^^^^^^^^^^

UniqueVector is similar to :ref:`SetVector <dss_setvector>` but it retains a
unique ID for each element inserted into the set.  It internally contains a map
and a vector, and it assigns a unique ID for each value inserted into the set.

UniqueVector is very expensive: its cost is the sum of the cost of maintaining
both the map and vector, it has high complexity, high constant factors, and
produces a lot of malloc traffic.  It should be avoided.

.. _dss_immutableset:

llvm/ADT/ImmutableSet.h
^^^^^^^^^^^^^^^^^^^^^^^

ImmutableSet is an immutable (functional) set implementation based on an AVL
tree.  Adding or removing elements is done through a Factory object and results
in the creation of a new ImmutableSet object.  If an ImmutableSet already exists
with the given contents, then the existing one is returned; equality is compared
with a FoldingSetNodeID.  The time and space complexity of add or remove
operations is logarithmic in the size of the original set.

There is no method for returning an element of the set, you can only check for
membership.

.. _dss_otherset:

Other Set-Like Container Options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The STL provides several other options, such as std::multiset and the various
"hash_set" like containers (whether from C++ TR1 or from the SGI library).  We
never use hash_set and unordered_set because they are generally very expensive
(each insertion requires a malloc) and very non-portable.

std::multiset is useful if you're not interested in elimination of duplicates,
but has all the drawbacks of :ref:`std::set <dss_set>`.  A sorted vector
(where you don't delete duplicate entries) or some other approach is almost
always better.

.. _ds_map:

Map-Like Containers (std::map, DenseMap, etc)
---------------------------------------------

Map-like containers are useful when you want to associate data to a key.  As
usual, there are a lot of different ways to do this. :)

.. _dss_sortedvectormap:

A sorted 'vector'
^^^^^^^^^^^^^^^^^

If your usage pattern follows a strict insert-then-query approach, you can
trivially use the same approach as :ref:`sorted vectors for set-like containers
<dss_sortedvectorset>`.  The only difference is that your query function (which
uses std::lower_bound to get efficient log(n) lookup) should only compare the
key, not both the key and value.  This yields the same advantages as sorted
vectors for sets.

.. _dss_stringmap:

llvm/ADT/StringMap.h
^^^^^^^^^^^^^^^^^^^^

Strings are commonly used as keys in maps, and they are difficult to support
efficiently: they are variable length, inefficient to hash and compare when
long, expensive to copy, etc.  StringMap is a specialized container designed to
cope with these issues.  It supports mapping an arbitrary range of bytes to an
arbitrary other object.

The StringMap implementation uses a quadratically-probed hash table, where the
buckets store a pointer to the heap allocated entries (and some other stuff).
The entries in the map must be heap allocated because the strings are variable
length.  The string data (key) and the element object (value) are stored in the
same allocation with the string data immediately after the element object.
This container guarantees the "``(char*)(&Value+1)``" points to the key string
for a value.

The StringMap is very fast for several reasons: quadratic probing is very cache
efficient for lookups, the hash value of strings in buckets is not recomputed
when looking up an element, StringMap rarely has to touch the memory for
unrelated objects when looking up a value (even when hash collisions happen),
hash table growth does not recompute the hash values for strings already in the
table, and each pair in the map is store in a single allocation (the string data
is stored in the same allocation as the Value of a pair).

StringMap also provides query methods that take byte ranges, so it only ever
copies a string if a value is inserted into the table.

StringMap iteratation order, however, is not guaranteed to be deterministic, so
any uses which require that should instead use a std::map.

.. _dss_indexmap:

llvm/ADT/IndexedMap.h
^^^^^^^^^^^^^^^^^^^^^

IndexedMap is a specialized container for mapping small dense integers (or
values that can be mapped to small dense integers) to some other type.  It is
internally implemented as a vector with a mapping function that maps the keys
to the dense integer range.

This is useful for cases like virtual registers in the LLVM code generator: they
have a dense mapping that is offset by a compile-time constant (the first
virtual register ID).

.. _dss_densemap:

llvm/ADT/DenseMap.h
^^^^^^^^^^^^^^^^^^^

DenseMap is a simple quadratically probed hash table.  It excels at supporting
small keys and values: it uses a single allocation to hold all of the pairs
that are currently inserted in the map.  DenseMap is a great way to map
pointers to pointers, or map other small types to each other.

There are several aspects of DenseMap that you should be aware of, however.
The iterators in a DenseMap are invalidated whenever an insertion occurs,
unlike map.  Also, because DenseMap allocates space for a large number of
key/value pairs (it starts with 64 by default), it will waste a lot of space if
your keys or values are large.  Finally, you must implement a partial
specialization of DenseMapInfo for the key that you want, if it isn't already
supported.  This is required to tell DenseMap about two special marker values
(which can never be inserted into the map) that it needs internally.

DenseMap's find_as() method supports lookup operations using an alternate key
type.  This is useful in cases where the normal key type is expensive to
construct, but cheap to compare against.  The DenseMapInfo is responsible for
defining the appropriate comparison and hashing methods for each alternate key
type used.

.. _dss_valuemap:

llvm/IR/ValueMap.h
^^^^^^^^^^^^^^^^^^^

ValueMap is a wrapper around a :ref:`DenseMap <dss_densemap>` mapping
``Value*``\ s (or subclasses) to another type.  When a Value is deleted or
RAUW'ed, ValueMap will update itself so the new version of the key is mapped to
the same value, just as if the key were a WeakVH.  You can configure exactly how
this happens, and what else happens on these two events, by passing a ``Config``
parameter to the ValueMap template.

.. _dss_intervalmap:

llvm/ADT/IntervalMap.h
^^^^^^^^^^^^^^^^^^^^^^

IntervalMap is a compact map for small keys and values.  It maps key intervals
instead of single keys, and it will automatically coalesce adjacent intervals.
When the map only contains a few intervals, they are stored in the map object
itself to avoid allocations.

The IntervalMap iterators are quite big, so they should not be passed around as
STL iterators.  The heavyweight iterators allow a smaller data structure.

.. _dss_map:

<map>
^^^^^

std::map has similar characteristics to :ref:`std::set <dss_set>`: it uses a
single allocation per pair inserted into the map, it offers log(n) lookup with
an extremely large constant factor, imposes a space penalty of 3 pointers per
pair in the map, etc.

std::map is most useful when your keys or values are very large, if you need to
iterate over the collection in sorted order, or if you need stable iterators
into the map (i.e. they don't get invalidated if an insertion or deletion of
another element takes place).

.. _dss_mapvector:

llvm/ADT/MapVector.h
^^^^^^^^^^^^^^^^^^^^

``MapVector<KeyT,ValueT>`` provides a subset of the DenseMap interface.  The
main difference is that the iteration order is guaranteed to be the insertion
order, making it an easy (but somewhat expensive) solution for non-deterministic
iteration over maps of pointers.

It is implemented by mapping from key to an index in a vector of key,value
pairs.  This provides fast lookup and iteration, but has two main drawbacks:
the key is stored twice and removing elements takes linear time.  If it is
necessary to remove elements, it's best to remove them in bulk using
``remove_if()``.

.. _dss_inteqclasses:

llvm/ADT/IntEqClasses.h
^^^^^^^^^^^^^^^^^^^^^^^

IntEqClasses provides a compact representation of equivalence classes of small
integers.  Initially, each integer in the range 0..n-1 has its own equivalence
class.  Classes can be joined by passing two class representatives to the
join(a, b) method.  Two integers are in the same class when findLeader() returns
the same representative.

Once all equivalence classes are formed, the map can be compressed so each
integer 0..n-1 maps to an equivalence class number in the range 0..m-1, where m
is the total number of equivalence classes.  The map must be uncompressed before
it can be edited again.

.. _dss_immutablemap:

llvm/ADT/ImmutableMap.h
^^^^^^^^^^^^^^^^^^^^^^^

ImmutableMap is an immutable (functional) map implementation based on an AVL
tree.  Adding or removing elements is done through a Factory object and results
in the creation of a new ImmutableMap object.  If an ImmutableMap already exists
with the given key set, then the existing one is returned; equality is compared
with a FoldingSetNodeID.  The time and space complexity of add or remove
operations is logarithmic in the size of the original map.

.. _dss_othermap:

Other Map-Like Container Options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The STL provides several other options, such as std::multimap and the various
"hash_map" like containers (whether from C++ TR1 or from the SGI library).  We
never use hash_set and unordered_set because they are generally very expensive
(each insertion requires a malloc) and very non-portable.

std::multimap is useful if you want to map a key to multiple values, but has all
the drawbacks of std::map.  A sorted vector or some other approach is almost
always better.

.. _ds_bit:

Bit storage containers (BitVector, SparseBitVector)
---------------------------------------------------

Unlike the other containers, there are only two bit storage containers, and
choosing when to use each is relatively straightforward.

One additional option is ``std::vector<bool>``: we discourage its use for two
reasons 1) the implementation in many common compilers (e.g.  commonly
available versions of GCC) is extremely inefficient and 2) the C++ standards
committee is likely to deprecate this container and/or change it significantly
somehow.  In any case, please don't use it.

.. _dss_bitvector:

BitVector
^^^^^^^^^

The BitVector container provides a dynamic size set of bits for manipulation.
It supports individual bit setting/testing, as well as set operations.  The set
operations take time O(size of bitvector), but operations are performed one word
at a time, instead of one bit at a time.  This makes the BitVector very fast for
set operations compared to other containers.  Use the BitVector when you expect
the number of set bits to be high (i.e. a dense set).

.. _dss_smallbitvector:

SmallBitVector
^^^^^^^^^^^^^^

The SmallBitVector container provides the same interface as BitVector, but it is
optimized for the case where only a small number of bits, less than 25 or so,
are needed.  It also transparently supports larger bit counts, but slightly less
efficiently than a plain BitVector, so SmallBitVector should only be used when
larger counts are rare.

At this time, SmallBitVector does not support set operations (and, or, xor), and
its operator[] does not provide an assignable lvalue.

.. _dss_sparsebitvector:

SparseBitVector
^^^^^^^^^^^^^^^

The SparseBitVector container is much like BitVector, with one major difference:
Only the bits that are set, are stored.  This makes the SparseBitVector much
more space efficient than BitVector when the set is sparse, as well as making
set operations O(number of set bits) instead of O(size of universe).  The
downside to the SparseBitVector is that setting and testing of random bits is
O(N), and on large SparseBitVectors, this can be slower than BitVector.  In our
implementation, setting or testing bits in sorted order (either forwards or
reverse) is O(1) worst case.  Testing and setting bits within 128 bits (depends
on size) of the current bit is also O(1).  As a general statement,
testing/setting bits in a SparseBitVector is O(distance away from last set bit).

.. _common:

Helpful Hints for Common Operations
===================================

This section describes how to perform some very simple transformations of LLVM
code.  This is meant to give examples of common idioms used, showing the
practical side of LLVM transformations.

Because this is a "how-to" section, you should also read about the main classes
that you will be working with.  The :ref:`Core LLVM Class Hierarchy Reference
<coreclasses>` contains details and descriptions of the main classes that you
should know about.

.. _inspection:

Basic Inspection and Traversal Routines
---------------------------------------

The LLVM compiler infrastructure have many different data structures that may be
traversed.  Following the example of the C++ standard template library, the
techniques used to traverse these various data structures are all basically the
same.  For a enumerable sequence of values, the ``XXXbegin()`` function (or
method) returns an iterator to the start of the sequence, the ``XXXend()``
function returns an iterator pointing to one past the last valid element of the
sequence, and there is some ``XXXiterator`` data type that is common between the
two operations.

Because the pattern for iteration is common across many different aspects of the
program representation, the standard template library algorithms may be used on
them, and it is easier to remember how to iterate.  First we show a few common
examples of the data structures that need to be traversed.  Other data
structures are traversed in very similar ways.

.. _iterate_function:

Iterating over the ``BasicBlock`` in a ``Function``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

It's quite common to have a ``Function`` instance that you'd like to transform
in some way; in particular, you'd like to manipulate its ``BasicBlock``\ s.  To
facilitate this, you'll need to iterate over all of the ``BasicBlock``\ s that
constitute the ``Function``.  The following is an example that prints the name
of a ``BasicBlock`` and the number of ``Instruction``\ s it contains:

.. code-block:: c++

  // func is a pointer to a Function instance
  for (Function::iterator i = func->begin(), e = func->end(); i != e; ++i)
    // Print out the name of the basic block if it has one, and then the
    // number of instructions that it contains
    errs() << "Basic block (name=" << i->getName() << ") has "
               << i->size() << " instructions.\n";

Note that i can be used as if it were a pointer for the purposes of invoking
member functions of the ``Instruction`` class.  This is because the indirection
operator is overloaded for the iterator classes.  In the above code, the
expression ``i->size()`` is exactly equivalent to ``(*i).size()`` just like
you'd expect.

.. _iterate_basicblock:

Iterating over the ``Instruction`` in a ``BasicBlock``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Just like when dealing with ``BasicBlock``\ s in ``Function``\ s, it's easy to
iterate over the individual instructions that make up ``BasicBlock``\ s.  Here's
a code snippet that prints out each instruction in a ``BasicBlock``:

.. code-block:: c++

  // blk is a pointer to a BasicBlock instance
  for (BasicBlock::iterator i = blk->begin(), e = blk->end(); i != e; ++i)
     // The next statement works since operator<<(ostream&,...)
     // is overloaded for Instruction&
     errs() << *i << "\n";


However, this isn't really the best way to print out the contents of a
``BasicBlock``!  Since the ostream operators are overloaded for virtually
anything you'll care about, you could have just invoked the print routine on the
basic block itself: ``errs() << *blk << "\n";``.

.. _iterate_insiter:

Iterating over the ``Instruction`` in a ``Function``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you're finding that you commonly iterate over a ``Function``'s
``BasicBlock``\ s and then that ``BasicBlock``'s ``Instruction``\ s,
``InstIterator`` should be used instead.  You'll need to include
``llvm/IR/InstIterator.h`` (`doxygen
<http://llvm.org/doxygen/InstIterator_8h.html>`__) and then instantiate
``InstIterator``\ s explicitly in your code.  Here's a small example that shows
how to dump all instructions in a function to the standard error stream:

.. code-block:: c++

  #include "llvm/IR/InstIterator.h"

  // F is a pointer to a Function instance
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
    errs() << *I << "\n";

Easy, isn't it?  You can also use ``InstIterator``\ s to fill a work list with
its initial contents.  For example, if you wanted to initialize a work list to
contain all instructions in a ``Function`` F, all you would need to do is
something like:

.. code-block:: c++

  std::set<Instruction*> worklist;
  // or better yet, SmallPtrSet<Instruction*, 64> worklist;

  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
    worklist.insert(&*I);

The STL set ``worklist`` would now contain all instructions in the ``Function``
pointed to by F.

.. _iterate_convert:

Turning an iterator into a class pointer (and vice-versa)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Sometimes, it'll be useful to grab a reference (or pointer) to a class instance
when all you've got at hand is an iterator.  Well, extracting a reference or a
pointer from an iterator is very straight-forward.  Assuming that ``i`` is a
``BasicBlock::iterator`` and ``j`` is a ``BasicBlock::const_iterator``:

.. code-block:: c++

  Instruction& inst = *i;   // Grab reference to instruction reference
  Instruction* pinst = &*i; // Grab pointer to instruction reference
  const Instruction& inst = *j;

However, the iterators you'll be working with in the LLVM framework are special:
they will automatically convert to a ptr-to-instance type whenever they need to.
Instead of derferencing the iterator and then taking the address of the result,
you can simply assign the iterator to the proper pointer type and you get the
dereference and address-of operation as a result of the assignment (behind the
scenes, this is a result of overloading casting mechanisms).  Thus the second
line of the last example,

.. code-block:: c++

  Instruction *pinst = &*i;

is semantically equivalent to

.. code-block:: c++

  Instruction *pinst = i;

It's also possible to turn a class pointer into the corresponding iterator, and
this is a constant time operation (very efficient).  The following code snippet
illustrates use of the conversion constructors provided by LLVM iterators.  By
using these, you can explicitly grab the iterator of something without actually
obtaining it via iteration over some structure:

.. code-block:: c++

  void printNextInstruction(Instruction* inst) {
    BasicBlock::iterator it(inst);
    ++it; // After this line, it refers to the instruction after *inst
    if (it != inst->getParent()->end()) errs() << *it << "\n";
  }

Unfortunately, these implicit conversions come at a cost; they prevent these
iterators from conforming to standard iterator conventions, and thus from being
usable with standard algorithms and containers.  For example, they prevent the
following code, where ``B`` is a ``BasicBlock``, from compiling:

.. code-block:: c++

  llvm::SmallVector<llvm::Instruction *, 16>(B->begin(), B->end());

Because of this, these implicit conversions may be removed some day, and
``operator*`` changed to return a pointer instead of a reference.

.. _iterate_complex:

Finding call sites: a slightly more complex example
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Say that you're writing a FunctionPass and would like to count all the locations
in the entire module (that is, across every ``Function``) where a certain
function (i.e., some ``Function *``) is already in scope.  As you'll learn
later, you may want to use an ``InstVisitor`` to accomplish this in a much more
straight-forward manner, but this example will allow us to explore how you'd do
it if you didn't have ``InstVisitor`` around.  In pseudo-code, this is what we
want to do:

.. code-block:: none

  initialize callCounter to zero
  for each Function f in the Module
    for each BasicBlock b in f
      for each Instruction i in b
        if (i is a CallInst and calls the given function)
          increment callCounter

And the actual code is (remember, because we're writing a ``FunctionPass``, our
``FunctionPass``-derived class simply has to override the ``runOnFunction``
method):

.. code-block:: c++

  Function* targetFunc = ...;

  class OurFunctionPass : public FunctionPass {
    public:
      OurFunctionPass(): callCounter(0) { }

      virtual runOnFunction(Function& F) {
        for (Function::iterator b = F.begin(), be = F.end(); b != be; ++b) {
          for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; ++i) {
            if (CallInst* callInst = dyn_cast<CallInst>(&*i)) {
              // We know we've encountered a call instruction, so we
              // need to determine if it's a call to the
              // function pointed to by m_func or not.
              if (callInst->getCalledFunction() == targetFunc)
                ++callCounter;
            }
          }
        }
      }

    private:
      unsigned callCounter;
  };

.. _calls_and_invokes:

Treating calls and invokes the same way
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You may have noticed that the previous example was a bit oversimplified in that
it did not deal with call sites generated by 'invoke' instructions.  In this,
and in other situations, you may find that you want to treat ``CallInst``\ s and
``InvokeInst``\ s the same way, even though their most-specific common base
class is ``Instruction``, which includes lots of less closely-related things.
For these cases, LLVM provides a handy wrapper class called ``CallSite``
(`doxygen <http://llvm.org/doxygen/classllvm_1_1CallSite.html>`__) It is
essentially a wrapper around an ``Instruction`` pointer, with some methods that
provide functionality common to ``CallInst``\ s and ``InvokeInst``\ s.

This class has "value semantics": it should be passed by value, not by reference
and it should not be dynamically allocated or deallocated using ``operator new``
or ``operator delete``.  It is efficiently copyable, assignable and
constructable, with costs equivalents to that of a bare pointer.  If you look at
its definition, it has only a single pointer member.

.. _iterate_chains:

Iterating over def-use & use-def chains
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Frequently, we might have an instance of the ``Value`` class (`doxygen
<http://llvm.org/doxygen/classllvm_1_1Value.html>`__) and we want to determine
which ``User`` s use the ``Value``.  The list of all ``User``\ s of a particular
``Value`` is called a *def-use* chain.  For example, let's say we have a
``Function*`` named ``F`` to a particular function ``foo``.  Finding all of the
instructions that *use* ``foo`` is as simple as iterating over the *def-use*
chain of ``F``:

.. code-block:: c++

  Function *F = ...;

  for (User *U : F->users()) {
    if (Instruction *Inst = dyn_cast<Instruction>(U)) {
      errs() << "F is used in instruction:\n";
      errs() << *Inst << "\n";
    }

Alternatively, it's common to have an instance of the ``User`` Class (`doxygen
<http://llvm.org/doxygen/classllvm_1_1User.html>`__) and need to know what
``Value``\ s are used by it.  The list of all ``Value``\ s used by a ``User`` is
known as a *use-def* chain.  Instances of class ``Instruction`` are common
``User`` s, so we might want to iterate over all of the values that a particular
instruction uses (that is, the operands of the particular ``Instruction``):

.. code-block:: c++

  Instruction *pi = ...;

  for (Use &U : pi->operands()) {
    Value *v = U.get();
    // ...
  }

Declaring objects as ``const`` is an important tool of enforcing mutation free
algorithms (such as analyses, etc.).  For this purpose above iterators come in
constant flavors as ``Value::const_use_iterator`` and
``Value::const_op_iterator``.  They automatically arise when calling
``use/op_begin()`` on ``const Value*``\ s or ``const User*``\ s respectively.
Upon dereferencing, they return ``const Use*``\ s.  Otherwise the above patterns
remain unchanged.

.. _iterate_preds:

Iterating over predecessors & successors of blocks
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Iterating over the predecessors and successors of a block is quite easy with the
routines defined in ``"llvm/IR/CFG.h"``.  Just use code like this to
iterate over all predecessors of BB:

.. code-block:: c++

  #include "llvm/Support/CFG.h"
  BasicBlock *BB = ...;

  for (pred_iterator PI = pred_begin(BB), E = pred_end(BB); PI != E; ++PI) {
    BasicBlock *Pred = *PI;
    // ...
  }

Similarly, to iterate over successors use ``succ_iterator/succ_begin/succ_end``.

.. _simplechanges:

Making simple changes
---------------------

There are some primitive transformation operations present in the LLVM
infrastructure that are worth knowing about.  When performing transformations,
it's fairly common to manipulate the contents of basic blocks.  This section
describes some of the common methods for doing so and gives example code.

.. _schanges_creating:

Creating and inserting new ``Instruction``\ s
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

*Instantiating Instructions*

Creation of ``Instruction``\ s is straight-forward: simply call the constructor
for the kind of instruction to instantiate and provide the necessary parameters.
For example, an ``AllocaInst`` only *requires* a (const-ptr-to) ``Type``.  Thus:

.. code-block:: c++

  AllocaInst* ai = new AllocaInst(Type::Int32Ty);

will create an ``AllocaInst`` instance that represents the allocation of one
integer in the current stack frame, at run time.  Each ``Instruction`` subclass
is likely to have varying default parameters which change the semantics of the
instruction, so refer to the `doxygen documentation for the subclass of
Instruction <http://llvm.org/doxygen/classllvm_1_1Instruction.html>`_ that
you're interested in instantiating.

*Naming values*

It is very useful to name the values of instructions when you're able to, as
this facilitates the debugging of your transformations.  If you end up looking
at generated LLVM machine code, you definitely want to have logical names
associated with the results of instructions!  By supplying a value for the
``Name`` (default) parameter of the ``Instruction`` constructor, you associate a
logical name with the result of the instruction's execution at run time.  For
example, say that I'm writing a transformation that dynamically allocates space
for an integer on the stack, and that integer is going to be used as some kind
of index by some other code.  To accomplish this, I place an ``AllocaInst`` at
the first point in the first ``BasicBlock`` of some ``Function``, and I'm
intending to use it within the same ``Function``.  I might do:

.. code-block:: c++

  AllocaInst* pa = new AllocaInst(Type::Int32Ty, 0, "indexLoc");

where ``indexLoc`` is now the logical name of the instruction's execution value,
which is a pointer to an integer on the run time stack.

*Inserting instructions*

There are essentially three ways to insert an ``Instruction`` into an existing
sequence of instructions that form a ``BasicBlock``:

* Insertion into an explicit instruction list

  Given a ``BasicBlock* pb``, an ``Instruction* pi`` within that ``BasicBlock``,
  and a newly-created instruction we wish to insert before ``*pi``, we do the
  following:

  .. code-block:: c++

      BasicBlock *pb = ...;
      Instruction *pi = ...;
      Instruction *newInst = new Instruction(...);

      pb->getInstList().insert(pi, newInst); // Inserts newInst before pi in pb

  Appending to the end of a ``BasicBlock`` is so common that the ``Instruction``
  class and ``Instruction``-derived classes provide constructors which take a
  pointer to a ``BasicBlock`` to be appended to.  For example code that looked
  like:

  .. code-block:: c++

    BasicBlock *pb = ...;
    Instruction *newInst = new Instruction(...);

    pb->getInstList().push_back(newInst); // Appends newInst to pb

  becomes:

  .. code-block:: c++

    BasicBlock *pb = ...;
    Instruction *newInst = new Instruction(..., pb);

  which is much cleaner, especially if you are creating long instruction
  streams.

* Insertion into an implicit instruction list

  ``Instruction`` instances that are already in ``BasicBlock``\ s are implicitly
  associated with an existing instruction list: the instruction list of the
  enclosing basic block.  Thus, we could have accomplished the same thing as the
  above code without being given a ``BasicBlock`` by doing:

  .. code-block:: c++

    Instruction *pi = ...;
    Instruction *newInst = new Instruction(...);

    pi->getParent()->getInstList().insert(pi, newInst);

  In fact, this sequence of steps occurs so frequently that the ``Instruction``
  class and ``Instruction``-derived classes provide constructors which take (as
  a default parameter) a pointer to an ``Instruction`` which the newly-created
  ``Instruction`` should precede.  That is, ``Instruction`` constructors are
  capable of inserting the newly-created instance into the ``BasicBlock`` of a
  provided instruction, immediately before that instruction.  Using an
  ``Instruction`` constructor with a ``insertBefore`` (default) parameter, the
  above code becomes:

  .. code-block:: c++

    Instruction* pi = ...;
    Instruction* newInst = new Instruction(..., pi);

  which is much cleaner, especially if you're creating a lot of instructions and
  adding them to ``BasicBlock``\ s.

* Insertion using an instance of ``IRBuilder``

  Inserting several ``Instruction``\ s can be quite laborious using the previous
  methods. The ``IRBuilder`` is a convenience class that can be used to add
  several instructions to the end of a ``BasicBlock`` or before a particular
  ``Instruction``. It also supports constant folding and renaming named
  registers (see ``IRBuilder``'s template arguments).

  The example below demonstrates a very simple use of the ``IRBuilder`` where
  three instructions are inserted before the instruction ``pi``. The first two
  instructions are Call instructions and third instruction multiplies the return
  value of the two calls.

  .. code-block:: c++

    Instruction *pi = ...;
    IRBuilder<> Builder(pi);
    CallInst* callOne = Builder.CreateCall(...);
    CallInst* callTwo = Builder.CreateCall(...);
    Value* result = Builder.CreateMul(callOne, callTwo);

  The example below is similar to the above example except that the created
  ``IRBuilder`` inserts instructions at the end of the ``BasicBlock`` ``pb``.

  .. code-block:: c++

    BasicBlock *pb = ...;
    IRBuilder<> Builder(pb);
    CallInst* callOne = Builder.CreateCall(...);
    CallInst* callTwo = Builder.CreateCall(...);
    Value* result = Builder.CreateMul(callOne, callTwo);


.. _schanges_deleting:

Deleting Instructions
^^^^^^^^^^^^^^^^^^^^^

Deleting an instruction from an existing sequence of instructions that form a
BasicBlock_ is very straight-forward: just call the instruction's
``eraseFromParent()`` method.  For example:

.. code-block:: c++

  Instruction *I = .. ;
  I->eraseFromParent();

This unlinks the instruction from its containing basic block and deletes it.  If
you'd just like to unlink the instruction from its containing basic block but
not delete it, you can use the ``removeFromParent()`` method.

.. _schanges_replacing:

Replacing an Instruction with another Value
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Replacing individual instructions
"""""""""""""""""""""""""""""""""

Including "`llvm/Transforms/Utils/BasicBlockUtils.h
<http://llvm.org/doxygen/BasicBlockUtils_8h-source.html>`_" permits use of two
very useful replace functions: ``ReplaceInstWithValue`` and
``ReplaceInstWithInst``.

.. _schanges_deleting_sub:

Deleting Instructions
"""""""""""""""""""""

* ``ReplaceInstWithValue``

  This function replaces all uses of a given instruction with a value, and then
  removes the original instruction.  The following example illustrates the
  replacement of the result of a particular ``AllocaInst`` that allocates memory
  for a single integer with a null pointer to an integer.

  .. code-block:: c++

    AllocaInst* instToReplace = ...;
    BasicBlock::iterator ii(instToReplace);

    ReplaceInstWithValue(instToReplace->getParent()->getInstList(), ii,
                         Constant::getNullValue(PointerType::getUnqual(Type::Int32Ty)));

* ``ReplaceInstWithInst``

  This function replaces a particular instruction with another instruction,
  inserting the new instruction into the basic block at the location where the
  old instruction was, and replacing any uses of the old instruction with the
  new instruction.  The following example illustrates the replacement of one
  ``AllocaInst`` with another.

  .. code-block:: c++

    AllocaInst* instToReplace = ...;
    BasicBlock::iterator ii(instToReplace);

    ReplaceInstWithInst(instToReplace->getParent()->getInstList(), ii,
                        new AllocaInst(Type::Int32Ty, 0, "ptrToReplacedInt"));


Replacing multiple uses of Users and Values
"""""""""""""""""""""""""""""""""""""""""""

You can use ``Value::replaceAllUsesWith`` and ``User::replaceUsesOfWith`` to
change more than one use at a time.  See the doxygen documentation for the
`Value Class <http://llvm.org/doxygen/classllvm_1_1Value.html>`_ and `User Class
<http://llvm.org/doxygen/classllvm_1_1User.html>`_, respectively, for more
information.

.. _schanges_deletingGV:

Deleting GlobalVariables
^^^^^^^^^^^^^^^^^^^^^^^^

Deleting a global variable from a module is just as easy as deleting an
Instruction.  First, you must have a pointer to the global variable that you
wish to delete.  You use this pointer to erase it from its parent, the module.
For example:

.. code-block:: c++

  GlobalVariable *GV = .. ;

  GV->eraseFromParent();


.. _create_types:

How to Create Types
-------------------

In generating IR, you may need some complex types.  If you know these types
statically, you can use ``TypeBuilder<...>::get()``, defined in
``llvm/Support/TypeBuilder.h``, to retrieve them.  ``TypeBuilder`` has two forms
depending on whether you're building types for cross-compilation or native
library use.  ``TypeBuilder<T, true>`` requires that ``T`` be independent of the
host environment, meaning that it's built out of types from the ``llvm::types``
(`doxygen <http://llvm.org/doxygen/namespacellvm_1_1types.html>`__) namespace
and pointers, functions, arrays, etc. built of those.  ``TypeBuilder<T, false>``
additionally allows native C types whose size may depend on the host compiler.
For example,

.. code-block:: c++

  FunctionType *ft = TypeBuilder<types::i<8>(types::i<32>*), true>::get();

is easier to read and write than the equivalent

.. code-block:: c++

  std::vector<const Type*> params;
  params.push_back(PointerType::getUnqual(Type::Int32Ty));
  FunctionType *ft = FunctionType::get(Type::Int8Ty, params, false);

See the `class comment
<http://llvm.org/doxygen/TypeBuilder_8h-source.html#l00001>`_ for more details.

.. _threading:

Threads and LLVM
================

This section describes the interaction of the LLVM APIs with multithreading,
both on the part of client applications, and in the JIT, in the hosted
application.

Note that LLVM's support for multithreading is still relatively young.  Up
through version 2.5, the execution of threaded hosted applications was
supported, but not threaded client access to the APIs.  While this use case is
now supported, clients *must* adhere to the guidelines specified below to ensure
proper operation in multithreaded mode.

Note that, on Unix-like platforms, LLVM requires the presence of GCC's atomic
intrinsics in order to support threaded operation.  If you need a
multhreading-capable LLVM on a platform without a suitably modern system
compiler, consider compiling LLVM and LLVM-GCC in single-threaded mode, and
using the resultant compiler to build a copy of LLVM with multithreading
support.

.. _shutdown:

Ending Execution with ``llvm_shutdown()``
-----------------------------------------

When you are done using the LLVM APIs, you should call ``llvm_shutdown()`` to
deallocate memory used for internal structures.

.. _managedstatic:

Lazy Initialization with ``ManagedStatic``
------------------------------------------

``ManagedStatic`` is a utility class in LLVM used to implement static
initialization of static resources, such as the global type tables.  In a
single-threaded environment, it implements a simple lazy initialization scheme.
When LLVM is compiled with support for multi-threading, however, it uses
double-checked locking to implement thread-safe lazy initialization.

.. _llvmcontext:

Achieving Isolation with ``LLVMContext``
----------------------------------------

``LLVMContext`` is an opaque class in the LLVM API which clients can use to
operate multiple, isolated instances of LLVM concurrently within the same
address space.  For instance, in a hypothetical compile-server, the compilation
of an individual translation unit is conceptually independent from all the
others, and it would be desirable to be able to compile incoming translation
units concurrently on independent server threads.  Fortunately, ``LLVMContext``
exists to enable just this kind of scenario!

Conceptually, ``LLVMContext`` provides isolation.  Every LLVM entity
(``Module``\ s, ``Value``\ s, ``Type``\ s, ``Constant``\ s, etc.) in LLVM's
in-memory IR belongs to an ``LLVMContext``.  Entities in different contexts
*cannot* interact with each other: ``Module``\ s in different contexts cannot be
linked together, ``Function``\ s cannot be added to ``Module``\ s in different
contexts, etc.  What this means is that is is safe to compile on multiple
threads simultaneously, as long as no two threads operate on entities within the
same context.

In practice, very few places in the API require the explicit specification of a
``LLVMContext``, other than the ``Type`` creation/lookup APIs.  Because every
``Type`` carries a reference to its owning context, most other entities can
determine what context they belong to by looking at their own ``Type``.  If you
are adding new entities to LLVM IR, please try to maintain this interface
design.

For clients that do *not* require the benefits of isolation, LLVM provides a
convenience API ``getGlobalContext()``.  This returns a global, lazily
initialized ``LLVMContext`` that may be used in situations where isolation is
not a concern.

.. _jitthreading:

Threads and the JIT
-------------------

LLVM's "eager" JIT compiler is safe to use in threaded programs.  Multiple
threads can call ``ExecutionEngine::getPointerToFunction()`` or
``ExecutionEngine::runFunction()`` concurrently, and multiple threads can run
code output by the JIT concurrently.  The user must still ensure that only one
thread accesses IR in a given ``LLVMContext`` while another thread might be
modifying it.  One way to do that is to always hold the JIT lock while accessing
IR outside the JIT (the JIT *modifies* the IR by adding ``CallbackVH``\ s).
Another way is to only call ``getPointerToFunction()`` from the
``LLVMContext``'s thread.

When the JIT is configured to compile lazily (using
``ExecutionEngine::DisableLazyCompilation(false)``), there is currently a `race
condition <http://llvm.org/bugs/show_bug.cgi?id=5184>`_ in updating call sites
after a function is lazily-jitted.  It's still possible to use the lazy JIT in a
threaded program if you ensure that only one thread at a time can call any
particular lazy stub and that the JIT lock guards any IR access, but we suggest
using only the eager JIT in threaded programs.

.. _advanced:

Advanced Topics
===============

This section describes some of the advanced or obscure API's that most clients
do not need to be aware of.  These API's tend manage the inner workings of the
LLVM system, and only need to be accessed in unusual circumstances.

.. _SymbolTable:

The ``ValueSymbolTable`` class
------------------------------

The ``ValueSymbolTable`` (`doxygen
<http://llvm.org/doxygen/classllvm_1_1ValueSymbolTable.html>`__) class provides
a symbol table that the :ref:`Function <c_Function>` and Module_ classes use for
naming value definitions.  The symbol table can provide a name for any Value_.

Note that the ``SymbolTable`` class should not be directly accessed by most
clients.  It should only be used when iteration over the symbol table names
themselves are required, which is very special purpose.  Note that not all LLVM
Value_\ s have names, and those without names (i.e. they have an empty name) do
not exist in the symbol table.

Symbol tables support iteration over the values in the symbol table with
``begin/end/iterator`` and supports querying to see if a specific name is in the
symbol table (with ``lookup``).  The ``ValueSymbolTable`` class exposes no
public mutator methods, instead, simply call ``setName`` on a value, which will
autoinsert it into the appropriate symbol table.

.. _UserLayout:

The ``User`` and owned ``Use`` classes' memory layout
-----------------------------------------------------

The ``User`` (`doxygen <http://llvm.org/doxygen/classllvm_1_1User.html>`__)
class provides a basis for expressing the ownership of ``User`` towards other
`Value instance <http://llvm.org/doxygen/classllvm_1_1Value.html>`_\ s.  The
``Use`` (`doxygen <http://llvm.org/doxygen/classllvm_1_1Use.html>`__) helper
class is employed to do the bookkeeping and to facilitate *O(1)* addition and
removal.

.. _Use2User:

Interaction and relationship between ``User`` and ``Use`` objects
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A subclass of ``User`` can choose between incorporating its ``Use`` objects or
refer to them out-of-line by means of a pointer.  A mixed variant (some ``Use``
s inline others hung off) is impractical and breaks the invariant that the
``Use`` objects belonging to the same ``User`` form a contiguous array.

We have 2 different layouts in the ``User`` (sub)classes:

* Layout a)

  The ``Use`` object(s) are inside (resp. at fixed offset) of the ``User``
  object and there are a fixed number of them.

* Layout b)

  The ``Use`` object(s) are referenced by a pointer to an array from the
  ``User`` object and there may be a variable number of them.

As of v2.4 each layout still possesses a direct pointer to the start of the
array of ``Use``\ s.  Though not mandatory for layout a), we stick to this
redundancy for the sake of simplicity.  The ``User`` object also stores the
number of ``Use`` objects it has. (Theoretically this information can also be
calculated given the scheme presented below.)

Special forms of allocation operators (``operator new``) enforce the following
memory layouts:

* Layout a) is modelled by prepending the ``User`` object by the ``Use[]``
  array.

  .. code-block:: none

    ...---.---.---.---.-------...
      | P | P | P | P | User
    '''---'---'---'---'-------'''

* Layout b) is modelled by pointing at the ``Use[]`` array.

  .. code-block:: none

    .-------...
    | User
    '-------'''
        |
        v
        .---.---.---.---...
        | P | P | P | P |
        '---'---'---'---'''

*(In the above figures* '``P``' *stands for the* ``Use**`` *that is stored in
each* ``Use`` *object in the member* ``Use::Prev`` *)*

.. _Waymarking:

The waymarking algorithm
^^^^^^^^^^^^^^^^^^^^^^^^

Since the ``Use`` objects are deprived of the direct (back)pointer to their
``User`` objects, there must be a fast and exact method to recover it.  This is
accomplished by the following scheme:

A bit-encoding in the 2 LSBits (least significant bits) of the ``Use::Prev``
allows to find the start of the ``User`` object:

* ``00`` --- binary digit 0

* ``01`` --- binary digit 1

* ``10`` --- stop and calculate (``s``)

* ``11`` --- full stop (``S``)

Given a ``Use*``, all we have to do is to walk till we get a stop and we either
have a ``User`` immediately behind or we have to walk to the next stop picking
up digits and calculating the offset:

.. code-block:: none

  .---.---.---.---.---.---.---.---.---.---.---.---.---.---.---.---.----------------
  | 1 | s | 1 | 0 | 1 | 0 | s | 1 | 1 | 0 | s | 1 | 1 | s | 1 | S | User (or User*)
  '---'---'---'---'---'---'---'---'---'---'---'---'---'---'---'---'----------------
      |+15                |+10            |+6         |+3     |+1
      |                   |               |           |       | __>
      |                   |               |           | __________>
      |                   |               | ______________________>
      |                   | ______________________________________>
      | __________________________________________________________>

Only the significant number of bits need to be stored between the stops, so that
the *worst case is 20 memory accesses* when there are 1000 ``Use`` objects
associated with a ``User``.

.. _ReferenceImpl:

Reference implementation
^^^^^^^^^^^^^^^^^^^^^^^^

The following literate Haskell fragment demonstrates the concept:

.. code-block:: haskell

  > import Test.QuickCheck
  >
  > digits :: Int -> [Char] -> [Char]
  > digits 0 acc = '0' : acc
  > digits 1 acc = '1' : acc
  > digits n acc = digits (n `div` 2) $ digits (n `mod` 2) acc
  >
  > dist :: Int -> [Char] -> [Char]
  > dist 0 [] = ['S']
  > dist 0 acc = acc
  > dist 1 acc = let r = dist 0 acc in 's' : digits (length r) r
  > dist n acc = dist (n - 1) $ dist 1 acc
  >
  > takeLast n ss = reverse $ take n $ reverse ss
  >
  > test = takeLast 40 $ dist 20 []
  >

Printing <test> gives: ``"1s100000s11010s10100s1111s1010s110s11s1S"``

The reverse algorithm computes the length of the string just by examining a
certain prefix:

.. code-block:: haskell

  > pref :: [Char] -> Int
  > pref "S" = 1
  > pref ('s':'1':rest) = decode 2 1 rest
  > pref (_:rest) = 1 + pref rest
  >
  > decode walk acc ('0':rest) = decode (walk + 1) (acc * 2) rest
  > decode walk acc ('1':rest) = decode (walk + 1) (acc * 2 + 1) rest
  > decode walk acc _ = walk + acc
  >

Now, as expected, printing <pref test> gives ``40``.

We can *quickCheck* this with following property:

.. code-block:: haskell

  > testcase = dist 2000 []
  > testcaseLength = length testcase
  >
  > identityProp n = n > 0 && n <= testcaseLength ==> length arr == pref arr
  >     where arr = takeLast n testcase
  >

As expected <quickCheck identityProp> gives:

::

  *Main> quickCheck identityProp
  OK, passed 100 tests.

Let's be a bit more exhaustive:

.. code-block:: haskell

  >
  > deepCheck p = check (defaultConfig { configMaxTest = 500 }) p
  >

And here is the result of <deepCheck identityProp>:

::

  *Main> deepCheck identityProp
  OK, passed 500 tests.

.. _Tagging:

Tagging considerations
^^^^^^^^^^^^^^^^^^^^^^

To maintain the invariant that the 2 LSBits of each ``Use**`` in ``Use`` never
change after being set up, setters of ``Use::Prev`` must re-tag the new
``Use**`` on every modification.  Accordingly getters must strip the tag bits.

For layout b) instead of the ``User`` we find a pointer (``User*`` with LSBit
set).  Following this pointer brings us to the ``User``.  A portable trick
ensures that the first bytes of ``User`` (if interpreted as a pointer) never has
the LSBit set. (Portability is relying on the fact that all known compilers
place the ``vptr`` in the first word of the instances.)

.. _polymorphism:

Designing Type Hiercharies and Polymorphic Interfaces
-----------------------------------------------------

There are two different design patterns that tend to result in the use of
virtual dispatch for methods in a type hierarchy in C++ programs. The first is
a genuine type hierarchy where different types in the hierarchy model
a specific subset of the functionality and semantics, and these types nest
strictly within each other. Good examples of this can be seen in the ``Value``
or ``Type`` type hierarchies.

A second is the desire to dispatch dynamically across a collection of
polymorphic interface implementations. This latter use case can be modeled with
virtual dispatch and inheritance by defining an abstract interface base class
which all implementations derive from and override. However, this
implementation strategy forces an **"is-a"** relationship to exist that is not
actually meaningful. There is often not some nested hierarchy of useful
generalizations which code might interact with and move up and down. Instead,
there is a singular interface which is dispatched across a range of
implementations.

The preferred implementation strategy for the second use case is that of
generic programming (sometimes called "compile-time duck typing" or "static
polymorphism"). For example, a template over some type parameter ``T`` can be
instantiated across any particular implementation that conforms to the
interface or *concept*. A good example here is the highly generic properties of
any type which models a node in a directed graph. LLVM models these primarily
through templates and generic programming. Such templates include the
``LoopInfoBase`` and ``DominatorTreeBase``. When this type of polymorphism
truly needs **dynamic** dispatch you can generalize it using a technique
called *concept-based polymorphism*. This pattern emulates the interfaces and
behaviors of templates using a very limited form of virtual dispatch for type
erasure inside its implementation. You can find examples of this technique in
the ``PassManager.h`` system, and there is a more detailed introduction to it
by Sean Parent in several of his talks and papers:

#. `Inheritance Is The Base Class of Evil
   <http://channel9.msdn.com/Events/GoingNative/2013/Inheritance-Is-The-Base-Class-of-Evil>`_
   - The GoingNative 2013 talk describing this technique, and probably the best
   place to start.
#. `Value Semantics and Concepts-based Polymorphism
   <http://www.youtube.com/watch?v=_BpMYeUFXv8>`_ - The C++Now! 2012 talk
   describing this technique in more detail.
#. `Sean Parent's Papers and Presentations
   <http://github.com/sean-parent/sean-parent.github.com/wiki/Papers-and-Presentations>`_
   - A Github project full of links to slides, video, and sometimes code.

When deciding between creating a type hierarchy (with either tagged or virtual
dispatch) and using templates or concepts-based polymorphism, consider whether
there is some refinement of an abstract base class which is a semantically
meaningful type on an interface boundary. If anything more refined than the
root abstract interface is meaningless to talk about as a partial extension of
the semantic model, then your use case likely fits better with polymorphism and
you should avoid using virtual dispatch. However, there may be some exigent
circumstances that require one technique or the other to be used.

If you do need to introduce a type hierarchy, we prefer to use explicitly
closed type hierarchies with manual tagged dispatch and/or RTTI rather than the
open inheritance model and virtual dispatch that is more common in C++ code.
This is because LLVM rarely encourages library consumers to extend its core
types, and leverages the closed and tag-dispatched nature of its hierarchies to
generate significantly more efficient code. We have also found that a large
amount of our usage of type hierarchies fits better with tag-based pattern
matching rather than dynamic dispatch across a common interface. Within LLVM we
have built custom helpers to facilitate this design. See this document's
section on :ref:`isa and dyn_cast <isa>` and our :doc:`detailed document
<HowToSetUpLLVMStyleRTTI>` which describes how you can implement this
pattern for use with the LLVM helpers.

.. _abi_breaking_checks:

ABI Breaking Checks
-------------------

Checks and asserts that alter the LLVM C++ ABI are predicated on the
preprocessor symbol `LLVM_ENABLE_ABI_BREAKING_CHECKS` -- LLVM
libraries built with `LLVM_ENABLE_ABI_BREAKING_CHECKS` are not ABI
compatible LLVM libraries built without it defined.  By default,
turning on assertions also turns on `LLVM_ENABLE_ABI_BREAKING_CHECKS`
so a default +Asserts build is not ABI compatible with a
default -Asserts build.  Clients that want ABI compatibility
between +Asserts and -Asserts builds should use the CMake or autoconf
build systems to set `LLVM_ENABLE_ABI_BREAKING_CHECKS` independently
of `LLVM_ENABLE_ASSERTIONS`.

.. _coreclasses:

The Core LLVM Class Hierarchy Reference
=======================================

``#include "llvm/IR/Type.h"``

header source: `Type.h <http://llvm.org/doxygen/Type_8h-source.html>`_

doxygen info: `Type Clases <http://llvm.org/doxygen/classllvm_1_1Type.html>`_

The Core LLVM classes are the primary means of representing the program being
inspected or transformed.  The core LLVM classes are defined in header files in
the ``include/llvm/IR`` directory, and implemented in the ``lib/IR``
directory. It's worth noting that, for historical reasons, this library is
called ``libLLVMCore.so``, not ``libLLVMIR.so`` as you might expect.

.. _Type:

The Type class and Derived Types
--------------------------------

``Type`` is a superclass of all type classes.  Every ``Value`` has a ``Type``.
``Type`` cannot be instantiated directly but only through its subclasses.
Certain primitive types (``VoidType``, ``LabelType``, ``FloatType`` and
``DoubleType``) have hidden subclasses.  They are hidden because they offer no
useful functionality beyond what the ``Type`` class offers except to distinguish
themselves from other subclasses of ``Type``.

All other types are subclasses of ``DerivedType``.  Types can be named, but this
is not a requirement.  There exists exactly one instance of a given shape at any
one time.  This allows type equality to be performed with address equality of
the Type Instance.  That is, given two ``Type*`` values, the types are identical
if the pointers are identical.

.. _m_Type:

Important Public Methods
^^^^^^^^^^^^^^^^^^^^^^^^

* ``bool isIntegerTy() const``: Returns true for any integer type.

* ``bool isFloatingPointTy()``: Return true if this is one of the five
  floating point types.

* ``bool isSized()``: Return true if the type has known size.  Things
  that don't have a size are abstract types, labels and void.

.. _derivedtypes:

Important Derived Types
^^^^^^^^^^^^^^^^^^^^^^^

``IntegerType``
  Subclass of DerivedType that represents integer types of any bit width.  Any
  bit width between ``IntegerType::MIN_INT_BITS`` (1) and
  ``IntegerType::MAX_INT_BITS`` (~8 million) can be represented.

  * ``static const IntegerType* get(unsigned NumBits)``: get an integer
    type of a specific bit width.

  * ``unsigned getBitWidth() const``: Get the bit width of an integer type.

``SequentialType``
  This is subclassed by ArrayType, PointerType and VectorType.

  * ``const Type * getElementType() const``: Returns the type of each
    of the elements in the sequential type.

``ArrayType``
  This is a subclass of SequentialType and defines the interface for array
  types.

  * ``unsigned getNumElements() const``: Returns the number of elements
    in the array.

``PointerType``
  Subclass of SequentialType for pointer types.

``VectorType``
  Subclass of SequentialType for vector types.  A vector type is similar to an
  ArrayType but is distinguished because it is a first class type whereas
  ArrayType is not.  Vector types are used for vector operations and are usually
  small vectors of an integer or floating point type.

``StructType``
  Subclass of DerivedTypes for struct types.

.. _FunctionType:

``FunctionType``
  Subclass of DerivedTypes for function types.

  * ``bool isVarArg() const``: Returns true if it's a vararg function.

  * ``const Type * getReturnType() const``: Returns the return type of the
    function.

  * ``const Type * getParamType (unsigned i)``: Returns the type of the ith
    parameter.

  * ``const unsigned getNumParams() const``: Returns the number of formal
    parameters.

.. _Module:

The ``Module`` class
--------------------

``#include "llvm/IR/Module.h"``

header source: `Module.h <http://llvm.org/doxygen/Module_8h-source.html>`_

doxygen info: `Module Class <http://llvm.org/doxygen/classllvm_1_1Module.html>`_

The ``Module`` class represents the top level structure present in LLVM
programs.  An LLVM module is effectively either a translation unit of the
original program or a combination of several translation units merged by the
linker.  The ``Module`` class keeps track of a list of :ref:`Function
<c_Function>`\ s, a list of GlobalVariable_\ s, and a SymbolTable_.
Additionally, it contains a few helpful member functions that try to make common
operations easy.

.. _m_Module:

Important Public Members of the ``Module`` class
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* ``Module::Module(std::string name = "")``

  Constructing a Module_ is easy.  You can optionally provide a name for it
  (probably based on the name of the translation unit).

* | ``Module::iterator`` - Typedef for function list iterator
  | ``Module::const_iterator`` - Typedef for const_iterator.
  | ``begin()``, ``end()``, ``size()``, ``empty()``

  These are forwarding methods that make it easy to access the contents of a
  ``Module`` object's :ref:`Function <c_Function>` list.

* ``Module::FunctionListType &getFunctionList()``

  Returns the list of :ref:`Function <c_Function>`\ s.  This is necessary to use
  when you need to update the list or perform a complex action that doesn't have
  a forwarding method.

----------------

* | ``Module::global_iterator`` - Typedef for global variable list iterator
  | ``Module::const_global_iterator`` - Typedef for const_iterator.
  | ``global_begin()``, ``global_end()``, ``global_size()``, ``global_empty()``

  These are forwarding methods that make it easy to access the contents of a
  ``Module`` object's GlobalVariable_ list.

* ``Module::GlobalListType &getGlobalList()``

  Returns the list of GlobalVariable_\ s.  This is necessary to use when you
  need to update the list or perform a complex action that doesn't have a
  forwarding method.

----------------

* ``SymbolTable *getSymbolTable()``

  Return a reference to the SymbolTable_ for this ``Module``.

----------------

* ``Function *getFunction(StringRef Name) const``

  Look up the specified function in the ``Module`` SymbolTable_.  If it does not
  exist, return ``null``.

* ``Function *getOrInsertFunction(const std::string &Name, const FunctionType
  *T)``

  Look up the specified function in the ``Module`` SymbolTable_.  If it does not
  exist, add an external declaration for the function and return it.

* ``std::string getTypeName(const Type *Ty)``

  If there is at least one entry in the SymbolTable_ for the specified Type_,
  return it.  Otherwise return the empty string.

* ``bool addTypeName(const std::string &Name, const Type *Ty)``

  Insert an entry in the SymbolTable_ mapping ``Name`` to ``Ty``.  If there is
  already an entry for this name, true is returned and the SymbolTable_ is not
  modified.

.. _Value:

The ``Value`` class
-------------------

``#include "llvm/IR/Value.h"``

header source: `Value.h <http://llvm.org/doxygen/Value_8h-source.html>`_

doxygen info: `Value Class <http://llvm.org/doxygen/classllvm_1_1Value.html>`_

The ``Value`` class is the most important class in the LLVM Source base.  It
represents a typed value that may be used (among other things) as an operand to
an instruction.  There are many different types of ``Value``\ s, such as
Constant_\ s, Argument_\ s.  Even Instruction_\ s and :ref:`Function
<c_Function>`\ s are ``Value``\ s.

A particular ``Value`` may be used many times in the LLVM representation for a
program.  For example, an incoming argument to a function (represented with an
instance of the Argument_ class) is "used" by every instruction in the function
that references the argument.  To keep track of this relationship, the ``Value``
class keeps a list of all of the ``User``\ s that is using it (the User_ class
is a base class for all nodes in the LLVM graph that can refer to ``Value``\ s).
This use list is how LLVM represents def-use information in the program, and is
accessible through the ``use_*`` methods, shown below.

Because LLVM is a typed representation, every LLVM ``Value`` is typed, and this
Type_ is available through the ``getType()`` method.  In addition, all LLVM
values can be named.  The "name" of the ``Value`` is a symbolic string printed
in the LLVM code:

.. code-block:: llvm

  %foo = add i32 1, 2

.. _nameWarning:

The name of this instruction is "foo". **NOTE** that the name of any value may
be missing (an empty string), so names should **ONLY** be used for debugging
(making the source code easier to read, debugging printouts), they should not be
used to keep track of values or map between them.  For this purpose, use a
``std::map`` of pointers to the ``Value`` itself instead.

One important aspect of LLVM is that there is no distinction between an SSA
variable and the operation that produces it.  Because of this, any reference to
the value produced by an instruction (or the value available as an incoming
argument, for example) is represented as a direct pointer to the instance of the
class that represents this value.  Although this may take some getting used to,
it simplifies the representation and makes it easier to manipulate.

.. _m_Value:

Important Public Members of the ``Value`` class
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* | ``Value::use_iterator`` - Typedef for iterator over the use-list
  | ``Value::const_use_iterator`` - Typedef for const_iterator over the
    use-list
  | ``unsigned use_size()`` - Returns the number of users of the value.
  | ``bool use_empty()`` - Returns true if there are no users.
  | ``use_iterator use_begin()`` - Get an iterator to the start of the
    use-list.
  | ``use_iterator use_end()`` - Get an iterator to the end of the use-list.
  | ``User *use_back()`` - Returns the last element in the list.

  These methods are the interface to access the def-use information in LLVM.
  As with all other iterators in LLVM, the naming conventions follow the
  conventions defined by the STL_.

* ``Type *getType() const``
  This method returns the Type of the Value.

* | ``bool hasName() const``
  | ``std::string getName() const``
  | ``void setName(const std::string &Name)``

  This family of methods is used to access and assign a name to a ``Value``, be
  aware of the :ref:`precaution above <nameWarning>`.

* ``void replaceAllUsesWith(Value *V)``

  This method traverses the use list of a ``Value`` changing all User_\ s of the
  current value to refer to "``V``" instead.  For example, if you detect that an
  instruction always produces a constant value (for example through constant
  folding), you can replace all uses of the instruction with the constant like
  this:

  .. code-block:: c++

    Inst->replaceAllUsesWith(ConstVal);

.. _User:

The ``User`` class
------------------

``#include "llvm/IR/User.h"``

header source: `User.h <http://llvm.org/doxygen/User_8h-source.html>`_

doxygen info: `User Class <http://llvm.org/doxygen/classllvm_1_1User.html>`_

Superclass: Value_

The ``User`` class is the common base class of all LLVM nodes that may refer to
``Value``\ s.  It exposes a list of "Operands" that are all of the ``Value``\ s
that the User is referring to.  The ``User`` class itself is a subclass of
``Value``.

The operands of a ``User`` point directly to the LLVM ``Value`` that it refers
to.  Because LLVM uses Static Single Assignment (SSA) form, there can only be
one definition referred to, allowing this direct connection.  This connection
provides the use-def information in LLVM.

.. _m_User:

Important Public Members of the ``User`` class
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The ``User`` class exposes the operand list in two ways: through an index access
interface and through an iterator based interface.

* | ``Value *getOperand(unsigned i)``
  | ``unsigned getNumOperands()``

  These two methods expose the operands of the ``User`` in a convenient form for
  direct access.

* | ``User::op_iterator`` - Typedef for iterator over the operand list
  | ``op_iterator op_begin()`` - Get an iterator to the start of the operand
    list.
  | ``op_iterator op_end()`` - Get an iterator to the end of the operand list.

  Together, these methods make up the iterator based interface to the operands
  of a ``User``.


.. _Instruction:

The ``Instruction`` class
-------------------------

``#include "llvm/IR/Instruction.h"``

header source: `Instruction.h
<http://llvm.org/doxygen/Instruction_8h-source.html>`_

doxygen info: `Instruction Class
<http://llvm.org/doxygen/classllvm_1_1Instruction.html>`_

Superclasses: User_, Value_

The ``Instruction`` class is the common base class for all LLVM instructions.
It provides only a few methods, but is a very commonly used class.  The primary
data tracked by the ``Instruction`` class itself is the opcode (instruction
type) and the parent BasicBlock_ the ``Instruction`` is embedded into.  To
represent a specific type of instruction, one of many subclasses of
``Instruction`` are used.

Because the ``Instruction`` class subclasses the User_ class, its operands can
be accessed in the same way as for other ``User``\ s (with the
``getOperand()``/``getNumOperands()`` and ``op_begin()``/``op_end()`` methods).
An important file for the ``Instruction`` class is the ``llvm/Instruction.def``
file.  This file contains some meta-data about the various different types of
instructions in LLVM.  It describes the enum values that are used as opcodes
(for example ``Instruction::Add`` and ``Instruction::ICmp``), as well as the
concrete sub-classes of ``Instruction`` that implement the instruction (for
example BinaryOperator_ and CmpInst_).  Unfortunately, the use of macros in this
file confuses doxygen, so these enum values don't show up correctly in the
`doxygen output <http://llvm.org/doxygen/classllvm_1_1Instruction.html>`_.

.. _s_Instruction:

Important Subclasses of the ``Instruction`` class
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _BinaryOperator:

* ``BinaryOperator``

  This subclasses represents all two operand instructions whose operands must be
  the same type, except for the comparison instructions.

.. _CastInst:

* ``CastInst``
  This subclass is the parent of the 12 casting instructions.  It provides
  common operations on cast instructions.

.. _CmpInst:

* ``CmpInst``

  This subclass respresents the two comparison instructions,
  `ICmpInst <LangRef.html#i_icmp>`_ (integer opreands), and
  `FCmpInst <LangRef.html#i_fcmp>`_ (floating point operands).

.. _TerminatorInst:

* ``TerminatorInst``

  This subclass is the parent of all terminator instructions (those which can
  terminate a block).

.. _m_Instruction:

Important Public Members of the ``Instruction`` class
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* ``BasicBlock *getParent()``

  Returns the BasicBlock_ that this
  ``Instruction`` is embedded into.

* ``bool mayWriteToMemory()``

  Returns true if the instruction writes to memory, i.e. it is a ``call``,
  ``free``, ``invoke``, or ``store``.

* ``unsigned getOpcode()``

  Returns the opcode for the ``Instruction``.

* ``Instruction *clone() const``

  Returns another instance of the specified instruction, identical in all ways
  to the original except that the instruction has no parent (i.e. it's not
  embedded into a BasicBlock_), and it has no name.

.. _Constant:

The ``Constant`` class and subclasses
-------------------------------------

Constant represents a base class for different types of constants.  It is
subclassed by ConstantInt, ConstantArray, etc. for representing the various
types of Constants.  GlobalValue_ is also a subclass, which represents the
address of a global variable or function.

.. _s_Constant:

Important Subclasses of Constant
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* ConstantInt : This subclass of Constant represents an integer constant of
  any width.

  * ``const APInt& getValue() const``: Returns the underlying
    value of this constant, an APInt value.

  * ``int64_t getSExtValue() const``: Converts the underlying APInt value to an
    int64_t via sign extension.  If the value (not the bit width) of the APInt
    is too large to fit in an int64_t, an assertion will result.  For this
    reason, use of this method is discouraged.

  * ``uint64_t getZExtValue() const``: Converts the underlying APInt value
    to a uint64_t via zero extension.  IF the value (not the bit width) of the
    APInt is too large to fit in a uint64_t, an assertion will result.  For this
    reason, use of this method is discouraged.

  * ``static ConstantInt* get(const APInt& Val)``: Returns the ConstantInt
    object that represents the value provided by ``Val``.  The type is implied
    as the IntegerType that corresponds to the bit width of ``Val``.

  * ``static ConstantInt* get(const Type *Ty, uint64_t Val)``: Returns the
    ConstantInt object that represents the value provided by ``Val`` for integer
    type ``Ty``.

* ConstantFP : This class represents a floating point constant.

  * ``double getValue() const``: Returns the underlying value of this constant.

* ConstantArray : This represents a constant array.

  * ``const std::vector<Use> &getValues() const``: Returns a vector of
    component constants that makeup this array.

* ConstantStruct : This represents a constant struct.

  * ``const std::vector<Use> &getValues() const``: Returns a vector of
    component constants that makeup this array.

* GlobalValue : This represents either a global variable or a function.  In
  either case, the value is a constant fixed address (after linking).

.. _GlobalValue:

The ``GlobalValue`` class
-------------------------

``#include "llvm/IR/GlobalValue.h"``

header source: `GlobalValue.h
<http://llvm.org/doxygen/GlobalValue_8h-source.html>`_

doxygen info: `GlobalValue Class
<http://llvm.org/doxygen/classllvm_1_1GlobalValue.html>`_

Superclasses: Constant_, User_, Value_

Global values ( GlobalVariable_\ s or :ref:`Function <c_Function>`\ s) are the
only LLVM values that are visible in the bodies of all :ref:`Function
<c_Function>`\ s.  Because they are visible at global scope, they are also
subject to linking with other globals defined in different translation units.
To control the linking process, ``GlobalValue``\ s know their linkage rules.
Specifically, ``GlobalValue``\ s know whether they have internal or external
linkage, as defined by the ``LinkageTypes`` enumeration.

If a ``GlobalValue`` has internal linkage (equivalent to being ``static`` in C),
it is not visible to code outside the current translation unit, and does not
participate in linking.  If it has external linkage, it is visible to external
code, and does participate in linking.  In addition to linkage information,
``GlobalValue``\ s keep track of which Module_ they are currently part of.

Because ``GlobalValue``\ s are memory objects, they are always referred to by
their **address**.  As such, the Type_ of a global is always a pointer to its
contents.  It is important to remember this when using the ``GetElementPtrInst``
instruction because this pointer must be dereferenced first.  For example, if
you have a ``GlobalVariable`` (a subclass of ``GlobalValue)`` that is an array
of 24 ints, type ``[24 x i32]``, then the ``GlobalVariable`` is a pointer to
that array.  Although the address of the first element of this array and the
value of the ``GlobalVariable`` are the same, they have different types.  The
``GlobalVariable``'s type is ``[24 x i32]``.  The first element's type is
``i32.`` Because of this, accessing a global value requires you to dereference
the pointer with ``GetElementPtrInst`` first, then its elements can be accessed.
This is explained in the `LLVM Language Reference Manual
<LangRef.html#globalvars>`_.

.. _m_GlobalValue:

Important Public Members of the ``GlobalValue`` class
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* | ``bool hasInternalLinkage() const``
  | ``bool hasExternalLinkage() const``
  | ``void setInternalLinkage(bool HasInternalLinkage)``

  These methods manipulate the linkage characteristics of the ``GlobalValue``.

* ``Module *getParent()``

  This returns the Module_ that the
  GlobalValue is currently embedded into.

.. _c_Function:

The ``Function`` class
----------------------

``#include "llvm/IR/Function.h"``

header source: `Function.h <http://llvm.org/doxygen/Function_8h-source.html>`_

doxygen info: `Function Class
<http://llvm.org/doxygen/classllvm_1_1Function.html>`_

Superclasses: GlobalValue_, Constant_, User_, Value_

The ``Function`` class represents a single procedure in LLVM.  It is actually
one of the more complex classes in the LLVM hierarchy because it must keep track
of a large amount of data.  The ``Function`` class keeps track of a list of
BasicBlock_\ s, a list of formal Argument_\ s, and a SymbolTable_.

The list of BasicBlock_\ s is the most commonly used part of ``Function``
objects.  The list imposes an implicit ordering of the blocks in the function,
which indicate how the code will be laid out by the backend.  Additionally, the
first BasicBlock_ is the implicit entry node for the ``Function``.  It is not
legal in LLVM to explicitly branch to this initial block.  There are no implicit
exit nodes, and in fact there may be multiple exit nodes from a single
``Function``.  If the BasicBlock_ list is empty, this indicates that the
``Function`` is actually a function declaration: the actual body of the function
hasn't been linked in yet.

In addition to a list of BasicBlock_\ s, the ``Function`` class also keeps track
of the list of formal Argument_\ s that the function receives.  This container
manages the lifetime of the Argument_ nodes, just like the BasicBlock_ list does
for the BasicBlock_\ s.

The SymbolTable_ is a very rarely used LLVM feature that is only used when you
have to look up a value by name.  Aside from that, the SymbolTable_ is used
internally to make sure that there are not conflicts between the names of
Instruction_\ s, BasicBlock_\ s, or Argument_\ s in the function body.

Note that ``Function`` is a GlobalValue_ and therefore also a Constant_.  The
value of the function is its address (after linking) which is guaranteed to be
constant.

.. _m_Function:

Important Public Members of the ``Function``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* ``Function(const FunctionType *Ty, LinkageTypes Linkage,
  const std::string &N = "", Module* Parent = 0)``

  Constructor used when you need to create new ``Function``\ s to add the
  program.  The constructor must specify the type of the function to create and
  what type of linkage the function should have.  The FunctionType_ argument
  specifies the formal arguments and return value for the function.  The same
  FunctionType_ value can be used to create multiple functions.  The ``Parent``
  argument specifies the Module in which the function is defined.  If this
  argument is provided, the function will automatically be inserted into that
  module's list of functions.

* ``bool isDeclaration()``

  Return whether or not the ``Function`` has a body defined.  If the function is
  "external", it does not have a body, and thus must be resolved by linking with
  a function defined in a different translation unit.

* | ``Function::iterator`` - Typedef for basic block list iterator
  | ``Function::const_iterator`` - Typedef for const_iterator.
  | ``begin()``, ``end()``, ``size()``, ``empty()``

  These are forwarding methods that make it easy to access the contents of a
  ``Function`` object's BasicBlock_ list.

* ``Function::BasicBlockListType &getBasicBlockList()``

  Returns the list of BasicBlock_\ s.  This is necessary to use when you need to
  update the list or perform a complex action that doesn't have a forwarding
  method.

* | ``Function::arg_iterator`` - Typedef for the argument list iterator
  | ``Function::const_arg_iterator`` - Typedef for const_iterator.
  | ``arg_begin()``, ``arg_end()``, ``arg_size()``, ``arg_empty()``

  These are forwarding methods that make it easy to access the contents of a
  ``Function`` object's Argument_ list.

* ``Function::ArgumentListType &getArgumentList()``

  Returns the list of Argument_.  This is necessary to use when you need to
  update the list or perform a complex action that doesn't have a forwarding
  method.

* ``BasicBlock &getEntryBlock()``

  Returns the entry ``BasicBlock`` for the function.  Because the entry block
  for the function is always the first block, this returns the first block of
  the ``Function``.

* | ``Type *getReturnType()``
  | ``FunctionType *getFunctionType()``

  This traverses the Type_ of the ``Function`` and returns the return type of
  the function, or the FunctionType_ of the actual function.

* ``SymbolTable *getSymbolTable()``

  Return a pointer to the SymbolTable_ for this ``Function``.

.. _GlobalVariable:

The ``GlobalVariable`` class
----------------------------

``#include "llvm/IR/GlobalVariable.h"``

header source: `GlobalVariable.h
<http://llvm.org/doxygen/GlobalVariable_8h-source.html>`_

doxygen info: `GlobalVariable Class
<http://llvm.org/doxygen/classllvm_1_1GlobalVariable.html>`_

Superclasses: GlobalValue_, Constant_, User_, Value_

Global variables are represented with the (surprise surprise) ``GlobalVariable``
class.  Like functions, ``GlobalVariable``\ s are also subclasses of
GlobalValue_, and as such are always referenced by their address (global values
must live in memory, so their "name" refers to their constant address).  See
GlobalValue_ for more on this.  Global variables may have an initial value
(which must be a Constant_), and if they have an initializer, they may be marked
as "constant" themselves (indicating that their contents never change at
runtime).

.. _m_GlobalVariable:

Important Public Members of the ``GlobalVariable`` class
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* ``GlobalVariable(const Type *Ty, bool isConstant, LinkageTypes &Linkage,
  Constant *Initializer = 0, const std::string &Name = "", Module* Parent = 0)``

  Create a new global variable of the specified type.  If ``isConstant`` is true
  then the global variable will be marked as unchanging for the program.  The
  Linkage parameter specifies the type of linkage (internal, external, weak,
  linkonce, appending) for the variable.  If the linkage is InternalLinkage,
  WeakAnyLinkage, WeakODRLinkage, LinkOnceAnyLinkage or LinkOnceODRLinkage, then
  the resultant global variable will have internal linkage.  AppendingLinkage
  concatenates together all instances (in different translation units) of the
  variable into a single variable but is only applicable to arrays.  See the
  `LLVM Language Reference <LangRef.html#modulestructure>`_ for further details
  on linkage types.  Optionally an initializer, a name, and the module to put
  the variable into may be specified for the global variable as well.

* ``bool isConstant() const``

  Returns true if this is a global variable that is known not to be modified at
  runtime.

* ``bool hasInitializer()``

  Returns true if this ``GlobalVariable`` has an intializer.

* ``Constant *getInitializer()``

  Returns the initial value for a ``GlobalVariable``.  It is not legal to call
  this method if there is no initializer.

.. _BasicBlock:

The ``BasicBlock`` class
------------------------

``#include "llvm/IR/BasicBlock.h"``

header source: `BasicBlock.h
<http://llvm.org/doxygen/BasicBlock_8h-source.html>`_

doxygen info: `BasicBlock Class
<http://llvm.org/doxygen/classllvm_1_1BasicBlock.html>`_

Superclass: Value_

This class represents a single entry single exit section of the code, commonly
known as a basic block by the compiler community.  The ``BasicBlock`` class
maintains a list of Instruction_\ s, which form the body of the block.  Matching
the language definition, the last element of this list of instructions is always
a terminator instruction (a subclass of the TerminatorInst_ class).

In addition to tracking the list of instructions that make up the block, the
``BasicBlock`` class also keeps track of the :ref:`Function <c_Function>` that
it is embedded into.

Note that ``BasicBlock``\ s themselves are Value_\ s, because they are
referenced by instructions like branches and can go in the switch tables.
``BasicBlock``\ s have type ``label``.

.. _m_BasicBlock:

Important Public Members of the ``BasicBlock`` class
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* ``BasicBlock(const std::string &Name = "", Function *Parent = 0)``

  The ``BasicBlock`` constructor is used to create new basic blocks for
  insertion into a function.  The constructor optionally takes a name for the
  new block, and a :ref:`Function <c_Function>` to insert it into.  If the
  ``Parent`` parameter is specified, the new ``BasicBlock`` is automatically
  inserted at the end of the specified :ref:`Function <c_Function>`, if not
  specified, the BasicBlock must be manually inserted into the :ref:`Function
  <c_Function>`.

* | ``BasicBlock::iterator`` - Typedef for instruction list iterator
  | ``BasicBlock::const_iterator`` - Typedef for const_iterator.
  | ``begin()``, ``end()``, ``front()``, ``back()``,
    ``size()``, ``empty()``
    STL-style functions for accessing the instruction list.

  These methods and typedefs are forwarding functions that have the same
  semantics as the standard library methods of the same names.  These methods
  expose the underlying instruction list of a basic block in a way that is easy
  to manipulate.  To get the full complement of container operations (including
  operations to update the list), you must use the ``getInstList()`` method.

* ``BasicBlock::InstListType &getInstList()``

  This method is used to get access to the underlying container that actually
  holds the Instructions.  This method must be used when there isn't a
  forwarding function in the ``BasicBlock`` class for the operation that you
  would like to perform.  Because there are no forwarding functions for
  "updating" operations, you need to use this if you want to update the contents
  of a ``BasicBlock``.

* ``Function *getParent()``

  Returns a pointer to :ref:`Function <c_Function>` the block is embedded into,
  or a null pointer if it is homeless.

* ``TerminatorInst *getTerminator()``

  Returns a pointer to the terminator instruction that appears at the end of the
  ``BasicBlock``.  If there is no terminator instruction, or if the last
  instruction in the block is not a terminator, then a null pointer is returned.

.. _Argument:

The ``Argument`` class
----------------------

This subclass of Value defines the interface for incoming formal arguments to a
function.  A Function maintains a list of its formal arguments.  An argument has
a pointer to the parent Function.


