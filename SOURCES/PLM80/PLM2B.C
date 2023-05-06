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

static byte opParamsCnt; // lifted to file scope

/*
  разбор одного параметра опкода T2
    parNo  - номер параметра
    parPtr - указатель на значение параметра
  подменяет относительные номера параметров, на абсолютные.
  например:
  34-й опкод: T2_NUMBER
  35-й опкод: T2_IDENTIFIER
  36-й опкод: T2_COLONEQUALS, 1, 2
  получаем:
    T2_COLONEQUALS, 35, 34
    то есть присвоить идентификатору (35) значение числа (34)
*/
static void parseCodeParam(byte parNo, wpointer parPtr)
{
    word p;

    if (*parPtr != 0)
    {
        if (parNo > opParamsCnt)
            *parPtr = 0;
        else {
            p = *parPtr;
            *parPtr = tx2qp;

            while (p != 0)
            {
                p = p - 1;
                *parPtr = *parPtr - 1;
                if (tx2opc[*parPtr] == T2_LINEINFO)
                    if (tx2op2[*parPtr] == 0)
                        if (tx2op3[*parPtr] != 0)
                            p = p - (tx2op3[*parPtr] - tx2op1[*parPtr]);
            }
        }
    }
}


static void Sub_68E8()
{
    byte i;

    opParamsCnt = curOpFlags & 3;
    if ((curOpFlags & 4) != 0)
    {
        // T2_JMPFALSE
        tx2op2[1] = tx2op1[tx2qp];
        tx2op1[tx2qp] = 1;
    } else
        parseCodeParam(1, &tx2op1[tx2qp]);

    parseCodeParam(2, &tx2op2[tx2qp]);
    if (opParamsCnt == 3)
    {
        if (curOp == T2_CALL)
            tx2op3[tx2qp] = tx2op3[tx2qp] + botInfo;
        else if (curOp == T2_BYTEINDEX || curOp == T2_WORDINDEX)
        {
            i = (byte)tx2op1[tx2qp];
            tx2op2[i] = tx2op2[i] + tx2op3[tx2qp] * getIdnSize(tx2op1[i]);
        }
        else
            parseCodeParam(3, &tx2op3[tx2qp]);
    }
    tx2Aux1b[tx2qp] = 0xc;
    tx2Aux2b[tx2qp] = 9;
}

static void m2p1_CorrectIdnOrNum()
{
    if (curOp == T2_IDENTIFIER)
    {
        tx2op1[tx2qp] = curInfoP = tx2op1[tx2qp] + botInfo;
        if (TestInfoFlag(F_MEMBER))
        {
            curInfoP = GetParentOffset();
            tx2Aux2b[tx2qp] = 4;
        }
        else if (TestInfoFlag(F_AUTOMATIC))
            tx2Aux2b[tx2qp] = 0xa;
        else
            tx2Aux2b[tx2qp] = 4;
        curInfoP = tx2op1[tx2qp];
        tx2op2[tx2qp] = GetLinkVal();
        // получаем размерность идентификатора: 0 - 8 bit, 1 - 16 bit, 2 - 32 bit
        tx2Aux1b[tx2qp] = aux1IdnType[GetType()];
    } else if (curOp <= T2_BIGNUMBER)
    {
        // T2_NUMBER || T2_BIGNUMBER
        tx2op2[tx2qp] = tx2op1[tx2qp];
        tx2Aux2b[tx2qp] = 8;
        tx2op1[tx2qp] = 0;
        if (curOp == T2_BIGNUMBER)
        {
            // переводим в NUMBER
            tx2Aux1b[tx2qp] = 1;        // выставляем флаг 16-битного числа
            tx2opc[tx2qp] = T2_NUMBER;
        } else
            tx2Aux1b[tx2qp] = 0;        // иначе число 8-ми битное
    } else {
        tx2Aux1b[tx2qp] = 0;
        tx2op2[tx2qp] = 0;
    }
}

void m2_Parse1()
{
    for (tx2qp = 4; tx2qp <= tx2lastPos - 1; tx2qp++)
    {
        curOp = tx2opc[tx2qp];
        curOpFlags = flagsT2[curOp];
        if ((curOpFlags & 0xc0) == 0)
            Sub_68E8();
        else if ((curOpFlags & 0xc0) == 0x40)
        {
            /*
              это идентификатор или число-константа
            */
            m2p1_CorrectIdnOrNum();
        }

    }
}

