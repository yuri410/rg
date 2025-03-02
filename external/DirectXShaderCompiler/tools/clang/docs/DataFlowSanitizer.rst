=================
DataFlowSanitizer
=================

.. toctree::
   :hidden:

   DataFlowSanitizerDesign

.. contents::
   :local:

Introduction
============

NOTE: this document applies to the original Clang project, not the DirectX
Compiler. It's made available for informational purposes only.

DataFlowSanitizer is a generalised dynamic data flow analysis.

Unlike other Sanitizer tools, this tool is not designed to detect a
specific class of bugs on its own.  Instead, it provides a generic
dynamic data flow analysis framework to be used by clients to help
detect application-specific issues within their own code.

Usage
=====

With no program changes, applying DataFlowSanitizer to a program
will not alter its behavior.  To use DataFlowSanitizer, the program
uses API functions to apply tags to data to cause it to be tracked, and to
check the tag of a specific data item.  DataFlowSanitizer manages
the propagation of tags through the program according to its data flow.

The APIs are defined in the header file ``sanitizer/dfsan_interface.h``.
For further information about each function, please refer to the header
file.

ABI List
--------

DataFlowSanitizer uses a list of functions known as an ABI list to decide
whether a call to a specific function should use the operating system's native
ABI or whether it should use a variant of this ABI that also propagates labels
through function parameters and return values.  The ABI list file also controls
how labels are propagated in the former case.  DataFlowSanitizer comes with a
default ABI list which is intended to eventually cover the glibc library on
Linux but it may become necessary for users to extend the ABI list in cases
where a particular library or function cannot be instrumented (e.g. because
it is implemented in assembly or another language which DataFlowSanitizer does
not support) or a function is called from a library or function which cannot
be instrumented.

DataFlowSanitizer's ABI list file is a :doc:`SanitizerSpecialCaseList`.
The pass treats every function in the ``uninstrumented`` category in the
ABI list file as conforming to the native ABI.  Unless the ABI list contains
additional categories for those functions, a call to one of those functions
will produce a warning message, as the labelling behavior of the function
is unknown.  The other supported categories are ``discard``, ``functional``
and ``custom``.

* ``discard`` -- To the extent that this function writes to (user-accessible)
  memory, it also updates labels in shadow memory (this condition is trivially
  satisfied for functions which do not write to user-accessible memory).  Its
  return value is unlabelled.
* ``functional`` -- Like ``discard``, except that the label of its return value
  is the union of the label of its arguments.
* ``custom`` -- Instead of calling the function, a custom wrapper ``__dfsw_F``
  is called, where ``F`` is the name of the function.  This function may wrap
  the original function or provide its own implementation.  This category is
  generally used for uninstrumentable functions which write to user-accessible
  memory or which have more complex label propagation behavior.  The signature
  of ``__dfsw_F`` is based on that of ``F`` with each argument having a
  label of type ``dfsan_label`` appended to the argument list.  If ``F``
  is of non-void return type a final argument of type ``dfsan_label *``
  is appended to which the custom function can store the label for the
  return value.  For example:

.. code-block:: c++

  void f(int x);
  void __dfsw_f(int x, dfsan_label x_label);

  void *memcpy(void *dest, const void *src, size_t n);
  void *__dfsw_memcpy(void *dest, const void *src, size_t n,
                      dfsan_label dest_label, dfsan_label src_label,
                      dfsan_label n_label, dfsan_label *ret_label);

If a function defined in the translation unit being compiled belongs to the
``uninstrumented`` category, it will be compiled so as to conform to the
native ABI.  Its arguments will be assumed to be unlabelled, but it will
propagate labels in shadow memory.

For example:

.. code-block:: none

  # main is called by the C runtime using the native ABI.
  fun:main=uninstrumented
  fun:main=discard

  # malloc only writes to its internal data structures, not user-accessible memory.
  fun:malloc=uninstrumented
  fun:malloc=discard

  # tolower is a pure function.
  fun:tolower=uninstrumented
  fun:tolower=functional

  # memcpy needs to copy the shadow from the source to the destination region.
  # This is done in a custom function.
  fun:memcpy=uninstrumented
  fun:memcpy=custom

Example
=======

The following program demonstrates label propagation by checking that
the correct labels are propagated.

.. code-block:: c++

  #include <sanitizer/dfsan_interface.h>
  #include <assert.h>

  int main(void) {
    int i = 1;
    dfsan_label i_label = dfsan_create_label("i", 0);
    dfsan_set_label(i_label, &i, sizeof(i));

    int j = 2;
    dfsan_label j_label = dfsan_create_label("j", 0);
    dfsan_set_label(j_label, &j, sizeof(j));

    int k = 3;
    dfsan_label k_label = dfsan_create_label("k", 0);
    dfsan_set_label(k_label, &k, sizeof(k));

    dfsan_label ij_label = dfsan_get_label(i + j);
    assert(dfsan_has_label(ij_label, i_label));
    assert(dfsan_has_label(ij_label, j_label));
    assert(!dfsan_has_label(ij_label, k_label));

    dfsan_label ijk_label = dfsan_get_label(i + j + k);
    assert(dfsan_has_label(ijk_label, i_label));
    assert(dfsan_has_label(ijk_label, j_label));
    assert(dfsan_has_label(ijk_label, k_label));

    return 0;
  }

Current status
==============

DataFlowSanitizer is a work in progress, currently under development for
x86\_64 Linux.

Design
======

Please refer to the :doc:`design document<DataFlowSanitizerDesign>`.
