===========================================
Clang |release| |ReleaseNotesTitle|
===========================================

.. contents::
   :local:
   :depth: 2

Written by the `LLVM Team <https://llvm.org/>`_

.. only:: PreRelease

  .. warning::
     These are in-progress notes for the upcoming Clang |version| release.
     Release notes for previous releases can be found on
     `the Releases Page <https://llvm.org/releases/>`_.

Introduction
============

This document contains the release notes for the Clang C/C++/Objective-C
frontend, part of the LLVM Compiler Infrastructure, release |release|. Here we
describe the status of Clang in some detail, including major
improvements from the previous release and new feature work. For the
general LLVM release notes, see `the LLVM
documentation <https://llvm.org/docs/ReleaseNotes.html>`_. For the libc++ release notes,
see `this page <https://libcxx.llvm.org/ReleaseNotes.html>`_. All LLVM releases
may be downloaded from the `LLVM releases web site <https://llvm.org/releases/>`_.

For more information about Clang or LLVM, including information about the
latest release, please see the `Clang Web Site <https://clang.llvm.org>`_ or the
`LLVM Web Site <https://llvm.org>`_.

Potentially Breaking Changes
============================
These changes are ones which we think may surprise users when upgrading to
Clang |release| because of the opportunity they pose for disruption to existing
code bases.

- Fix a bug in reversed argument for templated operators.
  This breaks code in C++20 which was previously accepted in C++17.
  Clang did not properly diagnose such casese in C++20 before this change. Eg:

  .. code-block:: cpp

    struct P {};
    template<class S> bool operator==(const P&, const S&);

    struct A : public P {};
    struct B : public P {};

    // This equality is now ambiguous in C++20.
    bool ambiguous(A a, B b) { return a == b; }

    template<class S> bool operator!=(const P&, const S&);
    // Ok. Found a matching operator!=.
    bool fine(A a, B b) { return a == b; }

  To reduce such widespread breakages, as an extension, Clang accepts this code
  with an existing warning ``-Wambiguous-reversed-operator`` warning.
  Fixes `#53954 <https://github.com/llvm/llvm-project/issues/53954>`_.

- The CMake variable ``GCC_INSTALL_PREFIX`` (which sets the default
  ``--gcc-toolchain=``) is deprecated and will be removed. Specify
  ``--gcc-install-dir=`` or ``--gcc-triple=`` in a `configuration file
  <https://clang.llvm.org/docs/UsersManual.html#configuration-files>`_ as a
  replacement.
  (`#77537 <https://github.com/llvm/llvm-project/pull/77537>`_)

C/C++ Language Potentially Breaking Changes
-------------------------------------------

- The default extension name for PCH generation (``-c -xc-header`` and ``-c
  -xc++-header``) is now ``.pch`` instead of ``.gch``.
- ``-include a.h`` probing ``a.h.gch`` will now ignore ``a.h.gch`` if it is not
  a clang pch file or a directory containing any clang pch file.
- Fixed a bug that caused ``__has_cpp_attribute`` and ``__has_c_attribute``
  return incorrect values for some C++-11-style attributes. Below is a complete
  list of behavior changes.

  .. csv-table::
    :header: Test, Old value, New value

    ``__has_cpp_attribute(unused)``,                    201603, 0
    ``__has_cpp_attribute(gnu::unused)``,               201603, 1
    ``__has_c_attribute(unused)``,                      202106, 0
    ``__has_cpp_attribute(clang::fallthrough)``,        201603, 1
    ``__has_cpp_attribute(gnu::fallthrough)``,          201603, 1
    ``__has_c_attribute(gnu::fallthrough)``,            201910, 1
    ``__has_cpp_attribute(warn_unused_result)``,        201907, 0
    ``__has_cpp_attribute(clang::warn_unused_result)``, 201907, 1
    ``__has_cpp_attribute(gnu::warn_unused_result)``,   201907, 1
    ``__has_c_attribute(warn_unused_result)``,          202003, 0
    ``__has_c_attribute(gnu::warn_unused_result)``,     202003, 1

- Fixed a bug in finding matching `operator!=` while adding reversed `operator==` as
  outlined in "The Equality Operator You Are Looking For" (`P2468 <http://wg21.link/p2468r2>`_).
  Fixes (`#68901 <https://github.com/llvm/llvm-project/issues/68901>`_).

C++ Specific Potentially Breaking Changes
-----------------------------------------
- The name mangling rules for function templates has been changed to take into
  account the possibility that functions could be overloaded on their template
  parameter lists or requires-clauses. This causes mangled names to change for
  function templates in the following cases:

  - When a template parameter in a function template depends on a previous
    template parameter, such as ``template<typename T, T V> void f()``.
  - When the function has any constraints, whether from constrained template
      parameters or requires-clauses.
  - When the template parameter list includes a deduced type -- either
      ``auto``, ``decltype(auto)``, or a deduced class template specialization
      type.
  - When a template template parameter is given a template template argument
      that has a different template parameter list.

  This fixes a number of issues where valid programs would be rejected due to
  mangling collisions, or would in some cases be silently miscompiled. Clang
  will use the old manglings if ``-fclang-abi-compat=17`` or lower is
  specified.
  (`#48216 <https://github.com/llvm/llvm-project/issues/48216>`_),
  (`#49884 <https://github.com/llvm/llvm-project/issues/49884>`_), and
  (`#61273 <https://github.com/llvm/llvm-project/issues/61273>`_)

- The `ClassScopeFunctionSpecializationDecl` AST node has been removed.
  Dependent class scope explicit function template specializations now use
  `DependentFunctionTemplateSpecializationInfo` to store candidate primary
  templates and explicit template arguments. This should not impact users of
  Clang as a compiler, but it may break assumptions in Clang-based tools
  iterating over the AST.

- The warning `-Wenum-constexpr-conversion` is now also enabled by default on
  system headers and macros. It will be turned into a hard (non-downgradable)
  error in the next Clang release.

- The flag `-fdelayed-template-parsing` won't be enabled by default with C++20
  when targetting MSVC to match the behavior of MSVC.
  (`MSVC Docs <https://learn.microsoft.com/en-us/cpp/build/reference/permissive-standards-conformance?view=msvc-170>`_)

- Remove the hardcoded path to the imported modules for C++20 named modules. Now we
  require all the dependent modules to specified from the command line.
  See (`#62707 <https://github.com/llvm/llvm-project/issues/62707>`_).

- Forbid `import XXX;` in C++ to find module `XXX` comes from explicit clang modules.
  See (`#64755 <https://github.com/llvm/llvm-project/issues/64755>`_).

ABI Changes in This Version
---------------------------

AST Dumping Potentially Breaking Changes
----------------------------------------
- When dumping a sugared type, Clang will no longer print the desugared type if
  its textual representation is the same as the sugared one. This applies to
  both text dumps of the form ``'foo':'foo'`` which will now be dumped as just
  ``'foo'``, and JSON dumps of the form:

  .. code-block:: json

    "type": {
      "qualType": "foo",
      "desugaredQualType": "foo"
    }

  which will now be dumped as just:

  .. code-block:: json

    "type": {
      "qualType": "foo"
    }

Clang Frontend Potentially Breaking Changes
-------------------------------------------
- Target OS macros extension
  A new Clang extension (see :ref:`here <target_os_detail>`) is enabled for
  Darwin (Apple platform) targets. Clang now defines ``TARGET_OS_*`` macros for
  these targets, which could break existing code bases with improper checks for
  the ``TARGET_OS_`` macros. For example, existing checks might fail to include
  the ``TargetConditionals.h`` header from Apple SDKs and therefore leaving the
  macros undefined and guarded code unexercised.

  Affected code should be checked to see if it's still intended for the specific
  target and fixed accordingly.

  The extension can be turned off by the option ``-fno-define-target-os-macros``
  as a workaround.

What's New in Clang |release|?
==============================
Some of the major new features and improvements to Clang are listed
here. Generic improvements to Clang as a whole or to its underlying
infrastructure are described first, followed by language-specific
sections with improvements to Clang's support for those languages.

C++ Language Changes
--------------------

C++20 Feature Support
^^^^^^^^^^^^^^^^^^^^^
- Implemented `P1907R1 <https://wg21.link/P1907R1>`_ which extends allowed non-type template argument
  kinds with e.g. floating point values and pointers and references to subobjects.
  This feature is still experimental. Accordingly, ``__cpp_nontype_template_args`` was not updated.
  However, its support can be tested with ``__has_extension(cxx_generalized_nttp)``.

- Clang won't perform ODR checks for decls in the global module fragment any
  more to ease the implementation and improve the user's using experience.
  This follows the MSVC's behavior. Users interested in testing the more strict
  behavior can use the flag '-Xclang -fno-skip-odr-check-in-gmf'.
  (`#79240 <https://github.com/llvm/llvm-project/issues/79240>`_).

- Clang won't perform ODR checks for decls in the global module fragment any
  more to ease the implementation and improve the user's using experience.
  This follows the MSVC's behavior.
  (`#79240 <https://github.com/llvm/llvm-project/issues/79240>`_).

C++23 Feature Support
^^^^^^^^^^^^^^^^^^^^^

- Implemented `P2718R0: Lifetime extension in range-based for loops <https://wg21.link/P2718R0>`_. Also
  materialize temporary object which is a prvalue in discarded-value expression.

C++2c Feature Support
^^^^^^^^^^^^^^^^^^^^^

- Implemented `P2662R3 Pack Indexing <https://wg21.link/P2662R3>`_.


Resolutions to C++ Defect Reports
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- Substitute template parameter pack, when it is not explicitly specified
  in the template parameters, but is deduced from a previous argument.
  (`#78449: <https://github.com/llvm/llvm-project/issues/78449>`_).

- Implemented `CWG2598 <https://wg21.link/CWG2598>`_ and `CWG2096 <https://wg21.link/CWG2096>`_,
  making unions (that have either no members or at least one literal member) literal types.
  (`#77924 <https://github.com/llvm/llvm-project/issues/77924>`_).


C Language Changes
------------------

C23 Feature Support
^^^^^^^^^^^^^^^^^^^

Non-comprehensive list of changes in this release
-------------------------------------------------

* Clang now has a ``__builtin_vectorelements()`` function that determines the number of elements in a vector.
  For fixed-sized vectors, e.g., defined via ``__attribute__((vector_size(N)))`` or ARM NEON's vector types
  (e.g., ``uint16x8_t``), this returns the constant number of elements at compile-time.
  For scalable vectors, e.g., SVE or RISC-V V, the number of elements is not known at compile-time and is
  determined at runtime.
* The ``__datasizeof`` keyword has been added. It is similar to ``sizeof``
  except that it returns the size of a type ignoring tail padding.
* ``__builtin_classify_type()`` now classifies ``_BitInt`` values as the return value ``18``
  and vector types as return value ``19``, to match GCC 14's behavior.
* The default value of `_MSC_VER` was raised from 1920 to 1933.
* Since MSVC 19.33 added undocumented attribute ``[[msvc::constexpr]]``, this release adds the attribute as well.

* Added ``#pragma clang fp reciprocal``.

* The version of Unicode used by Clang (primarily to parse identifiers) has been updated to 15.1.

* Clang now defines macro ``__LLVM_INSTR_PROFILE_GENERATE`` when compiling with
  PGO instrumentation profile generation, and ``__LLVM_INSTR_PROFILE_USE`` when
  compiling with PGO profile use.

New Compiler Flags
------------------

* ``-fverify-intermediate-code`` and its complement ``-fno-verify-intermediate-code``.
  Enables or disables verification of the generated LLVM IR.
  Users can pass this to turn on extra verification to catch certain types of
  compiler bugs at the cost of extra compile time.
  Since enabling the verifier adds a non-trivial cost of a few percent impact on
  build times, it's disabled by default, unless your LLVM distribution itself is
  compiled with runtime checks enabled.
* ``-fkeep-system-includes`` modifies the behavior of the ``-E`` option,
  preserving ``#include`` directives for "system" headers instead of copying
  the preprocessed text to the output. This can greatly reduce the size of the
  preprocessed output, which can be helpful when trying to reduce a test case.
* ``-fassume-nothrow-exception-dtor`` is added to assume that the destructor of
  a thrown exception object will not throw. The generated code for catch
  handlers will be smaller. A throw expression of a type with a
  potentially-throwing destructor will lead to an error.

* ``-fopenacc`` was added as a part of the effort to support OpenACC in Clang.

* ``-fcx-limited-range`` enables the naive mathematical formulas for complex
  division and multiplication with no NaN checking of results. The default is
  ``-fno-cx-limited-range``, but this option is enabled by ``-ffast-math``.

* ``-fcx-fortran-rules`` enables the naive mathematical formulas for complex
  multiplication and enables application of Smith's algorithm for complex
  division. See SMITH, R. L. Algorithm 116: Complex division. Commun. ACM 5, 8
  (1962). The default is ``-fno-cx-fortran-rules``.

* ``-fvisibility-global-new-delete=<value>`` gives more freedom to users to
  control how and if Clang forces a visibility for the replaceable new and
  delete declarations. The option takes 4 values: ``force-hidden``,
  ``force-protected``, ``force-default`` and ``source``; ``force-default`` is
  the default. Option values with prefix ``force-`` assign such declarations
  an implicit visibility attribute with the corresponding visibility. An option
  value of ``source`` implies that no implicit attribute is added. Without the
  attribute the replaceable global new and delete operators behave normally
  (like other functions) with respect to visibility attributes, pragmas and
  options (e.g ``--fvisibility=``).
* Full register names can be used when printing assembly via ``-mregnames``.
  This option now matches the one used by GCC.

.. _target_os_detail:

* ``-fdefine-target-os-macros`` and its complement
  ``-fno-define-target-os-macros``. Enables or disables the Clang extension to
  provide built-in definitions of a list of ``TARGET_OS_*`` macros based on the
  target triple.

  The extension is enabled by default for Darwin (Apple platform) targets.

Deprecated Compiler Flags
-------------------------

Modified Compiler Flags
-----------------------

* ``-Woverriding-t-option`` is renamed to ``-Woverriding-option``.
* ``-Winterrupt-service-routine`` is renamed to ``-Wexcessive-regsave`` as a generalization
* ``-frewrite-includes`` now guards the original #include directives with
  ``__CLANG_REWRITTEN_INCLUDES``, and ``__CLANG_REWRITTEN_SYSTEM_INCLUDES`` as
  appropriate.
* Introducing a new default calling convention for ``-fdefault-calling-conv``:
  ``rtdcall``. This new default CC only works for M68k and will use the new
  ``m68k_rtdcc`` CC on every functions that are not variadic. The ``-mrtd``
  driver/frontend flag has the same effect when targeting M68k.
* ``-fvisibility-global-new-delete-hidden`` is now a deprecated spelling of
  ``-fvisibility-global-new-delete=force-hidden`` (``-fvisibility-global-new-delete=``
  is new in this release).
* ``-fprofile-update`` is enabled for ``-fprofile-generate``.

Removed Compiler Flags
-------------------------

Attribute Changes in Clang
--------------------------
- On X86, a warning is now emitted if a function with ``__attribute__((no_caller_saved_registers))``
  calls a function without ``__attribute__((no_caller_saved_registers))``, and is not compiled with
  ``-mgeneral-regs-only``
- On X86, a function with ``__attribute__((interrupt))`` can now call a function without
  ``__attribute__((no_caller_saved_registers))`` provided that it is compiled with ``-mgeneral-regs-only``

- When a non-variadic function is decorated with the ``format`` attribute,
  Clang now checks that the format string would match the function's parameters'
  types after default argument promotion. As a result, it's no longer an
  automatic diagnostic to use parameters of types that the format style
  supports but that are never the result of default argument promotion, such as
  ``float``. (`#59824 <https://github.com/llvm/llvm-project/issues/59824>`_)

- Clang now supports ``[[clang::preferred_type(type-name)]]`` as an attribute
  which can be applied to a bit-field. This attribute helps to map a bit-field
  back to a particular type that may be better-suited to representing the bit-
  field but cannot be used for other reasons and will impact the debug
  information generated for the bit-field. This is most useful when mapping a
  bit-field of basic integer type back to a ``bool`` or an enumeration type,
  e.g.,

  .. code-block:: c++

      enum E { Apple, Orange, Pear };
      struct S {
        [[clang::preferred_type(E)]] unsigned FruitKind : 2;
      };

  When viewing ``S::FruitKind`` in a debugger, it will behave as if the member
  was declared as type ``E`` rather than ``unsigned``.

- Clang now warns you that the ``_Alignas`` attribute on declaration specifiers
  is ignored, changed from the former incorrect suggestion to move it past
  declaration specifiers. (`#58637 <https://github.com/llvm/llvm-project/issues/58637>`_)

- Clang now introduced ``[[clang::coro_only_destroy_when_complete]]`` attribute
  to reduce the size of the destroy functions for coroutines which are known to
  be destroyed after having reached the final suspend point.

- Clang now introduced ``[[clang::coro_return_type]]`` and ``[[clang::coro_wrapper]]``
  attributes. A function returning a type marked with ``[[clang::coro_return_type]]``
  should be a coroutine. A non-coroutine function marked with ``[[clang::coro_wrapper]]``
  is still allowed to return the such a type. This is helpful for analyzers to recognize coroutines from the function signatures.

- Clang now supports ``[[clang::code_align(N)]]`` as an attribute which can be
  applied to a loop and specifies the byte alignment for a loop. This attribute
  accepts a positive integer constant initialization expression indicating the
  number of bytes for the minimum alignment boundary. Its value must be a power
  of 2, between 1 and 4096(inclusive).

  .. code-block:: c++

      void Array(int *array, size_t n) {
        [[clang::code_align(64)]] for (int i = 0; i < n; ++i) array[i] = 0;
      }

      template<int A>
      void func() {
        [[clang::code_align(A)]] for(;;) { }
      }

- Clang now introduced ``[[clang::coro_lifetimebound]]`` attribute.
  All parameters of a function are considered to be lifetime bound if the function
  returns a type annotated with ``[[clang::coro_lifetimebound]]`` and ``[[clang::coro_return_type]]``.
  This analysis can be disabled for a function by annotating the function with ``[[clang::coro_disable_lifetimebound]]``.

Improvements to Clang's diagnostics
-----------------------------------
- Clang constexpr evaluator now prints template arguments when displaying
  template-specialization function calls.
- Clang contexpr evaluator now displays notes as well as an error when a constructor
  of a base class is not called in the constructor of its derived class.
- Clang no longer emits ``-Wmissing-variable-declarations`` for variables declared
  with the ``register`` storage class.
- Clang's ``-Wswitch-default`` flag now diagnoses whenever a ``switch`` statement
  does not have a ``default`` label.
- Clang's ``-Wtautological-negation-compare`` flag now diagnoses logical
  tautologies like ``x && !x`` and ``!x || x`` in expressions. This also
  makes ``-Winfinite-recursion`` diagnose more cases.
  (`#56035 <https://github.com/llvm/llvm-project/issues/56035>`_).
- Clang constexpr evaluator now diagnoses compound assignment operators against
  uninitialized variables as a read of uninitialized object.
  (`#51536 <https://github.com/llvm/llvm-project/issues/51536>`_)
- Clang's ``-Wformat-truncation`` now diagnoses ``snprintf`` call that is known to
  result in string truncation.
  (`#64871 <https://github.com/llvm/llvm-project/issues/64871>`_).
  Existing warnings that similarly warn about the overflow in ``sprintf``
  now falls under its own warning group ```-Wformat-overflow`` so that it can
  be disabled separately from ``Wfortify-source``.
  These two new warning groups have subgroups ``-Wformat-truncation-non-kprintf``
  and ``-Wformat-overflow-non-kprintf``, respectively. These subgroups are used when
  the format string contains ``%p`` format specifier.
  Because Linux kernel's codebase has format extensions for ``%p``, kernel developers
  are encouraged to disable these two subgroups by setting ``-Wno-format-truncation-non-kprintf``
  and ``-Wno-format-overflow-non-kprintf`` in order to avoid false positives on
  the kernel codebase.
  Also clang no longer emits false positive warnings about the output length of
  ``%g`` format specifier and about ``%o, %x, %X`` with ``#`` flag.
- Clang now emits ``-Wcast-qual`` for functional-style cast expressions.
- Clang no longer emits irrelevant notes about unsatisfied constraint expressions
  on the left-hand side of ``||`` when the right-hand side constraint is satisfied.
  (`#54678 <https://github.com/llvm/llvm-project/issues/54678>`_).
- Clang now prints its 'note' diagnostic in cyan instead of black, to be more compatible
  with terminals with dark background colors. This is also more consistent with GCC.
- Clang now displays an improved diagnostic and a note when a defaulted special
  member is marked ``constexpr`` in a class with a virtual base class
  (`#64843 <https://github.com/llvm/llvm-project/issues/64843>`_).
- ``-Wfixed-enum-extension`` and ``-Wmicrosoft-fixed-enum`` diagnostics are no longer
  emitted when building as C23, since C23 standardizes support for enums with a
  fixed underlying type.
- When describing the failure of static assertion of `==` expression, clang prints the integer
  representation of the value as well as its character representation when
  the user-provided expression is of character type. If the character is
  non-printable, clang now shows the escpaed character.
  Clang also prints multi-byte characters if the user-provided expression
  is of multi-byte character type.

  *Example Code*:

  .. code-block:: c++

     static_assert("A\n"[1] == U'🌍');

  *BEFORE*:

  .. code-block:: text

    source:1:15: error: static assertion failed due to requirement '"A\n"[1] == U'\U0001f30d''
    1 | static_assert("A\n"[1] == U'🌍');
      |               ^~~~~~~~~~~~~~~~~
    source:1:24: note: expression evaluates to ''
    ' == 127757'
    1 | static_assert("A\n"[1] == U'🌍');
      |               ~~~~~~~~~^~~~~~~~

  *AFTER*:

  .. code-block:: text

    source:1:15: error: static assertion failed due to requirement '"A\n"[1] == U'\U0001f30d''
    1 | static_assert("A\n"[1] == U'🌍');
      |               ^~~~~~~~~~~~~~~~~
    source:1:24: note: expression evaluates to ''\n' (0x0A, 10) == U'🌍' (0x1F30D, 127757)'
    1 | static_assert("A\n"[1] == U'🌍');
      |               ~~~~~~~~~^~~~~~~~
- Clang now always diagnoses when using non-standard layout types in ``offsetof`` .
  (`#64619 <https://github.com/llvm/llvm-project/issues/64619>`_)
- Clang now diagnoses redefined defaulted constructor when redefined
  defaulted constructor with different exception specs.
  (`#69094 <https://github.com/llvm/llvm-project/issues/69094>`_)
- Clang now diagnoses use of variable-length arrays in C++ by default (and
  under ``-Wall`` in GNU++ mode). This is an extension supported by Clang and
  GCC, but is very easy to accidentally use without realizing it's a
  nonportable construct that has different semantics from a constant-sized
  array. (`#62836 <https://github.com/llvm/llvm-project/issues/62836>`_)

- Clang changed the order in which it displays candidate functions on overloading failures.
  Previously, Clang used definition of ordering from the C++ Standard. The order defined in
  the Standard is partial and is not suited for sorting. Instead, Clang now uses a strict
  order that still attempts to push more relevant functions to the top by comparing their
  corresponding conversions. In some cases, this results in better order. E.g., for the
  following code

  .. code-block:: cpp

      struct Foo {
        operator int();
        operator const char*();
      };

      void test() { Foo() - Foo(); }

  Clang now produces a list with two most relevant builtin operators at the top,
  i.e. ``operator-(int, int)`` and ``operator-(const char*, const char*)``.
  Previously ``operator-(const char*, const char*)`` was the first element,
  but ``operator-(int, int)`` was only the 13th element in the output.
  However, new implementation does not take into account some aspects of
  C++ semantics, e.g. which function template is more specialized. This
  can sometimes lead to worse ordering.


- When describing a warning/error in a function-style type conversion Clang underlines only until
  the end of the expression we convert from. Now Clang underlines until the closing parenthesis.

  Before:

  .. code-block:: text

    warning: cast from 'long (*)(const int &)' to 'decltype(fun_ptr)' (aka 'long (*)(int &)') converts to incompatible function type [-Wcast-function-type-strict]
    24 | return decltype(fun_ptr)( f_ptr /*comment*/);
       |        ^~~~~~~~~~~~~~~~~~~~~~~~

  After:

  .. code-block:: text

    warning: cast from 'long (*)(const int &)' to 'decltype(fun_ptr)' (aka 'long (*)(int &)') converts to incompatible function type [-Wcast-function-type-strict]
    24 | return decltype(fun_ptr)( f_ptr /*comment*/);
       |        ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- ``-Wzero-as-null-pointer-constant`` diagnostic is no longer emitted when using ``__null``
  (or, more commonly, ``NULL`` when the platform defines it as ``__null``) to be more consistent
  with GCC.
- Clang will warn on deprecated specializations used in system headers when their instantiation
  is caused by user code.
- Clang will now print ``static_assert`` failure details for arithmetic binary operators.
  Example:

  .. code-block:: cpp

    static_assert(1 << 4 == 15);

  will now print:

  .. code-block:: text

    error: static assertion failed due to requirement '1 << 4 == 15'
       48 | static_assert(1 << 4 == 15);
          |               ^~~~~~~~~~~~
    note: expression evaluates to '16 == 15'
       48 | static_assert(1 << 4 == 15);
          |               ~~~~~~~^~~~~

- Clang now diagnoses definitions of friend function specializations, e.g. ``friend void f<>(int) {}``.
- Clang now diagnoses narrowing conversions involving const references.
  (`#63151 <https://github.com/llvm/llvm-project/issues/63151>`_).
- Clang now diagnoses unexpanded packs within the template argument lists of function template specializations.
- The warning `-Wnan-infinity-disabled` is now emitted when ``INFINITY``
  or ``NAN`` are used in arithmetic operations or function arguments in
  floating-point mode where ``INFINITY`` or ``NAN`` don't have the expected
  values.

- Clang now diagnoses attempts to bind a bitfield to an NTTP of a reference type as erroneous
  converted constant expression and not as a reference to subobject.
- Clang now diagnoses ``auto`` and ``decltype(auto)`` in declarations of conversion function template
  (`CWG1878 <https://cplusplus.github.io/CWG/issues/1878.html>`_)
- Clang now diagnoses the requirement that non-template friend declarations with requires clauses
  and template friend declarations with a constraint that depends on a template parameter from an
  enclosing template must be a definition.
- Clang now diagnoses incorrect usage of ``const`` and ``pure`` attributes, so ``-Wignored-attributes`` diagnoses more cases.
- Clang now emits more descriptive diagnostics for 'unusual' expressions (e.g. incomplete index
  expressions on matrix types or builtin functions without an argument list) as placement-args
  to new-expressions.

  Before:

  .. code-block:: text

    error: no matching function for call to 'operator new'
       13 |     new (__builtin_memset) S {};
          |     ^   ~~~~~~~~~~~~~~~~~~

    note: candidate function not viable: no known conversion from '<builtin fn type>' to 'int' for 2nd argument
        5 |     void* operator new(__SIZE_TYPE__, int);
          |           ^

  After:

  .. code-block:: text

    error: builtin functions must be directly called
       13 |     new (__builtin_memset) S {};
          |          ^

- Clang now diagnoses import before module declarations but not in global
  module fragment.
  (`#67627 <https://github.com/llvm/llvm-project/issues/67627>`_).

- Clang now diagnoses include headers with angle in module purviews, which is
  not usually intended.
  (`#68615 <https://github.com/llvm/llvm-project/issues/68615>`_)

- Clang now won't mention invisible namespace when diagnose invisible declarations
  inside namespace. The original diagnostic message is confusing.
  (`#73893 <https://github.com/llvm/llvm-project/issues/73893>`_)

- Clang now diagnoses member template declarations with multiple declarators.
- Clang now diagnoses use of the ``template`` keyword after declarative nested name specifiers.

Improvements to Clang's time-trace
----------------------------------

Bug Fixes in This Version
-------------------------
- Fixed an issue where a class template specialization whose declaration is
  instantiated in one module and whose definition is instantiated in another
  module may end up with members associated with the wrong declaration of the
  class, which can result in miscompiles in some cases.
- Fix crash on use of a variadic overloaded operator.
  (`#42535 <https://github.com/llvm/llvm-project/issues/42535>`_)
- Fix a hang on valid C code passing a function type as an argument to
  ``typeof`` to form a function declaration.
  (`#64713 <https://github.com/llvm/llvm-project/issues/64713>`_)
- Clang now reports missing-field-initializers warning for missing designated
  initializers in C++.
  (`#56628 <https://github.com/llvm/llvm-project/issues/56628>`_)
- Clang now respects ``-fwrapv`` and ``-ftrapv`` for ``__builtin_abs`` and
  ``abs`` builtins.
  (`#45129 <https://github.com/llvm/llvm-project/issues/45129>`_,
  `#45794 <https://github.com/llvm/llvm-project/issues/45794>`_)
- Fixed an issue where accesses to the local variables of a coroutine during
  ``await_suspend`` could be misoptimized, including accesses to the awaiter
  object itself.
  (`#56301 <https://github.com/llvm/llvm-project/issues/56301>`_)
  The current solution may bring performance regressions if the awaiters have
  non-static data members. See
  `#64945 <https://github.com/llvm/llvm-project/issues/64945>`_ for details.
- Clang now prints unnamed members in diagnostic messages instead of giving an
  empty ''. Fixes
  (`#63759 <https://github.com/llvm/llvm-project/issues/63759>`_)
- Fix crash in __builtin_strncmp and related builtins when the size value
  exceeded the maximum value representable by int64_t. Fixes
  (`#64876 <https://github.com/llvm/llvm-project/issues/64876>`_)
- Fixed an assertion if a function has cleanups and fatal erors.
  (`#48974 <https://github.com/llvm/llvm-project/issues/48974>`_)
- Clang now emits an error if it is not possible to deduce array size for a
  variable with incomplete array type.
  (`#37257 <https://github.com/llvm/llvm-project/issues/37257>`_)
- Clang's ``-Wunused-private-field`` no longer warns on fields whose type is
  declared with ``[[maybe_unused]]``.
  (`#61334 <https://github.com/llvm/llvm-project/issues/61334>`_)
- For function multi-versioning using the ``target``, ``target_clones``, or
  ``target_version`` attributes, remove comdat for internal linkage functions.
  (`#65114 <https://github.com/llvm/llvm-project/issues/65114>`_)
- Clang now reports ``-Wformat`` for bool value and char specifier confusion
  in scanf. Fixes
  (`#64987 <https://github.com/llvm/llvm-project/issues/64987>`_)
- Support MSVC predefined macro expressions in constant expressions and in
  local structs.
- Correctly parse non-ascii identifiers that appear immediately after a line splicing
  (`#65156 <https://github.com/llvm/llvm-project/issues/65156>`_)
- Clang no longer considers the loss of ``__unaligned`` qualifier from objects as
  an invalid conversion during method function overload resolution.
- Fix lack of comparison of declRefExpr in ASTStructuralEquivalence
  (`#66047 <https://github.com/llvm/llvm-project/issues/66047>`_)
- Fix parser crash when dealing with ill-formed objective C++ header code. Fixes
  (`#64836 <https://github.com/llvm/llvm-project/issues/64836>`_)
- Fix crash in implicit conversions from initialize list to arrays of unknown
  bound for C++20. Fixes
  (`#62945 <https://github.com/llvm/llvm-project/issues/62945>`_)
- Clang now allows an ``_Atomic`` qualified integer in a switch statement. Fixes
  (`#65557 <https://github.com/llvm/llvm-project/issues/65557>`_)
- Fixes crash when trying to obtain the common sugared type of
  `decltype(instantiation-dependent-expr)`.
  Fixes (`#67603 <https://github.com/llvm/llvm-project/issues/67603>`_)
- Fixes a crash caused by a multidimensional array being captured by a lambda
  (`#67722 <https://github.com/llvm/llvm-project/issues/67722>`_).
- Fixes a crash when instantiating a lambda with requires clause.
  (`#64462 <https://github.com/llvm/llvm-project/issues/64462>`_)
- Fixes a regression where the ``UserDefinedLiteral`` was not properly preserved
  while evaluating consteval functions. (`#63898 <https://github.com/llvm/llvm-project/issues/63898>`_).
- Fix a crash when evaluating value-dependent structured binding
  variables at compile time.
  Fixes (`#67690 <https://github.com/llvm/llvm-project/issues/67690>`_)
- Fixes a ``clang-17`` regression where ``LLVM_UNREACHABLE_OPTIMIZE=OFF``
  cannot be used with ``Release`` mode builds. (`#68237 <https://github.com/llvm/llvm-project/issues/68237>`_).
- Fix crash in evaluating ``constexpr`` value for invalid template function.
  Fixes (`#68542 <https://github.com/llvm/llvm-project/issues/68542>`_)
- Clang will correctly evaluate ``noexcept`` expression for template functions
  of template classes. Fixes
  (`#68543 <https://github.com/llvm/llvm-project/issues/68543>`_,
  `#42496 <https://github.com/llvm/llvm-project/issues/42496>`_,
  `#77071 <https://github.com/llvm/llvm-project/issues/77071>`_,
  `#77411 <https://github.com/llvm/llvm-project/issues/77411>`_)
- Fixed an issue when a shift count larger than ``__INT64_MAX__``, in a right
  shift operation, could result in missing warnings about
  ``shift count >= width of type`` or internal compiler error.
- Fixed an issue with computing the common type for the LHS and RHS of a `?:`
  operator in C. No longer issuing a confusing diagnostic along the lines of
  "incompatible operand types ('foo' and 'foo')" with extensions such as matrix
  types. Fixes (`#69008 <https://github.com/llvm/llvm-project/issues/69008>`_)
- Clang no longer permits using the `_BitInt` types as an underlying type for an
  enumeration as specified in the C23 Standard.
  Fixes (`#69619 <https://github.com/llvm/llvm-project/issues/69619>`_)
- Fixed an issue when a shift count specified by a small constant ``_BitInt()``,
  in a left shift operation, could result in a faulty warnings about
  ``shift count >= width of type``.
- Clang now accepts anonymous members initialized with designated initializers
  inside templates.
  Fixes (`#65143 <https://github.com/llvm/llvm-project/issues/65143>`_)
- Fix crash in formatting the real/imaginary part of a complex lvalue.
  Fixes (`#69218 <https://github.com/llvm/llvm-project/issues/69218>`_)
- No longer use C++ ``thread_local`` semantics in C23 when using
  ``thread_local`` instead of ``_Thread_local``.
  Fixes (`#70068 <https://github.com/llvm/llvm-project/issues/70068>`_) and
  (`#69167 <https://github.com/llvm/llvm-project/issues/69167>`_)
- Fix crash in evaluating invalid lambda expression which forget capture this.
  Fixes (`#67687 <https://github.com/llvm/llvm-project/issues/67687>`_)
- Fix crash from constexpr evaluator evaluating uninitialized arrays as rvalue.
  Fixes (`#67317 <https://github.com/llvm/llvm-project/issues/67317>`_)
- Clang now properly diagnoses use of stand-alone OpenMP directives after a
  label (including ``case`` or ``default`` labels).
- Fix compiler memory leak for enums with underlying type larger than 64 bits.
  Fixes (`#78311 <https://github.com/llvm/llvm-project/pull/78311>`_)

  Before:

  .. code-block:: c++

    label:
    #pragma omp barrier // ok

  After:

  .. code-block:: c++

    label:
    #pragma omp barrier // error: '#pragma omp barrier' cannot be an immediate substatement

- Fixed an issue that a benign assertion might hit when instantiating a pack expansion
  inside a lambda. (`#61460 <https://github.com/llvm/llvm-project/issues/61460>`_)
- Fix crash during instantiation of some class template specializations within class
  templates. Fixes (`#70375 <https://github.com/llvm/llvm-project/issues/70375>`_)
- Fix crash during code generation of C++ coroutine initial suspend when the return
  type of await_resume is not trivially destructible.
  Fixes (`#63803 <https://github.com/llvm/llvm-project/issues/63803>`_)
- ``__is_trivially_relocatable`` no longer returns true for non-object types
  such as references and functions.
  Fixes (`#67498 <https://github.com/llvm/llvm-project/issues/67498>`_)
- Fix crash when the object used as a ``static_assert`` message has ``size`` or ``data`` members
  which are not member functions.
- Support UDLs in ``static_assert`` message.
- Fixed false positive error emitted by clang when performing qualified name
  lookup and the current class instantiation has dependent bases.
  Fixes (`#13826 <https://github.com/llvm/llvm-project/issues/13826>`_)
- Fix a ``clang-17`` regression where a templated friend with constraints is not
  properly applied when its parameters reference an enclosing non-template class.
  Fixes (`#71595 <https://github.com/llvm/llvm-project/issues/71595>`_)
- Fix the name of the ifunc symbol emitted for multiversion functions declared with the
  ``target_clones`` attribute. This addresses a linker error that would otherwise occur
  when these functions are referenced from other TUs.
- Fixes compile error that double colon operator cannot resolve macro with parentheses.
  Fixes (`#64467 <https://github.com/llvm/llvm-project/issues/64467>`_)
- Clang's ``-Wchar-subscripts`` no longer warns on chars whose values are known non-negative constants.
  Fixes (`#18763 <https://github.com/llvm/llvm-project/issues/18763>`_)
- Fix crash due to incorrectly allowing conversion functions in copy elision.
  Fixes (`#39319 <https://github.com/llvm/llvm-project/issues/39319>`_) and
  (`#60182 <https://github.com/llvm/llvm-project/issues/60182>`_) and
  (`#62157 <https://github.com/llvm/llvm-project/issues/62157>`_) and
  (`#64885 <https://github.com/llvm/llvm-project/issues/64885>`_) and
  (`#65568 <https://github.com/llvm/llvm-project/issues/65568>`_)
- Fix an issue where clang doesn't respect detault template arguments that
  are added in a later redeclaration for CTAD.
  Fixes (`#69987 <https://github.com/llvm/llvm-project/issues/69987>`_)
- Fix an issue where CTAD fails for explicit type conversion.
  Fixes (`#64347 <https://github.com/llvm/llvm-project/issues/64347>`_)
- Fix crash when using C++ only tokens like ``::`` in C compiler clang.
  Fixes (`#73559 <https://github.com/llvm/llvm-project/issues/73559>`_)
- Clang now accepts recursive non-dependent calls to functions with deduced
  return type.
  Fixes (`#71015 <https://github.com/llvm/llvm-project/issues/71015>`_)
- Fix assertion failure when initializing union containing struct with
  flexible array member using empty initializer list.
  Fixes (`#77085 <https://github.com/llvm/llvm-project/issues/77085>`_)
- Fix assertion crash due to failed scope restoring caused by too-early VarDecl
  invalidation by invalid initializer Expr.
  Fixes (`#30908 <https://github.com/llvm/llvm-project/issues/30908>`_)
- Clang now emits correct source location for code-coverage regions in `if constexpr`
  and `if consteval` branches. Untaken branches are now skipped.
  Fixes (`#54419 <https://github.com/llvm/llvm-project/issues/54419>`_)
- Fix assertion failure when declaring a template friend function with
  a constrained parameter in a template class that declares a class method
  or lambda at different depth.
  Fixes (`#75426 <https://github.com/llvm/llvm-project/issues/75426>`_)
- Fix an issue where clang cannot find conversion function with template
  parameter when instantiation of template class.
  Fixes (`#77583 <https://github.com/llvm/llvm-project/issues/77583>`_)
- Fix an issue where CTAD fails for function-type/array-type arguments.
  Fixes (`#51710 <https://github.com/llvm/llvm-project/issues/51710>`_)
- Fix crashes when using the binding decl from an invalid structured binding.
  Fixes (`#67495 <https://github.com/llvm/llvm-project/issues/67495>`_) and
  (`#72198 <https://github.com/llvm/llvm-project/issues/72198>`_)
- Fix assertion failure when call noreturn-attribute function with musttail
  attribute.
  Fixes (`#76631 <https://github.com/llvm/llvm-project/issues/76631>`_)
  - The MS ``__noop`` builtin without an argument list is now accepted
  in the placement-args of new-expressions, matching MSVC's behaviour.
- Fix an issue that caused MS ``__decspec(property)`` accesses as well as
  Objective-C++ property accesses to not be converted to a function call
  to the getter in the placement-args of new-expressions.
  Fixes (`#65053 <https://github.com/llvm/llvm-project/issues/65053>`_)
- Fix an issue with missing symbol definitions when the first coroutine
  statement appears in a discarded ``if constexpr`` branch.
  Fixes (`#78290 <https://github.com/llvm/llvm-project/issues/78290>`_)
- Fixed assertion failure with deleted overloaded unary operators.
  Fixes (`#78314 <https://github.com/llvm/llvm-project/issues/78314>`_)
- The XCOFF object file format does not support aliases to symbols having common
  linkage. Clang now diagnoses the use of an alias for a common symbol when
  compiling for AIX.

- Clang now doesn't produce false-positive warning `-Wconstant-logical-operand`
  for logical operators in C23.
  Fixes (`#64356 <https://github.com/llvm/llvm-project/issues/64356>`_).

- Fixed missing warnings when doing bool-like conversions in C23 (`#79435 <https://github.com/llvm/llvm-project/issues/79435>`_).

- Clang now accepts qualified partial/explicit specializations of variable templates that
  are not nominable in the lookup context of the specialization.

Bug Fixes to Compiler Builtins
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Bug Fixes to Attribute Support
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Bug Fixes to C++ Support
^^^^^^^^^^^^^^^^^^^^^^^^

- Clang limits the size of arrays it will try to evaluate at compile time
  to avoid memory exhaustion.
  This limit can be modified by `-fconstexpr-steps`.
  (`#63562 <https://github.com/llvm/llvm-project/issues/63562>`_)

- Fix a crash caused by some named unicode escape sequences designating
  a Unicode character whose name contains a ``-``.
  (Fixes `#64161 <https://github.com/llvm/llvm-project/issues/64161>`_)

- Fix cases where we ignore ambiguous name lookup when looking up members.
  (`#22413 <https://github.com/llvm/llvm-project/issues/22413>`_),
  (`#29942 <https://github.com/llvm/llvm-project/issues/29942>`_),
  (`#35574 <https://github.com/llvm/llvm-project/issues/35574>`_) and
  (`#27224 <https://github.com/llvm/llvm-project/issues/27224>`_).

- Clang emits an error on substitution failure within lambda body inside a
  requires-expression. This fixes:
  (`#64138 <https://github.com/llvm/llvm-project/issues/64138>`_) and
  (`#71684 <https://github.com/llvm/llvm-project/issues/71684>`_).

- Update ``FunctionDeclBitfields.NumFunctionDeclBits``. This fixes:
  (`#64171 <https://github.com/llvm/llvm-project/issues/64171>`_).

- Expressions producing ``nullptr`` are correctly evaluated
  by the constant interpreter when appearing as the operand
  of a binary comparison.
  (`#64923 <https://github.com/llvm/llvm-project/issues/64923>`_)

- Fix a crash when an immediate invocation is not a constant expression
  and appear in an implicit cast.
  (`#64949 <https://github.com/llvm/llvm-project/issues/64949>`_).

- Fix crash when parsing ill-formed lambda trailing return type. Fixes:
  (`#64962 <https://github.com/llvm/llvm-project/issues/64962>`_) and
  (`#28679 <https://github.com/llvm/llvm-project/issues/28679>`_).

- Fix a crash caused by substitution failure in expression requirements.
  (`#64172 <https://github.com/llvm/llvm-project/issues/64172>`_) and
  (`#64723 <https://github.com/llvm/llvm-project/issues/64723>`_).

- Fix crash when parsing the requires clause of some generic lambdas.
  (`#64689 <https://github.com/llvm/llvm-project/issues/64689>`_)

- Fix crash when the trailing return type of a generic and dependent
  lambda refers to an init-capture.
  (`#65067 <https://github.com/llvm/llvm-project/issues/65067>`_ and
  `#63675 <https://github.com/llvm/llvm-project/issues/63675>`_)

- Clang now properly handles out of line template specializations when there is
  a non-template inner-class between the function and the class template.
  (`#65810 <https://github.com/llvm/llvm-project/issues/65810>`_)

- Fix a crash when calling a non-constant immediate function
  in the initializer of a static data member.
  (`#65985 <https://github.com/llvm/llvm-project/issues/65985>`_).
- Clang now properly converts static lambda call operator to function
  pointers on win32.
  (`#62594 <https://github.com/llvm/llvm-project/issues/62594>`_)

- Fixed some cases where the source location for an instantiated specialization
  of a function template or a member function of a class template was assigned
  the location of a non-defining declaration rather than the location of the
  definition the specialization was instantiated from.
  (`#26057 <https://github.com/llvm/llvm-project/issues/26057>`_)

- Fix a crash when a default member initializer of a base aggregate
  makes an invalid call to an immediate function.
  (`#66324 <https://github.com/llvm/llvm-project/issues/66324>`_)

- Fix crash for a lambda attribute with a statement expression
  that contains a `return`.
  (`#48527 <https://github.com/llvm/llvm-project/issues/48527>`_)

- Clang now no longer asserts when an UnresolvedLookupExpr is used as an
  expression requirement. (`#66612 <https://github.com/llvm/llvm-project/issues/66612>`_)

- Clang now disambiguates NTTP types when printing diagnostics where the
  NTTP types are compared with the 'diff' method.
  (`#66744 <https://github.com/llvm/llvm-project/issues/66744>`_)

- Fix crash caused by a spaceship operator returning a comparision category by
  reference. Fixes:
  (`#64162 <https://github.com/llvm/llvm-project/issues/64162>`_)
- Fix a crash when calling a consteval function in an expression used as
  the size of an array.
  (`#65520 <https://github.com/llvm/llvm-project/issues/65520>`_)

- Clang no longer tries to capture non-odr-used variables that appear
  in the enclosing expression of a lambda expression with a noexcept specifier.
  (`#67492 <https://github.com/llvm/llvm-project/issues/67492>`_)

- Fix crash when fold expression was used in the initialization of default
  argument. Fixes:
  (`#67395 <https://github.com/llvm/llvm-project/issues/67395>`_)

- Fixed a bug causing destructors of constant-evaluated structured bindings
  initialized by array elements to be called in the wrong evaluation context.

- Fix crash where ill-formed code was being treated as a deduction guide and
  we now produce a diagnostic. Fixes:
  (`#65522 <https://github.com/llvm/llvm-project/issues/65522>`_)

- Fixed a bug where clang incorrectly considered implicitly generated deduction
  guides from a non-templated constructor and a templated constructor as ambiguous,
  rather than prefer the non-templated constructor as specified in
  [standard.group]p3.

- Fixed a crash caused by incorrect handling of dependence on variable templates
  with non-type template parameters of reference type. Fixes:
  (`#65153 <https://github.com/llvm/llvm-project/issues/65153>`_)

- Clang now properly compares constraints on an out of line class template
  declaration definition. Fixes:
  (`#61763 <https://github.com/llvm/llvm-project/issues/61763>`_)

- Fix a bug where implicit deduction guides are not correctly generated for nested template
  classes. Fixes:
  (`#46200 <https://github.com/llvm/llvm-project/issues/46200>`_)
  (`#57812 <https://github.com/llvm/llvm-project/issues/57812>`_)

- Diagnose use of a variable-length array in a coroutine. The design of
  coroutines is such that it is not possible to support VLA use. Fixes:
  (`#65858 <https://github.com/llvm/llvm-project/issues/65858>`_)

- Fix bug where we were overriding zero-initialization of class members when
  default initializing a base class in a constant expression context. Fixes:
  (`#69890 <https://github.com/llvm/llvm-project/issues/69890>`_)

- Fix crash when template class static member imported to other translation unit.
  Fixes:
  (`#68769 <https://github.com/llvm/llvm-project/issues/68769>`_)

- Clang now rejects incomplete types for ``__builtin_dump_struct``. Fixes:
  (`#63506 <https://github.com/llvm/llvm-project/issues/63506>`_)

- Fixed a crash for C++98/03 while checking an ill-formed ``_Static_assert`` expression.
  Fixes: (`#72025 <https://github.com/llvm/llvm-project/issues/72025>`_)

- Clang now defers the instantiation of explicit specifier until constraint checking
  completes (except deduction guides). Fixes:
  (`#59827 <https://github.com/llvm/llvm-project/issues/59827>`_)

- Fix crash when parsing nested requirement. Fixes:
  (`#73112 <https://github.com/llvm/llvm-project/issues/73112>`_)

- Fixed a crash caused by using return type requirement in a lambda. Fixes:
  (`#63808 <https://github.com/llvm/llvm-project/issues/63808>`_)
  (`#64607 <https://github.com/llvm/llvm-project/issues/64607>`_)
  (`#64086 <https://github.com/llvm/llvm-project/issues/64086>`_)

- Fixed a crash where we lost uninstantiated constraints on placeholder NTTP packs. Fixes:
  (`#63837 <https://github.com/llvm/llvm-project/issues/63837>`_)

- Fixed a regression where clang forgets how to substitute into constraints on template-template
  parameters. Fixes:
  (`#57410 <https://github.com/llvm/llvm-project/issues/57410>`_) and
  (`#76604 <https://github.com/llvm/llvm-project/issues/57410>`_)

- Fix a bug where clang would produce inconsistent values when
  ``std::source_location::current()`` was used in a function template.
  Fixes (`#78128 <https://github.com/llvm/llvm-project/issues/78128>`_)

- Clang now allows parenthesized initialization of arrays in `operator new[]`.
  Fixes: (`#68198 <https://github.com/llvm/llvm-project/issues/68198>`_)

- Fixes CTAD for aggregates on nested template classes. Fixes:
  (`#77599 <https://github.com/llvm/llvm-project/issues/77599>`_)

- Fix crash when importing the same module with an dynamic initializer twice
  in different visibility.
  Fixes (`#67893 <https://github.com/llvm/llvm-project/issues/67893>`_)

- Remove recorded `#pragma once` state for headers included in named modules.
  Fixes (`#77995 <https://github.com/llvm/llvm-project/issues/77995>`_)

- Set the ``__cpp_auto_cast`` feature test macro in C++23 mode.

- Fix crash for inconsistent deducing state of function return types
  in importing modules.
  Fixes (`#78830 <https://github.com/llvm/llvm-project/issues/78830>`_)
  Fixes (`#60085 <https://github.com/llvm/llvm-project/issues/60085>`_)


- Fixed a bug where variables referenced by requires-clauses inside
  nested generic lambdas were not properly injected into the constraint scope.
  (`#73418 <https://github.com/llvm/llvm-project/issues/73418>`_)
- Fixed a crash where substituting into a requires-expression that refers to function
  parameters during the equivalence determination of two constraint expressions.
  (`#74447 <https://github.com/llvm/llvm-project/issues/74447>`_)
- Fixed deducing auto& from const int in template parameters of partial
  specializations.
  (`#77189 <https://github.com/llvm/llvm-project/issues/77189>`_)
- Fix for crash when using a erroneous type in a return statement.
  Fixes (`#63244 <https://github.com/llvm/llvm-project/issues/63244>`_)
  and (`#79745 <https://github.com/llvm/llvm-project/issues/79745>`_)
- Fix incorrect code generation caused by the object argument of ``static operator()`` and ``static operator[]`` calls not being evaluated.
  Fixes (`#67976 <https://github.com/llvm/llvm-project/issues/67976>`_)

- Fix incorrect code generation caused by the object argument of ``static operator()`` and ``static operator[]`` calls not being evaluated.
  Fixes (`#67976 <https://github.com/llvm/llvm-project/issues/67976>`_)

- Fix crash when using an immediate-escalated function at global scope.
  (`#82258 <https://github.com/llvm/llvm-project/issues/82258>`_)
- Correctly immediate-escalate lambda conversion functions.
  (`#82258 <https://github.com/llvm/llvm-project/issues/82258>`_)

Bug Fixes to AST Handling
^^^^^^^^^^^^^^^^^^^^^^^^^

Miscellaneous Bug Fixes
^^^^^^^^^^^^^^^^^^^^^^^

Miscellaneous Clang Crashes Fixed
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

OpenACC Specific Changes
------------------------

Target Specific Changes
-----------------------

AMDGPU Support
^^^^^^^^^^^^^^

X86 Support
^^^^^^^^^^^

Arm and AArch64 Support
^^^^^^^^^^^^^^^^^^^^^^^

- C++ function name mangling has been changed to align with the specification
  (https://github.com/ARM-software/abi-aa/blob/main/aapcs64/aapcs64.rst).
  This affects C++ functions with SVE ACLE parameters. Clang will use the old
  manglings if ``-fclang-abi-compat=17`` or lower is  specified.

- New AArch64 asm constraints have been added for r8-r11(Uci) and r12-r15(Ucj).

- Support has been added for the following processors (-mcpu identifiers in parenthesis):

  For Arm:

  * Cortex-M52 (cortex-m52).

  For AArch64:

  * Cortex-A520 (cortex-a520).
  * Cortex-A720 (cortex-a720).
  * Cortex-X4 (cortex-x4).

- Alpha support has been added for SVE2.1 intrinsics.

- Support has been added for `-fstack-clash-protection` and `-mstack-probe-size`
  command line options.

- Function Multi Versioning has been extended to support Load-Acquire RCpc
  instructions v3 (rcpc3) as well as Memory Copy and Memory Set Acceleration
  instructions (mops) when targeting AArch64. The feature identifiers (in
  parenthesis) can be used with either of the ``target_version`` and
  ``target_clones`` attributes.

Android Support
^^^^^^^^^^^^^^^

Windows Support
^^^^^^^^^^^^^^^
- Fixed an assertion failure that occurred due to a failure to propagate
  ``MSInheritanceAttr`` attributes to class template instantiations created
  for explicit template instantiation declarations.

- The ``-fno-auto-import`` option was added for MinGW targets. The option both
  affects code generation (inhibiting generating indirection via ``.refptr``
  stubs for potentially auto imported symbols, generating smaller and more
  efficient code) and linking (making the linker error out on such cases).
  If the option only is used during code generation but not when linking,
  linking may succeed but the resulting executables may expose issues at
  runtime.

- Clang now passes relevant LTO options to the linker (LLD) in MinGW mode.

LoongArch Support
^^^^^^^^^^^^^^^^^

RISC-V Support
^^^^^^^^^^^^^^
- Unaligned memory accesses can be toggled by ``-m[no-]unaligned-access`` or the
  aliases ``-m[no-]strict-align``.
- CodeGen of RV32E/RV64E was supported experimentally.
- CodeGen of ilp32e/lp64e was supported experimentally.

- Default ABI with F but without D was changed to ilp32f for RV32 and to lp64f
  for RV64.

- ``__attribute__((rvv_vector_bits(N)))`` is now supported for RVV vbool*_t types.
- ``-mtls-dialect=desc`` is now supported to enable TLS descriptors (TLSDESC).

- ``__attribute__((rvv_vector_bits(N)))`` is now supported for RVV vbool*_t types.

CUDA/HIP Language Changes
^^^^^^^^^^^^^^^^^^^^^^^^^

CUDA Support
^^^^^^^^^^^^

- Clang now supports CUDA SDK up to 12.3
- Added support for sm_90a

PowerPC Support
^^^^^^^^^^^^^^^

- Added ``nmmintrin.h`` to intrinsics headers.
- Added ``__builtin_ppc_fence`` as barrier of code motion, and
  ``__builtin_ppc_mffsl`` for corresponding instruction.
- Supported ``__attribute__((target("tune=cpu")))``.
- Emit ``float-abi`` module flag on 64-bit ELFv2 PowerPC targets if
  ``long double`` type is used in current module.

AIX Support
^^^^^^^^^^^

- Introduced the ``-maix-small-local-exec-tls`` option to produce a faster
  access sequence for local-exec TLS variables where the offset from the TLS
  base is encoded as an immediate operand.
  This access sequence is not used for TLS variables larger than 32KB, and is
  currently only supported on 64-bit mode.
- Inline assembler supports VSR register in pure digits.
- Enabled ThinLTO support. Requires AIX 7.2 TL5 SP7 or newer, or AIX 7.3 TL2
  or newer. Similar to the LTO support on AIX, ThinLTO is implemented with
  the libLTO.so plugin.

WebAssembly Support
^^^^^^^^^^^^^^^^^^^

AVR Support
^^^^^^^^^^^

DWARF Support in Clang
----------------------

Floating Point Support in Clang
-------------------------------

AST Matchers
------------

clang-format
------------

libclang
--------

- Exposed arguments of ``clang::annotate``.
- ``clang::getCursorKindForDecl`` now recognizes linkage specifications such as
  ``extern "C"`` and reports them as ``CXCursor_LinkageSpec``.
- Changed the libclang library on AIX to export only the necessary symbols to
  prevent issues of resolving to the wrong duplicate symbol.

Static Analyzer
---------------

New features
^^^^^^^^^^^^

- Implemented the ``[[clang::suppress]]`` attribute for suppressing diagnostics
  of static analysis tools, such as the Clang Static Analyzer.
  `Documentation <https://clang.llvm.org/docs/AttributeReference.html#suppress>`__.

- Support "Deducing this" (P0847R7). (Worked out of the box)
  (`af4751738db8 <https://github.com/llvm/llvm-project/commit/af4751738db89a142a8880c782d12d4201b222a8>`__)

- Added a new checker ``core.BitwiseShift`` which reports situations where
  bitwise shift operators produce undefined behavior (because some operand is
  negative or too large).
  `Documentation <https://clang.llvm.org/docs/analyzer/checkers.html#core-bitwiseshift-c-c>`__.

- Added a new experimental checker ``alpha.core.StdVariant`` to detect variant
  accesses via wrong alternatives.
  `Documentation <https://clang.llvm.org/docs/analyzer/checkers.html#alpha-core-stdvariant-c>`__.
  (`#66481 <https://github.com/llvm/llvm-project/pull/66481>`_)

- Added a new experimental checker ``alpha.cplusplus.ArrayDelete`` to detect
  destructions of arrays of polymorphic objects that are destructed as their
  base class (`CERT EXP51-CPP <https://wiki.sei.cmu.edu/confluence/display/cplusplus/EXP51-CPP.+Do+not+delete+an+array+through+a+pointer+of+the+incorrect+type>`_).
  `Documentation <https://clang.llvm.org/docs/analyzer/checkers.html#alpha-cplusplus-arraydelete-c>`__.
  (`0e246bb67573 <https://github.com/llvm/llvm-project/commit/0e246bb67573799409d0085b89902a330998ddcc>`_)

- Added a new checker configuration option ``InvalidatingGetEnv=[true,false]`` to
  ``security.cert.env.InvalidPtr``. It's not set by default.
  If set, ``getenv`` calls won't invalidate previously returned pointers.
  `Documentation <https://clang.llvm.org/docs/analyzer/checkers.html#security-cert-env-invalidptr>`__.
  (`#67663 <https://github.com/llvm/llvm-project/pull/67663>`_)

Crash and bug fixes
^^^^^^^^^^^^^^^^^^^

Improvements
^^^^^^^^^^^^

- Support importing C++20 modules in clang-repl.

- Added support for ``TypeLoc::dump()`` for easier debugging, and improved
  textual and JSON dumping for various ``TypeLoc``-related nodes.

Moved checkers
^^^^^^^^^^^^^^

.. _release-notes-sanitizers:

Sanitizers
----------

Python Binding Changes
----------------------

- Exposed `CXRewriter` API as `class Rewriter`.

Additional Information
======================

A wide variety of additional information is available on the `Clang web
page <https://clang.llvm.org/>`_. The web page contains versions of the
API documentation which are up-to-date with the Git version of
the source code. You can access versions of these documents specific to
this release by going into the "``clang/docs/``" directory in the Clang
tree.

If you have any questions or comments about Clang, please feel free to
contact us on the `Discourse forums (Clang Frontend category)
<https://discourse.llvm.org/c/clang/6>`_.
