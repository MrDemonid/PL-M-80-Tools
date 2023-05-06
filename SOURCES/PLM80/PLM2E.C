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
#include <stdio.h>

static byte b7A81[4] = { 0x3C, 0x46, 0x50, 0x5A };

static byte bC28B[4], bC28F;

static void cfPutXCHG()
{
    EncodeFragData(CF_XCHG);
    curPC = curPC + 1;
    Sub_5B96(3, 4);
    Sub_5B96(2, 3);
    Sub_5B96(4, 2);
}

/*
  отправляет в буфер код CF_XTHL
*/
static void cfPutXTHL()
{
    cfCmdBuf[0] = 0xA;
    cfCmdBuf[1] = parInStack;
    EncodeFragData(CF_XTHL);
    curPC = curPC + 1;
    Sub_5C97(4);
    Sub_5C1D(3);
    Sub_5B96(4, 3);
}


static void Sub_7D7E(byte arg1b)
{
    if (arg1b <= 3)
        if (bC28B[arg1b] < 0xc8)
            bC28B[arg1b] = bC28B[arg1b] + bC28F;
}

void cf_7A85()
{
    byte i, reg, k, m, n;

    Sub_597E();
    for (i = 0; i <= 3; i++)
    {
        if (boC072[i] || boC069[i])
            bC28B[i] = 0xc8;
        else if (boC060[i])
            bC28B[i] = b7A81[i] + 0xf;
        else
            bC28B[i] = b7A81[i];
    }

    if ((bC0C3[parInStack] >> 4) != 0xb && bC140[parInStack] != 0)
        bC28B[0] = 0xC8;
    for (m = 0; m <= 1; m++)
    {
        if (curOpParams[m] != 0)
        {
            if (curTx2aux2b[m] == 9 && bC140[parInStack] == curOpParams[m])
            {
                k = m;
                bC28F = 0xce;
                boC1D8 = false;
            } else
                bC28F = 0x32;

            Sub_7D7E(b52B5[bC0BF[m]]);
            Sub_7D7E(b4C2D[bC0BD[m]] >> 5);
        }
    }

    n = 0xc8;
    for (i = 0; i <= 3; i++)
    {
        if (bC28B[i] <= n)
            n = bC28B[reg = i];
    }

    if (n == 0xC8)
    {
        if (boC069[3])
        {
            cfPutXCHG();
            if (curTx2aux2b[0] == 3)
            {
                curTx2aux2b[0] = 2;
                Sub_61A9(0);
            } else {
                curTx2aux2b[1] = 2;
                Sub_61A9(1);
            }
        }
        cfPutXTHL();
        reg = 3;
    }
    else
        cfPutPOP(reg);

    if (bC045[reg] == 0xb)
    {
        bC045[reg] = 0;
        if (reg != 0 && bC04E[reg] != 0)
        {
            cfCmdBuf[0] = reg;
            EncodeFragData(CF_MOVLRHR);
            curPC = curPC + 1;
        }
    }
    if (!boC1D8)
    {
        if (curTx2aux2b[1 - k] == 9)
        {
            if (curOpParams[1 - k] == curOpParams[k])
            {
                curTx2aux2b[1 - k] = reg;
                curTx2aux1b[1 - k] = bC045[reg];
                Sub_61A9(1 - k);
            } else
                boC1D8 = true;
        }
        curTx2aux2b[k] = reg;
        curTx2aux1b[k] = bC045[reg];
        Sub_61A9(k);
    }
}


static byte bC294, bC295, bC296, bC297, bC298;

static bool Sub_7FD9(byte arg1b)
{
    if (arg1b <= 3)
        if (curTx2aux2b[bC296] == arg1b)
            return true;
    return false;
}

static void Sub_7F19()
{
    for (bC295 = 0; bC295 <= 1; bC295++) {
        if (curOpParams[bC295] != 0 && bC0BB[bC295] != 0) {
            if (bC0BB[bC296 = 1 - bC295] != 0) {
                if (Sub_7FD9(b4C2D[bC0BD[bC295]] >> 5) || Sub_7FD9(b52B5[bC0BF[bC295]]))
                    bC0BB[bC296] = 10 + bC0BB[bC296];
            }
        }
    }

    if (bC0BB[0] > bC0BB[1])
        bC295 = 0;
    else
        bC295 = 1;

    bC296 = 1 - bC295;
    bC298 = bC0BD[bC295];
}



static void Sub_7FFC()
{
    if (bC298 == 0x13) {
        if (boC072[3] || boC069[3])
            bC298 = 0x15;
    }
    else if (bC298 == 0x14) {
        bC294 = 5 - bC297;
        if (curTx2aux2b[0] == curTx2aux2b[1])
            if (curTx2aux1b[bC295] == 0)
                bC298 = 0xE;
            else
                bC298 = 0x11;
    }
    else if (bC298 == 8) {
        if (tx2op1[curOpParams[bC295]] != 0) {
            bC298 = 6;
            if (bC294 == 0)
                bC294 = 1;
        }
    }
}



static void Sub_8086()
{
    if (9 <= bC298 && bC298 <= 13)
        cf_63AC(bC297);

    if (b4C2D[bC298] & 1) {
        if (bC294 != bC297)
            Sub_5D6B(bC294);
        else if (9 <= bC298 && bC298 <= 13) {
            curTx2aux2b[bC295] = 9;
            Sub_597E();
            Sub_5D6B(bC294);
            curTx2aux2b[bC295] = bC297;
            Sub_597E();
        }
    }
    else if (bC298 == 0x15) {
        if (boC069[3])
            curTx2aux2b[bC296] = 9;
    }
    else if (bC298 == 0x14) {
        if (curTx2aux2b[bC296] == bC294)
            curTx2aux2b[bC296] = bC297;
    }
}

static void Sub_8148(byte arg1b, byte arg2b)
{
    byte i;
    if (arg2b == 0)
        return;
    if (arg2b == 1 || arg2b == 2) {
        if (arg1b == 0)
            cfCmdBuf[cfCmdBufPos] = bC297;
        else
            cfCmdBuf[cfCmdBufPos] = bC294;
        cfCmdBufPos = cfCmdBufPos + 1;
    }
    else if (arg1b == 2) {
        cfCmdBuf[cfCmdBufPos] = 8;
        cfCmdBufPos = cfCmdBufPos + 2;
    }
    else if (arg1b == 3) {
        cfCmdBuf[cfCmdBufPos] = 0xA;
        cfCmdBuf[cfCmdBufPos + 1] = parInStack;
        cfCmdBufPos = cfCmdBufPos + 3;
    }
    else {
        i = cfCmdBufPos;
        Sub_636A(bC295);
    }
}

static void Sub_8207()
{
    switch (b4C15[bC298] >> 4) {
    case 0:
        Sub_5C97(bC294);
        parInStack = parInStack - 1;
        break;
    case 1:
        Sub_5C97(4);
        Sub_5C1D(3);
        Sub_5B96(4, 3);
        break;
    case 2:
        boC057[bC294] = true;
        bC0A8[bC294] = 0;
        bC04E[bC294] = curOpParams[bC295];
        wC096[bC294] = 0x100;
        if (cfCmdBuf[0] == 0xA) {
            wC084[bC294] = -(cfCmdBuf[1] * 2);
            if (bC0C3[tx2op3[curOpParams[bC295]]] == 0xb0)
                if (bC298 == 5) {
                    wC084[bC294] = wC084[bC294] - 1;
                    cfCmdBuf[2] = cfCmdBuf[2] + 1;
                }
        }
        else
            wC084[bC294] = cfCmdBuf[3] - parInStack * 2;
        break;
    case 3:
        boC057[bC294] = true;
        bC0A8[bC294] = 0;
        bC04E[bC294] = curOpParams[bC295];
        Sub_5FBF(bC04E[bC294], &wC084[bC294], &wC096[bC294]);
        break;
    case 4:
        boC057[bC294] = 0;
        bC04E[bC294] = curOpParams[bC295];
        if (curTx2aux1b[bC295] == 4 || curTx2aux1b[bC295] == 5) {
            bC0A8[bC294] = bC0C3[tx2op3[curOpParams[bC295]]] & 0xf;
            if (bC0A8[bC294] > 7)
                bC0A8[bC294] = bC0A8[bC294] | 0xf0;
        }
        else
            bC0A8[bC294] = 0;
        break;
    case 5:
        Sub_5B96(bC297, bC294);
        break;
    case 6:
        Sub_5B96(3, 4);
        Sub_5B96(2, 3);
        Sub_5B96(4, 2);
        break;
    case 7: break;
    }
}

static void Sub_841E()
{
    switch (b4C15[bC298] & 0xf) {
    case 0:  break;
    case 1: bC045[bC294] = 1; break;
    case 2: bC045[bC294] = 0; break;
    case 3: bC045[bC294] = 6; break;
    case 4:
        if (curTx2aux2b[bC295] != 8)
            bC045[bC294] = Sub_5748(curTx2aux1b[bC295]);
        else if (curTx2aux1b[bC295] == 0)
            bC045[bC294] = 6;
        else
            bC045[bC294] = curTx2aux1b[bC295];
        break;
    case 5:
        bC045[bC294] = curTx2aux1b[bC295] - 2;
        bC0A8[3] = bC0A8[3] + 1;
        break;
    }
}



void cf_7DA9()
{
    byte i;

    Sub_7F19();
    if (bC298 == 0x17)
        Sub_58F5(ERR214);
    else if (bC298 == 0x16) {
        bC0C1[bC295] = bC0BF[bC295];
        curTx2aux1b[bC295] = b528D[bC0C1[bC295]];
        curTx2aux2b[bC295] = b52B5[bC0C1[bC295]];
    }
    else if (bC298 == 0x12)
        boC1D8 = true;
    else {
        bC294 = b4C2D[bC298] >> 5;
        if (bC294 > 3)
            bC294 = b52B5[bC0BF[bC295]];
        bC297 = curTx2aux2b[bC295];
        Sub_597E();
        Sub_7FFC();
        i = b5012[bC298];
        Sub_8086();
        cfCmdBufPos = 0;
        Sub_8148((b4C2D[bC298] >> 3) & 3, (b4029[i] >> 4) & 7);
        Sub_8148((b4C2D[bC298] >> 1) & 3, (b4029[i] >> 1) & 7);
        Sub_8207();
        Sub_841E();
        curTx2aux1b[bC295] = bC045[bC294];
        curTx2aux2b[bC295] = bC294;
        Sub_61A9(0);
        Sub_61A9(1);
        EncodeFragData(i);
        curPC = curPC + (cfTypeAndLen[i] & 0x1f);

    }
}



static void Sub_8698(byte arg1b, byte arg2b)
{
    byte nParam;
    word p;
    switch (arg1b)
    {
        case 0: return;
        case 1: nParam = 0; break;
        case 2: nParam = 1; break;
        case 3:
            /* завершение CF_PUSH (для временного сохранения результата) */
            cfCmdBuf[cfCmdBufPos] = 0xA;
            cfCmdBuf[cfCmdBufPos + 1] = parInStack;
            cfCmdBufPos = cfCmdBufPos + 3;
            return;
        case 4:
            /* CF_DAD, CF_INDEXW */
            if (curTx2aux2b[0] == 3)
                nParam = 1;
            else
                nParam = 0;
            break;
        case 5:
            if (curTx2aux2b[0] == 0)
                nParam = 1;
            else
                nParam = 0;
            break;
        case 6:
            if (arg2b == 7)
            {
                /* CF_CALL */
                cfCmdBuf[cfCmdBufPos] = 0x10;
                cfCmdBuf[cfCmdBufPos + 1] = tx2op3[tx2qp] - botInfo;
                cfCmdBufPos = cfCmdBufPos + 2;
            } else
                /* CF_CALLVAR */
                Sub_61E0((byte)tx2op3[tx2qp]); /* tx2op3[] - номер опкода */
            return;
        default:
            fprintf(stderr, "out of bounds in Sub_8698 arg1b = %d\n", arg1b);
            Exit();
    }
    if (arg2b <= 3)
        Sub_636A(nParam);
    else {
        cfCmdBuf[cfCmdBufPos] = arg2b + 9;
        if (arg2b == 6)
            cfCmdBuf[cfCmdBufPos + 1] = tx2op2[1];
        else
            Sub_5FBF(curOpParams[nParam], &cfCmdBuf[cfCmdBufPos + 1], &p);
        cfCmdBufPos = cfCmdBufPos + 2;
    }
}

void cf_84ED()
{
    byte i;

    if (cfrag1 > CF_3) {
        cfCmdBufPos = 0;
        Sub_8698(b42F9[cfrag1] >> 4, (b4029[cfrag1] >> 4) & 7);
        if (cfrag1 == CF_67 || cfrag1 == CF_68)
            cfCmdBuf[cfCmdBufPos - 1] = cfCmdBuf[cfCmdBufPos - 1] + 2;
        Sub_8698(b42F9[cfrag1] & 0xf, (b4029[cfrag1] >> 1) & 7);
        EncodeFragData(cfrag1);
        curPC = curPC + (cfTypeAndLen[cfrag1] & 0x1f);
        if (cfrag1 == CF_DELAY) {
            WordP(helpersP)[105] = 1;
            if (curSP < (parInStack + 1) * 2)
                curSP = (parInStack + 1) * 2;
        }
        else if (cfrag1 > CF_171) {
            i = b413B[cfrag1 - CF_INTERNALFUNCS];
            i = b4128[i] + 11 * b425D[b4273[curOp]];
            i = b3FCD[b418C[i] >> 2] + (b418C[i] & 3);
            WordP(helpersP)[i] = 1;
            if (curOp == T2_SLASH || curOp == T2_MOD || curOp == T2_44) {
                if (curSP < (parInStack + 2) * 2)
                    curSP = (parInStack + 2) * 2;
            } else if (curSP < (parInStack + 1) * 2)
                curSP = (parInStack + 1) * 2;
        }
    }
}


