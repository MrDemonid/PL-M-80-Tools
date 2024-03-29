# PLM-80-Tools
Cross-compilation tools PL/M for 8080.

Based on:
  C ports of PL/M 80 and Fortran applications
  Copyright (C) 2020 Mark Ogden <mark.pm.ogden@btinternet.com>.
  https://github.com/ogdenpm/c-ports.git

Modified programs have been assigned new versions, so that
there is no confusion.


The code is adapted for the Open WATCOM compiler.

For compilation the sources code request environment variable 'WATCOM'.
If the environment variables are not set, then you need to set
the 'DRIVE' variable in the file MAKE.BAT, specifying the disk with
the WATCOM compiler installed.



New features:

ASM80:
  - Fixed all found errors.
  - The maximum file name is now 8 characters.
    In the original - 6 characters.
  - The maximum length of identifiers is 18 characters.
    In the original - 6 characters.
  - The '_' character is allowed in identifier names.
  - The stack of identifiers has been increased from 8 to 16 elements.
    This allows you to specify more elements in one line of
    the directives 'DB' and 'DW'.
  - File paths are set via environment variables,
    such as :F1: - :F9:, as it was done in the original
    compiler from Intel, for the ISIS system.
  - Works correctly with Russian ascii characters.
  - Return results in ERRORLEVEL.
  - Print to screen source file name.
  - Fixed bug with opening a file in UPDATE mode.
  - Fixed a bug in the XREF module.
  - Support for source code files in UNIX format.
  - Added help output on compiler options.

LINK:
  - Fixed all found errors.
  - The ability to transfer parameters via a file.
    Example:
      link @:f1:fileparams.txt
  - The maximum file name is now 8 characters.
  - Return results in ERRORLEVEL.
  - Print to screen source file name.

LOCATE:
  - Fixed all found errors.
  - The maximum file name is now 8 characters.
  - Return results in ERRORLEVEL.
  - Print to screen source file name.

OBJCPM:
  - Support for paths as in ISIS-II
  - The maximum file name is now 8 characters.
  - Return results in ERRORLEVEL.

PLM80:
  - Fixed all found errors.
  - The maximum file name is now 8 characters.
  - Added two words: 'Break' and 'Continue', for cycles.
  - The '_' character is allowed in identifier names, as in PL/M-286.
  - The page length for the listing file has been expanded from 255 to 65535 lines.
  - Works correctly with Russian ascii characters.
  - File paths are set via environment variables,
    such as :F1: - :F9:, as it was done in the original
    compiler from Intel, for the ISIS system.
  - Return results in ERRORLEVEL.
  - Print to screen source file name.
  - Added a new cycle DO-UNTIL to the language.
  - Increased the working memory of the compiler, for large source code programs. 
  - Increased the maximum size of strings defined via the LITERALLY directive from 256 to 4096 characters.
  - Increased the number of elements in structures from 32 to 256.
  - Added help output on compiler options.


PS: File SETENV.BAT make PLM80 environment. My all PL/M-programs request this
variable for compilation.
