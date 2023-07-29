/****************************************************************************
 *  asm80 v 5.1                                                             *
 *  Copyright (C) 2023 Andrey Hlus                                          *
 *                                                                          *
 *  Created based on:                                                       *
 *  asm80: C port of ASM80 v4.1                                             *
 *  Copyright (C) 2020 Mark Ogden <mark.pm.ogden@btinternet.com>            *
 *                                                                          *
 *  This program is free software; you can redistribute it and/or           *
 *  modify it under the terms of the GNU General Public License             *
 *  as published by the Free Software Foundation; either version 2          *
 *  of the License, or (at your option) any later version.                  *
 *                                                                          *
 *  This program is distributed in the hope that it will be useful,         *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with this program; if not, write to the Free Software             *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,              *
 *  MA  02110-1301, USA.                                                    *
 ****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <limits.h>


#include "asm80.h"


byte szHlp1[] = "Primary controls:\n"
                " OBJECT(pathname)*  An object code file is generated and is output to the\n"
                "                    specified file.\n"
                " NOOBJECT           Object code generation is suppressed.\n"
                " MOD85              The assembler assumes that 8080 code is being assembled\n"
                "                    unless 8085 instruction set is specified by this control.\n"
                " DEBUG              If an object file is requested, the symbol table is\n"
                "                    output to that file.\n"
                " NODEBUG*           The symbol table is  not included in the object file.\n"
                " PRINT(pathname)*   An assembly list file is generated and is output to the\n"
                "                    specified file.\n"
                " NOPRINT            The assembly output listing is suppressed.\n"
                " SYMBOLS*           If a listing file is opened by PRINT, the symbol table is\n"
                "                    output to the list file.\n"
                " NOSYMBOLS          The symbol table is not included in the list file created\n"
                "                    by PRINT.\n"
                " XREF               A symbol-cross-reference file is requested.\n"
                " NOXREF*            Symbol-cross-reference file generation is suppressed.\n";

byte szHlp2[] = "\r MACROFILE[(:dev:)] The macro definition file is directed to the specified\n"
                "                    device. If no device is specified, the device where the\n"
                "                    source file resides is used. MACROFILE also adds the\n"
                "                    macro reserved words (MACRO, LOCAL, etc.) to the reserved\n"
                "                    word list.\n"
                " NOMACROFILE*       No macro temporary files are created. If your source file\n"
                "                    contains macros, all definitions and calls cause errors.\n"
                " PAGELENGTH(n)      Each list file page is 'n' lines long, where 'n' must be\n"
                "                    at least 12. The default value is 66.\n"
                " PAGEWIDTH(n)       Each list file line can be up to 'n' characters long,\n"
                "                    where 'n' must be in the range [72..132]. The default\n"
                "                    page width is 120.\n"
                " PAGING*            The assembler separates listing into pages with headers\n"
                "                    at each page break.\n"
                " NOPAGING           The listing is not separated into pages.\n"
                " MACRODEBUG         Assembler-generated macro symbols are output to the list\n"
                "                    and object files when the symbol table is output.\n"
                " NOMACRODEBUG*      Assembler-generated macro symbols are not output to the\n"
                "                    list and object files.\n"
                " TTY                Simulates form-feed for teletypewriter output.\n"
                " NOTTY              No teletypewriter output (form-feed simulation).\n"
                "\n";
byte szHlp3[] = "\rGeneral controls:                                                        \n"
                " INCLUDE(pathname)  Subsequent source lines are input from a specified file\n"
                "                    until an end-of-file or nested INCLUDE os found.\n"
                " LIST*              An assembly output listing is generated and sent to the\n"
                "                    file specified by the PRINT control.\n"
                " NOLIST             Assembly listing is suppressed, except lines containing\n"
                "                    errors.\n"
                " COND*              Conditionally-skipped source code is included in the\n"
                "                    assembly listing if LIST is selected.\n"
                "                    The conditional-assembly dirictives are also listed.\n"
                " NOCOND             Listing of conditionally-skipped source code and\n"
                "                    conditional-assembly directives is suppressed.\n"
                " GEN*               Macro expansion source text generated by macro calls is\n"
                "                    listed if LIST is selected.\n"
                " NOGEN              Macro expansion source text listing is suppressed.\n";

byte szHlp4[] = "\r TITLE('string')    The specified string is printed in charecter positions\n"
                "                    1-64 of the second line of page headings. 'String' must\n"
                "                    not be null.\n"
                " EJECT              Spaces are skipped to the next top-of-form. The position\n"
                "                    of the next to-of-form is determined by PAGELENGTH, not\n"
                "                    by the physical top-of-form.\n"
                " SAVE               The current settings of the LIST, COND, and GEN controls\n"
                "                    are stacked (but remain valid until explicitly changed).\n"
                " RESTORE            The LIST, COND, and GEN control settings at the top of\n"
                "                    the stack are restored.\n"
                "Where:\n"
                "  *        - default control.\n"
                "  pathname - is a standard ISIS-II pathname with specifies the file or device.\n"
                "  :dev:    - is a standard ISIS-II device, emulated throught system\n"
                "             environment variables.\n";





void showVersion(FILE *fp, bool full)
{
    fprintf(fp, "ISIS-II ASM-80 %s  (C) 2023 Andrey Hlus\n", verNo);

    fputs("Created based on:\n", fp);
    fputs("C port of Intel's ISIS-II ASM80 v4.1 - 0.2.0.X  (C)2020 Mark Ogden\n", fp);
    if (full)
    {
        fputs("32 bit target - Git: untracked [2020-08-09] +untracked files\n", fp);
    }
    fprintf(fp, "\nUsage: ASM80.EXE sourcefile [controls [controls] [...]]\n");
    fprintf(fp, "%s", szHlp1);
    fprintf(fp, "press any key to continue...");
    fflush(fp);
    getch();
    fprintf(fp, "%s", szHlp2);
    fprintf(fp, "press any key to continue...");
    fflush(fp);
    getch();
    fprintf(fp, "%s", szHlp3);
    fprintf(fp, "press any key to continue...");
    fflush(fp);
    getch();
    fprintf(fp, "%s", szHlp4);
    fflush(fp);
}
