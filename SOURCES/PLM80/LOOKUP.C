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


#include "plm.h"

/*
  ������ ��� ����� �����䨪���
*/
byte Hash(pointer pstr)
{
    pointer p = pstr;
    byte len = *p;
    byte hash = 0;

    while (len != 0) {
        hash = (hash << 1) + (hash & 0x80 ? 1 : 0) + *p++;
        len = len - 1;
    }
    return hash & 0x3F;
}



/*
  �������� � ⠡���� ���-����� ����� ���
*/
void Lookup(pointer pstr)
{
    offset_t p, q, r;
    word hval;
    byte cmp;

    hval = Hash(pstr);
    curSymbolP = hashChains[hval & 0x3F];
    p = 0;
    while (curSymbolP != 0)
    {
        /*
          � ⠡��� 㦥 ���� ��� � ⠪�� �� ��襬
        */
        if (SymbolP(curSymbolP)->name[0] == pstr[0])
        {
            cmp = Strncmp(&SymbolP(curSymbolP)->name[1], pstr + 1, pstr[0]);
            if (cmp == 0)
            {
                /*
                  ⠪�� ��� 㦥 ���� � ⠡���
                */
                if (p != 0 )
                {
                    /*
                      ��६�頥� ��� �� ��ࢮ� ���� � ⠡���
                    */
                    q = SymbolP(curSymbolP)->link;
                    r = curSymbolP;
                    curSymbolP = p;
                    SymbolP(curSymbolP)->link = q;
                    curSymbolP = r;
                    SymbolP(curSymbolP)->link = hashChains[hval];
                    hashChains[hval] = curSymbolP;
                }
                return;
            }
        }
        p = curSymbolP;
        curSymbolP = SymbolP(curSymbolP)->link;
    }
    /*
      ᮧ��� ����� ������ � ⠡���
    */
    Alloc(0, pstr[0] + 1);
    curSymbolP = AllocSymbol(sizeof(sym_t) + pstr[0]);
    memmove(SymbolP(curSymbolP)->name, pstr, pstr[0] + 1);
    SymbolP(curSymbolP)->infoP = 0;
    SymbolP(curSymbolP)->link = hashChains[hval];
    hashChains[hval] = curSymbolP;
} /* Lookup() */
