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


#include "asm80.h"

/* packToken - packs the token pointed by tokStart, with length toksiz into 4 bytes
 packed version replaces original and toksize set to 4 bytes

 "уплотняет" символы в диапазон [0..39]:
      0                  --> 0
      '0'..'9'           --> 1..10
      '?','@','A'..'Z'   --> 11..38
      '_'                --> 39
*/
static byte Pack1(byte i)
{
    return  i >= tokenSize[0] ? 0 : tokStart[0][i] < 0x3f ? tokStart[0][i] - 0x2f : tokStart[0][i] < '_' ? tokStart[0][i] - 0x34: tokStart[0][i] - 0x38;
}


void PackToken()
{
    if (tokenSize[0] < MAXSYMSIZE)
    {
        memset(&tokStart[0][tokenSize[0]], 0, MAXSYMSIZE - tokenSize[0]);
    }
    *(word *)tokStart[0]       = (Pack1(0) * 40 + Pack1(1)) * 40 + Pack1(2);
    *(word *)(tokStart[0] + 2) = (Pack1(3) * 40 + Pack1(4)) * 40 + Pack1(5);
    *(word *)(tokStart[0] + 4) = (Pack1(6) * 40 + Pack1(7)) * 40 + Pack1(8);
    *(word *)(tokStart[0] + 6) = (Pack1(9) * 40 + Pack1(10)) * 40 + Pack1(11);
    *(word *)(tokStart[0] + 8) = (Pack1(12) * 40 + Pack1(13)) * 40 + Pack1(14);
    *(word *)(tokStart[0] +10) = (Pack1(15) * 40 + Pack1(16)) * 40 + Pack1(17);
    tokenSize[0] = MAXPACKSIZE;
}
