===================================
How To Setup Clang Tooling For LLVM
===================================

NOTE: this document applies to the original Clang project, not the DirectX
Compiler. It's made available for informational purposes only.

Clang Tooling provides infrastructure to write tools that need syntactic
and semantic information about a program. This term also relates to a set
of specific tools using this infrastructure (e.g. ``clang-check``). This
document provides information on how to set up and use Clang Tooling for
the LLVM source code.

Introduction
============

Clang Tooling needs a compilation database to figure out specific build
options for each file. Currently it can create a compilation database
from the ``compilation_commands.json`` file, generated by CMake. When
invoking clang tools, you can either specify a path to a build directory
using a command line parameter ``-p`` or let Clang Tooling find this
file in your source tree. In either case you need to configure your
build using CMake to use clang tools.

Setup Clang Tooling Using CMake and Make
========================================

If you intend to use make to build LLVM, you should have CMake 2.8.6 or
later installed (can be found `here <http://cmake.org>`_).

First, you need to generate Makefiles for LLVM with CMake. You need to
make a build directory and run CMake from it:

.. code-block:: console

  $ mkdir your/build/directory
  $ cd your/build/directory
  $ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON path/to/llvm/sources

If you want to use clang instead of GCC, you can add
``-DCMAKE_C_COMPILER=/path/to/clang -DCMAKE_CXX_COMPILER=/path/to/clang++``.
You can also use ``ccmake``, which provides a curses interface to configure
CMake variables for lazy people.

As a result, the new ``compile_commands.json`` file should appear in the
current directory. You should link it to the LLVM source tree so that
Clang Tooling is able to use it:

.. code-block:: console

  $ ln -s $PWD/compile_commands.json path/to/llvm/source/

Now you are ready to build and test LLVM using make:

.. code-block:: console

  $ make check-all

Using Clang Tools
=================

After you completed the previous steps, you are ready to run clang tools. If
you have a recent clang installed, you should have ``clang-check`` in
``$PATH``. Try to run it on any ``.cpp`` file inside the LLVM source tree:

.. code-block:: console

  $ clang-check tools/clang/lib/Tooling/CompilationDatabase.cpp

If you're using vim, it's convenient to have clang-check integrated. Put
this into your ``.vimrc``:

::

    function! ClangCheckImpl(cmd)
      if &autowrite | wall | endif
      echo "Running " . a:cmd . " ..."
      let l:output = system(a:cmd)
      cexpr l:output
      cwindow
      let w:quickfix_title = a:cmd
      if v:shell_error != 0
        cc
      endif
      let g:clang_check_last_cmd = a:cmd
    endfunction

    function! ClangCheck()
      let l:filename = expand('%')
      if l:filename =~ '\.\(cpp\|cxx\|cc\|c\)$'
        call ClangCheckImpl("clang-check " . l:filename)
      elseif exists("g:clang_check_last_cmd")
        call ClangCheckImpl(g:clang_check_last_cmd)
      else
        echo "Can't detect file's compilation arguments and no previous clang-check invocation!"
      endif
    endfunction

    nmap <silent> <F5> :call ClangCheck()<CR><CR>

When editing a .cpp/.cxx/.cc/.c file, hit F5 to reparse the file. In
case the current file has a different extension (for example, .h), F5
will re-run the last clang-check invocation made from this vim instance
(if any). The output will go into the error window, which is opened
automatically when clang-check finds errors, and can be re-opened with
``:cope``.

Other ``clang-check`` options that can be useful when working with clang
AST:

* ``-ast-print`` --- Build ASTs and then pretty-print them.
* ``-ast-dump`` --- Build ASTs and then debug dump them.
* ``-ast-dump-filter=<string>`` --- Use with ``-ast-dump`` or ``-ast-print`` to
  dump/print only AST declaration nodes having a certain substring in a
  qualified name. Use ``-ast-list`` to list all filterable declaration node
  names.
* ``-ast-list`` --- Build ASTs and print the list of declaration node qualified
  names.

Examples:

.. code-block:: console

  $ clang-check tools/clang/tools/clang-check/ClangCheck.cpp -ast-dump -ast-dump-filter ActionFactory::newASTConsumer
  Processing: tools/clang/tools/clang-check/ClangCheck.cpp.
  Dumping ::ActionFactory::newASTConsumer:
  clang::ASTConsumer *newASTConsumer() (CompoundStmt 0x44da290 </home/alexfh/local/llvm/tools/clang/tools/clang-check/ClangCheck.cpp:64:40, line:72:3>
    (IfStmt 0x44d97c8 <line:65:5, line:66:45>
      <<<NULL>>>
        (ImplicitCastExpr 0x44d96d0 <line:65:9> '_Bool':'_Bool' <UserDefinedConversion>
  ...
  $ clang-check tools/clang/tools/clang-check/ClangCheck.cpp -ast-print -ast-dump-filter ActionFactory::newASTConsumer
  Processing: tools/clang/tools/clang-check/ClangCheck.cpp.
  Printing <anonymous namespace>::ActionFactory::newASTConsumer:
  clang::ASTConsumer *newASTConsumer() {
      if (this->ASTList.operator _Bool())
          return clang::CreateASTDeclNodeLister();
      if (this->ASTDump.operator _Bool())
          return clang::CreateASTDumper(this->ASTDumpFilter);
      if (this->ASTPrint.operator _Bool())
          return clang::CreateASTPrinter(&llvm::outs(), this->ASTDumpFilter);
      return new clang::ASTConsumer();
  }

(Experimental) Using Ninja Build System
=======================================

Optionally you can use the `Ninja <https://github.com/martine/ninja>`_
build system instead of make. It is aimed at making your builds faster.
Currently this step will require building Ninja from sources.

To take advantage of using Clang Tools along with Ninja build you need
at least CMake 2.8.9.

Clone the Ninja git repository and build Ninja from sources:

.. code-block:: console

  $ git clone git://github.com/martine/ninja.git
  $ cd ninja/
  $ ./bootstrap.py

This will result in a single binary ``ninja`` in the current directory.
It doesn't require installation and can just be copied to any location
inside ``$PATH``, say ``/usr/local/bin/``:

.. code-block:: console

  $ sudo cp ninja /usr/local/bin/
  $ sudo chmod a+rx /usr/local/bin/ninja

After doing all of this, you'll need to generate Ninja build files for
LLVM with CMake. You need to make a build directory and run CMake from
it:

.. code-block:: console

  $ mkdir your/build/directory
  $ cd your/build/directory
  $ cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON path/to/llvm/sources

If you want to use clang instead of GCC, you can add
``-DCMAKE_C_COMPILER=/path/to/clang -DCMAKE_CXX_COMPILER=/path/to/clang++``.
You can also use ``ccmake``, which provides a curses interface to configure
CMake variables in an interactive manner.

As a result, the new ``compile_commands.json`` file should appear in the
current directory. You should link it to the LLVM source tree so that
Clang Tooling is able to use it:

.. code-block:: console

  $ ln -s $PWD/compile_commands.json path/to/llvm/source/

Now you are ready to build and test LLVM using Ninja:

.. code-block:: console

  $ ninja check-all

Other target names can be used in the same way as with make.

