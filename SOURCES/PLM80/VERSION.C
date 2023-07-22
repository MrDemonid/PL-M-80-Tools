/****************************************************************************
 *  plm80 v 5.0                                                             *
 *  Copyright (C) 2023 Andrey Hlus                                          *
 *                                                                          *
 *  Created based on:                                                       *
 *  plm80: C port of Intel's ISIS-II PLM80 v4.0                             *
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
#include <stdbool.h>
#include <limits.h>

#include "plm.h"



// primary control names
byte szHelp[] = "Primary controls:\n"
" PRINT[(pathname)]/NOPRINT   The control specifies that printed output is to\n"
"                             be produced.\n"
" XREF/NOXREF                 The control specifies that a cross-reference\n"
"                             listing of source program identifiers is to be\n"
"                             produced on the listing file.\n"
" SYMBOLS/NOSYMBOLS           The control specifies that a listing of all\n"
"                             identifiers in the source program and their\n"
"                             attributes is to be produced on the listing file.\n"
" DEBUG/NODEBUG               The control specifies that the object module is\n"
"                             to contain the name and relative address of each\n"
"                             symbol whose address is known at compile-time,\n"
"                             and the statement number and relative address of\n"
"                             each source program statement.\n"
" PAGING/NOPAGING             The control specifies that the listed output is\n"
"                             to be formatted onto pages.\n"
" PAGELENGTH(length)          The control specifies the maximum number of\n"
"                             lines to be printed per page of listing output.\n"
" PAGEWIDTH(width)            The control specifying the maximum line width,\n"
"                             in characters, to be used for listing output.\n"
" DATE(date)                  Where date is any sequence of nine or fewer\n"
"                             characters not containing parentheses. The date\n"
"                             appears in the heading of all pages of listing\n"
"                             output.\n"
" TITLE('title')              Where 'title' is a sequence of printable ASCII\n"
"                             characters which are enclosed in quotes. The\n"
"                             sequence is placed in the title line of each\n"
"                             page of listed output.\n"
" OBJECT[(pathname)]/NOOBJECT The control specifies that an object module is\n"
"                             to be created during the compilations.\n"
" OPTIMIZE/NOOPTIMIZE         The control allows optimizations when generating\n"
"                             object code.\n"
" WORKFILES(:dev:,:dev:)      The control allows you to specify any two devices\n"
"                             for storage of work files, which are deleted at\n"
"                             the end of compilation. Default device is a :F1:.\n"
" INTVECTOR/NOINTVECTOR       Under the INVECTOR control, the compiler creates\n"
"                             an interrupt vector consisting of a 4-byte entry\n"
"                             for each interrupt procedure in the module.\n"
" IXREF[(pathname)]/NOIXREF   The control causes an 'intermediate intermodule\n"
"                             cross-reference file' to be produced and written\n"
"                             out to the file specified by the pathname.\n"
"General controls:\n"
" LIST/NOLIST                 The control specifies that listing of the source\n"
"                             program.\n"
" CODE/NOCODE                 The control specifies that listing of the\n"
"                             generated object code, in standart assembly\n"
"                             language format.\n"
" EJECT                       It causes printing of the current page to\n"
"                             terminate and a new page to be started.\n"
" LEFTMARGIN(column)          The control specifies the left margin of the\n"
"                             source input.\n"
" INCLUDE                     The control causes subsequent source lines to be\n"
"                             input from the specified file.\n"
" SAVE/RESTORE                The control allow the setting of certain general\n"
"                             controls to be saved on a stack, and\n"
"                             restores from the stack.\n"
" SET(switch assignemt list)  Set the conditional variables.\n"
"                                Example: $SET(VAR1, VAR2=12).\n"
"                                This example sets the switch VAR1 to\n"
"                                'TRUE' (0FFh) and the switch VAR2 to 12.\n"
" RESET(switch list)          Each switch in the switch list is set\n"
"                             to 'FALSE' (0).\n"
" COND/NOCOND                 These controls determine whether text within\n"
"                             an $IF element will appear in the listing or it\n"
"                             is skipped.\n"
"Where:\n"
"  pathname - is a standard ISIS-II pathname with specifies the file or device.\n"
"  :dev:    - is a standard ISIS-II device, emulated throught system\n"
"             environment variables.\n";




// primary control names



// use the following function declaration in the main code

void showVersion(FILE *fp, bool full)
{
    fprintf(fp, "ISIS-II PL/M-80 COMPILER %s  (C) 2023 Andrey Hlus\n", verNo);
    fputs("Created based on:\n", fp);
    fputs("C port of Intel's ISIS-II PLM80 v4.0 - 0.2.0.X  (C)2020 Mark Ogden\n", fp);
    if (full)
    {
        fputs("32 bit target - Git: untracked [2020-08-09] +untracked files\n", fp);
    }
    fprintf(fp, "Usage: PLM80.EXE sourcefile [controls]\n");
    fprintf(fp, "%s", szHelp);
}
