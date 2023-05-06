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


static byte bC259, bC25A;   // lifted to file scope


static void Sub_6C54(byte arg1b)
{
    if (arg1b != 0)
        tx2Auxw[arg1b] = tx2Auxw[arg1b] + 1;
}



static void Sub_6C73(byte arg1b)
{
    if (OPTIMIZE)
        while (tx2qp - 1 > arg1b) {
            arg1b = arg1b + 1;
            if (tx2opc[arg1b] == curOp) {
                if (tx2op1[arg1b] == tx2op1[tx2qp]) {
                    if (tx2op2[arg1b] == tx2op2[tx2qp]) {
                        if (tx2Aux1b[arg1b] == tx2Aux1b[tx2qp]) {
                            if (tx2op3[arg1b] != 0xff00) {
                                tx2opc[tx2qp] = T2_OPTBACKREF;
                                tx2op1[tx2qp] = arg1b;
                                return;
                            }
                        }
                    }
                }
            }
        }
    if ((curOpFlags & 0xc0) == 0) {
        Sub_6C54((byte)tx2op1[tx2qp]);
        Sub_6C54((byte)tx2op2[tx2qp]);
    }
    tx2op3[tx2qp] = tx2qp;
}


static bool isPtrVar;  // указатель (BASED, AT, INDEX и тд)

static void Sub_6EAB(wpointer arg1wP)
{
    if (*arg1wP != 0) {
        if (tx2opc[*arg1wP] == T2_OPTBACKREF)
            *arg1wP = tx2op1[*arg1wP];
    }
}


static void Sub_6F20(byte arg1b)
{
    byte i;
    if ((i = tx2opc[arg1b]) == T2_BASED)
        isPtrVar = true;
    else if (i == T2_BYTEINDEX || i == T2_WORDINDEX) {
        if (tx2opc[tx2op2[arg1b]] != T2_NUMBER)
            isPtrVar = true;
        else {
            curInfoP = tx2op1[tx2op1[arg1b]];
            if (tx2op2[tx2op2[arg1b]] >= GetDimension() || TestInfoFlag(F_AT))
            isPtrVar = true;
        }
    }
    else if (i == T2_IDENTIFIER) {
        curInfoP = tx2op1[arg1b];
        if (TestInfoFlag(F_AT))
            isPtrVar = true;
    }
} /* Sub_6F20() */

static void Sub_6EE1(byte arg1b)
{
    isPtrVar = false;
    if (tx2opc[arg1b] == T2_MEMBER) {
        Sub_6F20((byte)tx2op1[arg1b]);
        Sub_6F20((byte)tx2op2[arg1b]);
    }
    else
        Sub_6F20(arg1b);
}


static byte bC263;

static void Sub_7018(byte arg1b)
{
    if (tx2opc[arg1b] != T2_CALL)
        arg1b = (byte)tx2op3[arg1b];
    if (arg1b != 0)
        if (arg1b > bC263)
            bC263 = arg1b;
}

static byte Sub_6FE2()
{
    if (isPtrVar)
        bC263 = bC259;
    else
        bC263 = bC25A;

    Sub_7018((byte)tx2op1[tx2qp]);
    Sub_7018((byte)tx2op2[tx2qp]);
    return bC263;
}


static bool sub_70BC(byte arg1b)
{
    Sub_6EE1((byte)tx2op1[arg1b]);
    if (isPtrVar)
        return true;
    if (tx2op1[arg1b] == 0xac) {
        curInfoP = tx2op1[tx2op1[arg1b]];
        if (TestInfoFlag(F_AUTOMATIC))
            return true;
    }
    return false;
}

void Sub_7055()
{
    if (sub_70BC(tx2qp))
        bC25A = tx2qp;
    if (tx2op3[tx2op1[bC259 = tx2qp]] != 0xff00)
        tx2op3[tx2op1[tx2qp]] = tx2qp;
    Sub_6C54((byte)tx2op1[tx2qp]);
    Sub_6C54((byte)tx2op2[tx2qp]);
}

static void Sub_6D52()
{
    Sub_6EAB(&tx2op1[tx2qp]);
    Sub_6EAB(&tx2op2[tx2qp]);
    if (curOp == T2_COLONEQUALS)
        Sub_7055();
    else if (procCallDepth > 0) {
        Sub_6C54((byte)tx2op1[tx2qp]);
        Sub_6C54((byte)tx2op2[tx2qp]);
        if (curOp == T2_CALL)
            procCallDepth = procCallDepth - 1;
        else if (curOp == T2_MOVE || curOp == T2_CALLVAR) {
            procCallDepth = procCallDepth - 1;
            Sub_6EAB(&tx2op3[tx2qp]);
            Sub_6C54((byte)tx2op3[tx2qp]);
        }
        else
            tx2op3[tx2qp] = 0xff00;
    } else if (curOp == T2_OUTPUT || curOp == T2_TIME) {
        Sub_6C54((byte)tx2op1[tx2qp]);
        Sub_6C54((byte)tx2op2[tx2qp]);
    } else {
        tx2op3[tx2qp] = 0;
        Sub_6EE1(tx2qp);
        Sub_6C73(Sub_6FE2());
        if (curOp == T2_JMPFALSE) {
            if (tx2opc[tx2qp - 1] == T2_NOT) {
                boC20F = true;
                tx2op2[tx2qp] = tx2op1[tx2qp - 1];
                copyTx2Code(tx2qp, tx2qp - 1);
                tx2opc[tx2qp] = T2_SEMICOLON;
            }
        }
    }
}



/*
  обработка опкодов T2_NUMBER, T2_IDENTIFIER, T2_LENGTH, T2_SIZE
*/
static void m2p2_doIdnOrNum()
{
    if (procCallDepth > 0)
    tx2op3[tx2qp] = 0xff00;
    else {
        tx2op3[tx2qp] = 0;
        if (curOp == T2_IDENTIFIER)
        {
            curInfoP = tx2op1[tx2qp];
            if (TestInfoFlag(F_AT))
                Sub_6C73(bC259);
            else
                Sub_6C73(bC25A);
        } else
            Sub_6C73(bC25A);
    }
}

void m2_Parse2()
{
    bC259 = 4;
    bC25A = 4;
    for (tx2qp = 4; tx2qp <= tx2lastPos - 1; tx2qp++)
    {
        tx2Auxw[tx2qp] = 0;
        curOp = tx2opc[tx2qp];
        curOpFlags = flagsT2[curOp];
        if ((curOpFlags & 0xc0) == 0)
            Sub_6D52();
        else if ((curOpFlags & 0xc0) == 0x40)
            m2p2_doIdnOrNum();
        if (curOp == T2_BEGCALL || curOp == T2_BEGMOVE)
            procCallDepth = procCallDepth + 1;

    }
}
