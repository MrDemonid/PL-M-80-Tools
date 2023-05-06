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

static byte curIdxPar, curTx2Par;
static word wC2A9, wC2AB, wC2AD;


static bool Sub_8861()
{
    byte i;
    word idx;

    for (idx = wC2AB; idx <= wC2AD; idx++)
    {
        i = Sub_5679(curIdxPar, idx);
        if (i <= 3)
            return true;
        if (12 <= i && i <= 14)
            return true;
    }
    return false;
} /* Sub_8861() */



static void Sub_88C1()
{
    byte idx;

    if (Sub_8861()) {
        for (idx = 0; idx <= 3; idx++) {
            if (bC04E[idx] == curTx2Par) {
                if (bC045[idx] == 0 || bC045[idx] == 1 || bC045[idx] == 6) {
                    curTx2aux1b[curIdxPar] = bC045[idx];
                    curTx2aux2b[curIdxPar] = idx;
                    if (curTx2aux2b[1 - curIdxPar] != idx)
                        return;
                }
            }
        }
    }
} /* Sub_88C1() */

static void Sub_894A()
{
    byte idx;

    if (curTx2aux2b[curIdxPar] > 3)
    {
        for (idx = 1; idx <= 3; idx++) {
            if (bC04E[idx] == curTx2Par) {
                if (bC045[idx] == 2 || bC045[idx] == 3) {
                    curTx2aux1b[curIdxPar] = bC045[idx];
                    curTx2aux2b[curIdxPar] = idx;
                    if (curTx2aux2b[1 - curIdxPar] != idx)
                        return;
                }
            }
        }
    }
} /* Sub_894A() */


static void Sub_89D1()
{
    byte i;
    word p;

    if (curTx2aux2b[curIdxPar] == 0xA)
        wC2A9 = tx2op2[curTx2Par];
    else if (curTx2aux2b[curIdxPar] == 9) {
        wC2A9 = tx2op3[curTx2Par];
        if (( ! boC069[0] && boC072[0]) || bC0B1 > 0 || wC2A9 != parInStack) {
            i = bC0B1 + bC0B2;
            for (p = wC2A9; p <= parInStack; p++) {
                if (bC140[p] != 0)
                    i = i + 1;
            }
            if (i < 4)
                boC1D8 = true;
            else
                curTx2aux2b[curIdxPar] = 0xA;
        }
        wC2A9 = - (wC2A9 * 2);
    }
} /* Sub_89D1() */

static void Sub_8A9C()
{
    word p, q;
    byte i, j;
    word r;
    byte idx;

    if (curTx2aux2b[curIdxPar] == 0xA) {
        p = wC2A9;
        q = 0x100;
        i = 4;
        j = Sub_5748(curTx2aux1b[curIdxPar]);
    } else if (curTx2aux2b[curIdxPar] == 8 && curTx2aux1b[curIdxPar] == 1) {
        Sub_5FBF(curOpParams[curIdxPar], &p, &q);
        i = 2;
        j = 1;
    } else if (curTx2aux2b[curIdxPar] == 4 && (curTx2aux1b[curIdxPar] == 0 || curTx2aux1b[curIdxPar] == 8 || !Sub_8861())) {
        Sub_5FBF(curOpParams[curIdxPar], &p, &q);
        i = 2;
        j = Sub_5748(curTx2aux1b[curIdxPar]);
    } else
        return;

    for (idx = 1; idx <= 3; idx++) {
        if (boC069[idx]) {
            if (curOpParams[0] == curOpParams[1] && curOp != T2_COLONEQUALS)
                if (curTx2aux2b[curIdxPar] > 3)
                    curTx2aux2b[curIdxPar] = idx;
        }
        else if (! boC072[idx] && wC096[idx] == q && boC057[idx] && 1 <= bC045[idx] && bC045[idx] <= 6) {
            r = wC084[idx] + bC0A8[idx] - p;
            if (r > 0xff)
                r = -r;
            if (r < i) {
                curTx2aux2b[curIdxPar] = idx;
                i = (byte)r;
            }
        }
    }
    if (curTx2aux2b[curIdxPar] <= 3) {
        idx = curTx2aux2b[curIdxPar];
        bC045[idx] = curTx2aux1b[curIdxPar] = j;
        bC04E[idx] = curOpParams[curIdxPar];
        bC0A8[idx] = wC084[idx] + bC0A8[idx] - p;
        wC084[idx] = p;
    }
} /* Sub_8A9C() */



static void setAuxParams()
{
    byte curTx2op;

    curTx2aux2b[0] = 8;
    curTx2aux2b[1] = 8;
    for (curIdxPar = 0; curIdxPar <= 1; curIdxPar++)
    {
        if ((curTx2Par = curOpParams[curIdxPar]) == 0)
            curTx2aux1b[curIdxPar] = 0xC;

        else if ((curTx2op = tx2opc[curTx2Par]) == T2_STACKPTR)
            curTx2aux1b[curIdxPar] = 0xA;

        else if (curTx2op == T2_LOCALLABEL)
            curTx2aux1b[curIdxPar] = 9;

        else {
            curTx2aux1b[curIdxPar] = tx2Aux1b[curTx2Par];
            curTx2aux2b[curIdxPar] = tx2Aux2b[curTx2Par];
            Sub_88C1();
            Sub_894A();/*  checked */
        }
    }
    for (curIdxPar = 0; curIdxPar <= 1; curIdxPar++)
    {
        curTx2Par = curOpParams[curIdxPar];
        Sub_597E();
        Sub_89D1();
        Sub_8A9C();
        Sub_61A9(curIdxPar);
    }
} /* setAuxParams() */




static byte Sub_8E7E(byte arg1b, word idx)
{
    word p;
    byte i;

    if (curOpParams[arg1b] == 0 || curOpParams[arg1b] == 1)
        return 1;
    i = Sub_5679(arg1b, idx);
    return b4D23[p = bC0C1[arg1b] * 16 + i];
} /* Sub_8E7E() */

static void Sub_8ECD(byte arg1b, byte arg2b)
{
    bC0B9[arg1b] = b4C45[arg2b];
    bC0BB[arg1b] = b4CB4[arg2b];
    bC0BD[arg1b] = b4FA3[arg2b];
} /* Sub_8ECD() */

static void Sub_8DCD()
{
    byte h, i, j, k, m, n;
    word idx;

    j = 198;
    for (idx = wC2AB; idx <= wC2AD; idx++)
    {
        k = Sub_8E7E(0, idx);
        m = Sub_8E7E(1, idx);
        n = b4C45[k] + b4C45[m] + (cfTypeAndLen[b4A21[idx]] & 0x1f);
        if (n < j)
        {
            j = n;
            h = k;
            i = m;
            cfrag1 = b4A21[idx];
            bC1D9 = b46EB[idx];
            bC0BF[0] = Sub_5679(0, idx);
            bC0BF[1] = Sub_5679(1, idx);
        }
    }
    Sub_8ECD(0, h);
    Sub_8ECD(1, i);
} /* Sub_8DCD() */



static void Sub_8F16()
{
    if (curOpParams[0] != 0)
        cf_63AC(curTx2aux2b[0]);

    if (curOpParams[1] != 0)
        cf_63AC(curTx2aux2b[1]);

} /* Sub_8F16() */


static void Sub_8F35()
{
    word p;

    if (curOp == T2_STKARG || curOp == T2_STKBARG || curOp == T2_STKWARG)
    {
        cf_5795(-(wB53C[procCallDepth] * 2));
        wB53C[procCallDepth] = wB53C[procCallDepth] + 1;
        parInStack = parInStack + 1;
    } else if (curOp == T2_CALL)
    {
        cf_5795(-(wB53C[procCallDepth] * 2));
        curInfoP = tx2op3[tx2qp];
        if (TestInfoFlag(F_EXTERNAL))
            p = (wB53C[procCallDepth] + 1) * 2;
        else
            p = (wB528[procCallDepth] + 1) * 2 + GetBaseVal();
        if (p > curSP)
            curSP = p;
    } else if (curOp == T2_CALLVAR)
    {
        cf_5795(-(wB53C[procCallDepth] * 2));
        if (curSP < parInStack * 2)
            curSP = parInStack * 2;
    } else if (curOp == T2_RETURN || curOp == T2_RETURNBYTE || curOp == T2_RETURNWORD)
    {
        boC1CD = true;
        procRestoreRegs();
    } else if (curOp == T2_JMPFALSE)
    {
        cf_5795(0);
        if (boC20F) {
            cfrag1 = CF_JMPTRUE;
            boC20F = false;
        }
    } else if (curOp == T2_63)
    {
        cf_5795(0);
    } else if (curOp == T2_MOVE)
    {
        if (wB53C[procCallDepth] != parInStack)
        {
            cf_5795(-((wB53C[procCallDepth] + 1) * 2));
            cfPutPOP(REG_HL);
        }
        if (bC045[3] == 1)
            cfrag1 = CF_MOVE_HL;
    }
} /* Sub_8F35() */


static void Sub_940D()
{
    byte idx;

    for (idx = 0; idx <= 3; idx++) {
        if (bC04E[idx] == curOpParams[0])
            if (bC045[idx] < 2 || 5 < bC045[idx])
                bC04E[idx] = 0;
    }
}

static void Sub_90EB()
{
    word p, q;
    byte i, j, k;
    byte idx;

    p = w48DF[bC1D9] * 16;
    q = w493D[bC1D9];
    k = 0;
    if (curOp == T2_COLONEQUALS) {
        Sub_940D();
        if (tx2Auxw[curOpParams[1]] == 0)
            if (tx2Auxw[curOpParams[0]] > 0) {
                if (cfrag1 == CF_MOVMLR || cfrag1 == CF_STA) {
                    bC045[curTx2aux2b[1]] = 0;
                    bC04E[curTx2aux2b[1]] = curOpParams[0];
                } else if (cfrag1 == CF_SHLD || cfrag1 == CF_MOVMRP) {
                    bC045[curTx2aux2b[1]] = 1;
                    bC04E[curTx2aux2b[1]] = curOpParams[0];
                }
            }
    }
    else if (T2_51 <= curOp && curOp <= T2_56)
        Sub_940D();
    for (idx = 5; idx <= 8; idx++) {
        i = p >> 13;
        j = q >> 12;
        p <<= 3;
        q <<= 4;
        if (j <= 3) {
            Sub_5B96(j, idx);
            if (i == 1)
                bC0A8[idx] = bC0A8[idx] + 1;
            else if (i == 2) {
                if (bC045[idx] == 0) {
                    bC045[idx] = 6;
                } else {
                    bC045[idx] = 1;
                    boC057[idx] = 0;
                }
            }
        }
        else if (j == 4) {
            boC057[k = idx] = 0;
            if (0 < tx2Auxw[tx2qp]) {
                bC04E[idx] = tx2qp;
                bC045[idx] = tx2Aux1b[tx2qp] = cfTypeAndLen[cfrag1] >> 5;
                bC0A8[idx] = 0;
            } else
                bC04E[idx] = 0;
        } else if (j == 5) {
            bC04E[idx] = 0;
            wC096[idx] = 0;
            bC0A8[idx] = 0;
            boC057[idx] = 0xFF;
            bC045[idx] = 0;
            wC084[idx] = i;
        } else {
            bC04E[idx] = 0;
            boC057[idx] = 0;
        }
    }
    if (k == 0 && tx2Auxw[tx2qp] > 0) {
        for (idx = 5; idx <= 8; idx++) {
            if (bC04E[idx] == 0)
                if (! boC057[k = idx])
                    break;
        }
        if (k != 0) {
            bC04E[k] = tx2qp;
            boC057[k] = 0;
            bC045[k] = 0;
            tx2Aux1b[tx2qp] = 0;
            bC0A8[k] = 0;
        }
    }
    for (idx = 0; idx <= 3; idx++)
        Sub_5B96(idx + 5, idx);
} /* Sub_90EB() */

void Sub_87CB()
{
    curOpParams[0] = (byte)tx2op1[tx2qp];
    curOpParams[1] = (byte)tx2op2[tx2qp];
    wC2AB = wAF54[curOp];
    wC2AD = wC2AB + b499B[curOp] - 1;
    setAuxParams();                     // устанавливаем curTx2aux1/2[]

    while (1) {
        Sub_8DCD();     /*  OK */
        if (bC0B9[0] == 0)
            if (bC0B9[1] == 0)
                break;
        if (boC1D8)
            cf_7A85();
        else
            cf_7DA9();
    }
    Sub_8F16();
    Sub_611A();
    Sub_5E66(w48DF[bC1D9] >> 12);
    Sub_8F35();
    cf_84ED();
    Sub_90EB();
} /* Sub_87CB() */


void m2_doPROCEDURE()
{
    if (EnterBlk())
    {
        stackPC[procChainId] = curPC;
        wB4B0[procChainId] = parInStack;
        stackSP[procChainId] = curSP;
        extProcId[procChainId] = curExtProcId;
        procChainNext[blkSP] = procChainId;
        procChainId = blkSP;
        curInfoP = blkCurInfo[blkSP] = tx2op1[tx2qp] + botInfo;
        curExtProcId = GetProcId();
        curPC = 0;
        EmitTopItem();       // сохраняем в tx1 код формата T2_PROCEDURE...
        makeEntryProc();     // создаем код входа в процедуру (сохраняем параметры)
    }
}
