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
- Clang now diagnoses function/variable templates that shadow their own template parameters, e.g. ``template<class T> void T();``.
  This error can be disabled via `-Wno-strict-primary-template-shadow` for compatibility with previous versions of clang.

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
- Removed support for constructing on-stack ``TemplateArgumentList``s; interfaces should instead
  use ``ArrayRef<TemplateArgument>`` to pass template arguments. Transitioning internal uses to
  ``ArrayRef<TemplateArgument>`` reduces AST memory usage by 0.4% when compiling clang, and is
  expected to show similar improvements on other workloads.

- The ``-Wgnu-binary-literal`` diagnostic group no longer controls any
  diagnostics. Binary literals are no longer a GNU extension, they're now a C23
  extension which is controlled via ``-pedantic`` or ``-Wc23-extensions``. Use
  of ``-Wno-gnu-binary-literal`` will no longer silence this pedantic warning,
  which may break existing uses with ``-Werror``.

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
  (#GH79240).

- Implemented the `__is_layout_compatible` intrinsic to support
  `P0466R5: Layout-compatibility and Pointer-interconvertibility Traits <https://wg21.link/P0466R5>`_.

- Clang now implements [module.import]p7 fully. Clang now will import module
  units transitively for the module units coming from the same module of the
  current module units.
  Fixes `#84002 <https://github.com/llvm/llvm-project/issues/84002>`_.

C++23 Feature Support
^^^^^^^^^^^^^^^^^^^^^

- Implemented `P2718R0: Lifetime extension in range-based for loops <https://wg21.link/P2718R0>`_. Also
  materialize temporary object which is a prvalue in discarded-value expression.

- Implemented `P2448R2: Relaxing some constexpr restrictions <https://wg21.link/P2448R2>`_.

C++2c Feature Support
^^^^^^^^^^^^^^^^^^^^^

- Implemented `P2662R3 Pack Indexing <https://wg21.link/P2662R3>`_.


Resolutions to C++ Defect Reports
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- Substitute template parameter pack, when it is not explicitly specified
  in the template parameters, but is deduced from a previous argument.
  (`#78449: <https://github.com/llvm/llvm-project/issues/78449>`_).

- Type qualifications are now ignored when evaluating layout compatibility
  of two types.
  (`CWG1719: Layout compatibility and cv-qualification revisited <https://cplusplus.github.io/CWG/issues/1719.html>`_).

- Alignment of members is now respected when evaluating layout compatibility
  of structs.
  (`CWG2583: Common initial sequence should consider over-alignment <https://cplusplus.github.io/CWG/issues/2583.html>`_).

- ``[[no_unique_address]]`` is now respected when evaluating layout
  compatibility of two types.
  (`CWG2759: [[no_unique_address] and common initial sequence  <https://cplusplus.github.io/CWG/issues/2759.html>`_).

C Language Changes
------------------

C23 Feature Support
^^^^^^^^^^^^^^^^^^^
- No longer diagnose use of binary literals as an extension in C23 mode. Fixes
  #GH72017.

- Corrected parsing behavior for the ``alignas`` specifier/qualifier in C23. We
  previously handled it as an attribute as in C++, but there are parsing
  differences. The behavioral differences are:

  .. code-block:: c

     struct alignas(8) /* was accepted, now rejected */ S {
       char alignas(8) /* was rejected, now accepted */ C;
     };
     int i alignas(8) /* was accepted, now rejected */ ;

  Fixes (#GH81472).

- Clang now generates predefined macros of the form ``__TYPE_FMTB__`` and
  ``__TYPE_FMTb__`` (e.g., ``__UINT_FAST64_FMTB__``) in C23 mode for use with
  macros typically exposed from ``<inttypes.h>``, such as ``PRIb8``.
  (`#81896: <https://github.com/llvm/llvm-project/issues/81896>`_).

- Clang now supports `N3018 The constexpr specifier for object definitions`
  <https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3018.htm>`_.

Non-comprehensive list of changes in this release
-------------------------------------------------

- Added ``__builtin_readsteadycounter`` for reading fixed frequency hardware
  counters.

- ``__builtin_addc``, ``__builtin_subc``, and the other sizes of those
  builtins are now constexpr and may be used in constant expressions.

- Added ``__builtin_popcountg`` as a type-generic alternative to
  ``__builtin_popcount{,l,ll}`` with support for any unsigned integer type. Like
  the previous builtins, this new builtin is constexpr and may be used in
  constant expressions.

New Compiler Flags
------------------

- ``-Wmissing-designated-field-initializers``, grouped under ``-Wmissing-field-initializers``.
  This diagnostic can be disabled to make ``-Wmissing-field-initializers`` behave
  like it did before Clang 18.x. Fixes (`#56628 <https://github.com/llvm/llvm-project/issues/68933>`_)

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

- The ``-freroll-loops`` flag has been removed. It had no effect since Clang 13.

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

- Clang now diagnoses use of the ``template`` keyword after declarative nested
  name specifiers.

- The ``-Wshorten-64-to-32`` diagnostic is now grouped under ``-Wimplicit-int-conversion`` instead
   of ``-Wconversion``. Fixes #GH69444.

- Clang now uses thousand separators when printing large numbers in integer overflow diagnostics.
  Fixes #GH80939.

- Clang now diagnoses friend declarations with an ``enum`` elaborated-type-specifier in language modes after C++98.

- Added diagnostics for C11 keywords being incompatible with language standards
  before C11, under a new warning group: ``-Wpre-c11-compat``.

- Now diagnoses an enumeration constant whose value is larger than can be
  represented by ``unsigned long long``, which can happen with a large constant
  using the ``wb`` or ``uwb`` suffix. The maximal underlying type is currently
  ``unsigned long long``, but this behavior may change in the future when Clang
  implements
  `WG14 N3029 <https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3029.htm>`_.
  (#GH69352).

- Clang now diagnoses extraneous template parameter lists as a language extension.

- Clang now diagnoses declarative nested name specifiers that name alias templates.

- Clang now diagnoses lambda function expressions being implicitly cast to boolean values, under ``-Wpointer-bool-conversion``.
  Fixes #GH82512.

Improvements to Clang's time-trace
----------------------------------

Bug Fixes in This Version
-------------------------
- Fixed missing warnings when comparing mismatched enumeration constants
  in C (`#29217 <https://github.com/llvm/llvm-project/issues/29217>`).

- Clang now accepts elaborated-type-specifiers that explicitly specialize
  a member class template for an implicit instantiation of a class template.

- Fixed missing warnings when doing bool-like conversions in C23 (#GH79435).
- Clang's ``-Wshadow`` no longer warns when an init-capture is named the same as
  a class field unless the lambda can capture this.
  Fixes (#GH71976)

- Clang now accepts qualified partial/explicit specializations of variable templates that
  are not nominable in the lookup context of the specialization.

- Clang now doesn't produce false-positive warning `-Wconstant-logical-operand`
  for logical operators in C23.
  Fixes (#GH64356).

- Clang no longer produces a false-positive `-Wunused-variable` warning
  for variables created through copy initialization having side-effects in C++17 and later.
  Fixes (#GH64356) (#GH79518).

- Clang now emits errors for explicit specializations/instatiations of lambda call
  operator.
  Fixes (#GH83267).

Bug Fixes to Compiler Builtins
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Bug Fixes to Attribute Support
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Bug Fixes to C++ Support
^^^^^^^^^^^^^^^^^^^^^^^^

- Fix crash when calling the constructor of an invalid class.
  (#GH10518) (#GH67914) (#GH78388)
- Fix crash when using lifetimebound attribute in function with trailing return.
  (#GH73619)
- Addressed an issue where constraints involving injected class types are perceived
  distinct from its specialization types. (#GH56482)
- Fixed a bug where variables referenced by requires-clauses inside
  nested generic lambdas were not properly injected into the constraint scope. (#GH73418)
- Fixed a crash where substituting into a requires-expression that refers to function
  parameters during the equivalence determination of two constraint expressions.
  (#GH74447)
- Fixed deducing auto& from const int in template parameters of partial
  specializations. (#GH77189)
- Fix for crash when using a erroneous type in a return statement.
  (#GH63244) (#GH79745)
- Fixed an out-of-bounds error caused by building a recovery expression for ill-formed
  function calls while substituting into constraints. (#GH58548)
- Fix incorrect code generation caused by the object argument
  of ``static operator()`` and ``static operator[]`` calls not being evaluated. (#GH67976)
- Fix crash and diagnostic with const qualified member operator new.
  Fixes (#GH79748)
- Fixed a crash where substituting into a requires-expression that involves parameter packs
  during the equivalence determination of two constraint expressions. (#GH72557)
- Fix a crash when specializing an out-of-line member function with a default
  parameter where we did an incorrect specialization of the initialization of
  the default parameter. (#GH68490)
- Fix a crash when trying to call a varargs function that also has an explicit object parameter.
  Fixes (#GH80971)
- Reject explicit object parameters on `new` and `delete` operators. (#GH82249)
- Fix a crash when trying to call a varargs function that also has an explicit object parameter. (#GH80971)
- Fixed a bug where abbreviated function templates would append their invented template parameters to
  an empty template parameter lists.
- Fix parsing of abominable function types inside type traits.
  Fixes (`#77585 <https://github.com/llvm/llvm-project/issues/77585>`_)
- Clang now classifies aggregate initialization in C++17 and newer as constant
  or non-constant more accurately. Previously, only a subset of the initializer
  elements were considered, misclassifying some initializers as constant. Partially fixes
  #GH80510.
- Clang now ignores top-level cv-qualifiers on function parameters in template partial orderings. (#GH75404)
- No longer reject valid use of the ``_Alignas`` specifier when declaring a
  local variable, which is supported as a C11 extension in C++. Previously, it
  was only accepted at namespace scope but not at local function scope.
- Clang no longer tries to call consteval constructors at runtime when they appear in a member initializer. (#GH82154)
- Fix crash when using an immediate-escalated function at global scope. (#GH82258)
- Correctly immediate-escalate lambda conversion functions. (#GH82258)
- Fixed an issue where template parameters of a nested abbreviated generic lambda within
  a requires-clause lie at the same depth as those of the surrounding lambda. This,
  in turn, results in the wrong template argument substitution during constraint checking.
  (#GH78524)
- Clang no longer instantiates the exception specification of discarded candidate function
  templates when determining the primary template of an explicit specialization.
- Fixed a crash in Microsoft compatibility mode where unqualified dependent base class
  lookup searches the bases of an incomplete class.
- Fix a crash when an unresolved overload set is encountered on the RHS of a ``.*`` operator.
  (#GH53815)
- In ``__restrict``-qualified member functions, attach ``__restrict`` to the pointer type of
  ``this`` rather than the pointee type.
  Fixes (#GH82941), (#GH42411) and (#GH18121).
- Clang now properly reports supported C++11 attributes when using
  ``__has_cpp_attribute`` and parses attributes with arguments in C++03 (#GH82995)
- Clang now properly diagnoses missing 'default' template arguments on a variety
  of templates. Previously we were diagnosing on any non-function template
  instead of only on class, alias, and variable templates, as last updated by
  CWG2032. Fixes (#GH83461)
- Fixed an issue where an attribute on a declarator would cause the attribute to
  be destructed prematurely. This fixes a pair of Chromium that were brought to
  our attention by an attempt to fix in (#GH77703). Fixes (#GH83385).
- Fix evaluation of some immediate calls in default arguments.
  Fixes (#GH80630)
- Fixed an issue where the ``RequiresExprBody`` was involved in the lambda dependency
  calculation. (#GH56556), (#GH82849).
- Fix a bug where overload resolution falsely reported an ambiguity when it was comparing
  a member-function against a non member function or a member-function with an
  explicit object parameter against a member function with no explicit object parameter
  when one of the function had more specialized templates.
  Fixes (`#82509 <https://github.com/llvm/llvm-project/issues/82509>`_)
  and (`#74494 <https://github.com/llvm/llvm-project/issues/74494>`_)

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

- Do not attempt to dump the layout of dependent types or invalid declarations
  when ``-fdump-record-layouts-complete`` is passed.
  Fixes (`#83684 <https://github.com/llvm/llvm-project/issues/83684>`_).

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

- ARMv7+ targets now default to allowing unaligned access, except Armv6-M, and
  Armv8-M without the Main Extension. Baremetal targets should check that the
  new default will work with their system configurations, since it requires
  that SCTLR.A is 0, SCTLR.U is 1, and that the memory in question is
  configured as "normal" memory. This brings Clang in-line with the default
  settings for GCC and Arm Compiler. Aside from making Clang align with other
  compilers, changing the default brings major performance and code size
  improvements for most targets. We have not changed the default behavior for
  ARMv6, but may revisit that decision in the future. Users can restore the old
  behavior with -m[no-]unaligned-access.
- An alias identifier (rdma) has been added for targeting the AArch64
  Architecture Extension which uses Rounding Doubling Multiply Accumulate
  instructions (rdm). The identifier is available on the command line as
  a feature modifier for -march and -mcpu as well as via target attributes
  like ``target_version`` or ``target_clones``.

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

- PTX is no longer included by default when compiling for CUDA. Using 
  ``--cuda-include-ptx=all`` will return the old behavior.

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

Fixed Point Support in Clang
----------------------------

- Support fixed point precision macros according to ``7.18a.3`` of
  `ISO/IEC TR 18037:2008 <https://standards.iso.org/ittf/PubliclyAvailableStandards/c051126_ISO_IEC_TR_18037_2008.zip>`_.

AST Matchers
------------

- ``isInStdNamespace`` now supports Decl declared with ``extern "C++"``.
- Add ``isExplicitObjectMemberFunction``.

clang-format
------------

- ``AlwaysBreakTemplateDeclarations`` is deprecated and renamed to
  ``BreakTemplateDeclarations``.
- ``AlwaysBreakAfterReturnType`` is deprecated and renamed to
  ``BreakAfterReturnType``.

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

- ``-fsanitize=signed-integer-overflow`` now instruments signed arithmetic even
  when ``-fwrapv`` is enabled. Previously, only division checks were enabled.

  Users with ``-fwrapv`` as well as a sanitizer group like
  ``-fsanitize=undefined`` or ``-fsanitize=integer`` enabled may want to
  manually disable potentially noisy signed integer overflow checks with
  ``-fno-sanitize=signed-integer-overflow``

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
