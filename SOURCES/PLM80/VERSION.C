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
}
