============================
LLVM |release| Release Notes
============================

.. contents::
    :local:

.. only:: PreRelease

  .. warning::
     These are in-progress notes for the upcoming LLVM |version| release.
     Release notes for previous releases can be found on
     `the Download Page <https://releases.llvm.org/download.html>`_.


Introduction
============

This document contains the release notes for the LLVM Compiler Infrastructure,
release |release|.  Here we describe the status of LLVM, including major improvements
from the previous release, improvements in various subprojects of LLVM, and
some of the current users of the code.  All LLVM releases may be downloaded
from the `LLVM releases web site <https://llvm.org/releases/>`_.

For more information about LLVM, including information about the latest
release, please check out the `main LLVM web site <https://llvm.org/>`_.  If you
have questions or comments, the `Discourse forums
<https://discourse.llvm.org>`_ is a good place to ask
them.

Note that if you are reading this file from a Git checkout or the main
LLVM web page, this document applies to the *next* release, not the current
one.  To see the release notes for a specific release, please see the `releases
page <https://llvm.org/releases/>`_.

Non-comprehensive list of changes in this release
=================================================
.. NOTE
   For small 1-3 sentence descriptions, just add an entry at the end of
   this list. If your description won't fit comfortably in one bullet
   point (e.g. maybe you would like to give an example of the
   functionality, or simply have a lot to talk about), see the `NOTE` below
   for adding a new subsection.

* ...

Update on required toolchains to build LLVM
-------------------------------------------

Changes to the LLVM IR
----------------------

Changes to LLVM infrastructure
------------------------------

Changes to building LLVM
------------------------

Changes to TableGen
-------------------

- We can define type aliases via new keyword ``deftype``.

Changes to Interprocedural Optimizations
----------------------------------------

Changes to the AArch64 Backend
------------------------------

* Added support for Cortex-A520, Cortex-A720 and Cortex-X4 CPUs.

* Neoverse-N2 was incorrectly marked as an Armv8.5a core. This has been
  changed to an Armv9.0a core. However, crypto options are not enabled
  by default for Armv9 cores, so `-mcpu=neoverse-n2+crypto` is now required
  to enable crypto for this core. As far as the compiler is concerned,
  Armv9.0a has the same features enabled as Armv8.5a, with the exception
  of crypto.

* Assembler/disassembler support has been added for 2023 architecture
  extensions.

* Support has been added for Stack Clash Protection. During function frame
  creation and dynamic stack allocations, the compiler will issue memory
  accesses at reguilar intervals so that a guard area at the top of the stack
  can't be skipped over.

Changes to the AMDGPU Backend
-----------------------------

Changes to the ARM Backend
--------------------------

Changes to the AVR Backend
--------------------------

Changes to the DirectX Backend
------------------------------

Changes to the Hexagon Backend
------------------------------

Changes to the LoongArch Backend
--------------------------------

Changes to the MIPS Backend
---------------------------

Changes to the PowerPC Backend
------------------------------

* LLJIT's JIT linker now defaults to JITLink on 64-bit ELFv2 targets.
* Initial-exec TLS model is supported on AIX.
* Implemented new resource based scheduling model of POWER7 and POWER8.
* ``frexp`` libcall now references correct symbol name for ``fp128``.
* Optimized materialization of 64-bit immediates, code generation of
  ``vec_promote`` and atomics.
* Global constant strings are pooled in the TOC under one entry to reduce the
  number of entries in the TOC.
* Added a number of missing Power10 extended mnemonics.
* Added the SCV instruction.
* Fixed register class for the paddi instruction.
* Optimize VPERM and fix code order for swapping vector operands on LE.
* Added various bug fixes and code gen improvements.

AIX Support/improvements:

* Support for a non-TOC-based access sequence for the local-exec TLS model (called small local-exec).
* XCOFF toc-data peephole optimization and bug fixes.
* Move less often used __ehinfo TOC entries to the end of the TOC section.
* Fixed problems when the AIX libunwind unwinds starting from a signal handler
  and the function that raised the signal happens to be a leaf function that
  shares the stack frame with its caller or a leaf function that does not store
  the stack frame backchain.

Changes to the RISC-V Backend
-----------------------------

* Added assembler/disassembler support for the experimental Zabha (Byte and
  Halfword Atomic Memory Operations) extension.
* Added assembler/disassembler support for the experimenatl Zalasr
  (Load-Acquire and Store-Release) extension.
* The names of the majority of the S-prefixed (supervisor-level) extension
  names in the RISC-V profiles specification are now recognised.

Changes to the WebAssembly Backend
----------------------------------

Changes to the Windows Target
-----------------------------

Changes to the X86 Backend
--------------------------

Changes to the OCaml bindings
-----------------------------

Changes to the Python bindings
------------------------------

Changes to the C API
--------------------

Changes to the CodeGen infrastructure
-------------------------------------

Changes to the Metadata Info
---------------------------------

Changes to the Debug Info
---------------------------------

Changes to the LLVM tools
---------------------------------

* llvm-symbolizer now treats invalid input as an address for which source
  information is not found.
* Fixed big-endian support in llvm-symbolizer's DWARF location parser.
* llvm-readelf now supports ``--extra-sym-info`` (``-X``) to display extra
  information (section name) when showing symbols.

* ``llvm-nm`` now supports the ``--line-numbers`` (``-l``) option to use
  debugging information to print symbols' filenames and line numbers.

* llvm-symbolizer and llvm-addr2line now support addresses specified as symbol names.

* llvm-objcopy now supports ``--gap-fill`` and ``--pad-to`` options, for
  ELF input and binary output files only.

* Supported parsing XCOFF auxiliary symbols in obj2yaml.

* ``llvm-ranlib`` now supports ``-X`` on AIX to specify the type of object file
  ranlib should examine.

* ``llvm-nm`` now supports ``--export-symbol`` to ignore the import symbol file.

* llvm-rc and llvm-windres now accept file path references in ``.rc`` files
  concatenated from multiple string literals.

* The llvm-windres option ``--preprocessor`` now resolves its argument
  in the PATH environment variable as expected, and options passed with
  ``--preprocessor-arg`` are placed before the input file as they should
  be.

* The llvm-windres option ``--preprocessor`` has been updated with the
  breaking behaviour change from GNU windres from binutils 2.36, where
  the whole argument is considered as one path, not considered as a
  sequence of tool name and parameters.

Changes to LLDB
---------------------------------

* ``SBWatchpoint::GetHardwareIndex`` is deprecated and now returns -1
  to indicate the index is unavailable.
* Methods in SBHostOS related to threads have had their implementations
  removed. These methods will return a value indicating failure.
* ``SBType::FindDirectNestedType`` function is added. It's useful
  for formatters to quickly find directly nested type when it's known
  where to search for it, avoiding more expensive global search via
  ``SBTarget::FindFirstType``.
* ``lldb-vscode`` was renamed to ``lldb-dap`` and and its installation
  instructions have been updated to reflect this. The underlying functionality
  remains unchanged.
* The ``mte_ctrl`` register can now be read from AArch64 Linux core files.
* LLDB on AArch64 Linux now supports debugging the Scalable Matrix Extension
  (SME) and Scalable Matrix Extension 2 (SME2) for both live processes and core
  files. For details refer to the
  `AArch64 Linux documentation <https://lldb.llvm.org/use/aarch64-linux.html>`_.
* LLDB now supports symbol and binary acquisition automatically using the
  DEBUFINFOD protocol. The standard mechanism of specifying DEBUFINOD servers in
  the ``DEBUGINFOD_URLS`` environment variable is used by default. In addition,
  users can specify servers to request symbols from using the LLDB setting
  ``plugin.symbol-locator.debuginfod.server_urls``, override or adding to the
  environment variable.


* When running on AArch64 Linux, ``lldb-server`` now provides register
  field information for the following registers: ``cpsr``, ``fpcr``,
  ``fpsr``, ``svcr`` and ``mte_ctrl``. ::

    (lldb) register read cpsr
          cpsr = 0x80001000
               = (N = 1, Z = 0, C = 0, V = 0, SS = 0, IL = 0, <...>

  This is only available when ``lldb`` is built with XML support.
  Where possible the CPU's capabilities are used to decide which
  fields are present, however this is not always possible or entirely
  accurate. If in doubt, refer to the numerical value.

* On Windows, LLDB can now read the thread names.

Changes to Sanitizers
---------------------
* HWASan now defaults to detecting use-after-scope bugs.

Changes to the Profile Runtime
------------------------------

* Public header ``profile/instr_prof_interface.h`` is added to declare four
  API functions to fine tune profile collection.

Other Changes
-------------

External Open Source Projects Using LLVM 19
===========================================

* A project...

Additional Information
======================

A wide variety of additional information is available on the `LLVM web page
<https://llvm.org/>`_, in particular in the `documentation
<https://llvm.org/docs/>`_ section.  The web page also contains versions of the
API documentation which is up-to-date with the Git version of the source
code.  You can access versions of these documents specific to this release by
going into the ``llvm/docs/`` directory in the LLVM tree.

If you have any questions or comments about LLVM, please feel free to contact
us via the `Discourse forums <https://discourse.llvm.org>`_.
