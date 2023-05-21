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

static byte b9BA8[2] = {12, 13};
static byte b9BAA[2] = {1, 2};
static byte b9BAC[2] = {12, 13};
static byte b9BAE[2] = {1, 2};



static bool Sub_9C33()
{
    byte i, j, k;

    i = (byte)tx2op1[tx2qp];
    if ((flagsT2[tx2opc[i]] & 0xc0) == 0) {
        if (tx2Auxw[i] > 1)
            return false;
        if (tx2op3[i] != 0)
            bC140[tx2op3[i]] = tx2qp;
    }
    k = (byte)tx2Auxw[tx2qp];
    copyTx2Code(i, tx2qp);
    tx2Auxw[tx2qp] = k;
    tx2Auxw[i] = tx2Auxw[i] - 1;
    for (j = 0; j <= 3; j++) {
        if (bC04E[j] == i)
            bC04E[j] = tx2qp;
    }
    return true;
}

void Sub_9BB0()
{
    curOpParams[0] = (byte)tx2op1[tx2qp];
    curOpParams[1] = (byte)tx2op2[tx2qp];
    if (T2_DOUBLE <= curOp && curOp <= T2_ADDRESSOF)
        Sub_717B();
    if (curOp <= T2_MEMBER) {
        Sub_7550();
        if (curOp == T2_65)
            if (Sub_9C33())
                return;
    }
    if ((curOpFlags & 0xc0) == 0) {
        Sub_87CB();
        if (curOp == T2_MOVE)
            procCallDepth = 0;
    } else if ((curOpFlags & 0xc0) == 0x80)
        cf_994D();
}


/*
  обрабатываем T2_CALL
*/
void m2p3_doCALL()
{
    byte i, j, k;
    pointer pbyt;
    byte m;

    if (procCallDepth <= 10) {
        curInfoP = tx2op3[tx2qp];
        i = GetDataType();
        if (i == 3)
            wAF54[T2_CALL] = 1;
        else
            wAF54[T2_CALL] = 0;
        j = m = GetParamCnt();
        pbyt = &b44F7[wAF54[T2_CALL]];
        k = 0;

        while (j > 0) {
            AdvNxtInfo();
            j = j - 1;
            if (j < 2) {
                *pbyt = (*pbyt << 4) & 0xf0;
                if (GetType() == ADDRESS_T)
                    *pbyt = *pbyt | b9BA8[k];
                else
                    *pbyt = *pbyt | b9BAA[k];
                k = 1;
            }
        }

        if (m == 1)
            *pbyt = (*pbyt << 4) & 0xf0;
        Sub_9BB0();
        parInStack = wB528[procCallDepth];
    }
    procCallDepth = procCallDepth - 1;
}


static pointer pb_C2EB;


static void Sub_9EAA(byte arg1b, byte arg2b)
{

    *pb_C2EB = (*pb_C2EB << 4) & 0xf0;
    if (arg1b != 0)
        if (tx2Aux1b[arg1b] == 0)
            *pb_C2EB = *pb_C2EB | b9BAE[arg2b];
        else
            *pb_C2EB = *pb_C2EB | b9BAC[arg2b];
}

void m2p3_doCALLVAR()
{
    byte i;

    if (procCallDepth <= 10) {
        i = (byte)tx2op3[tx2qp];
        if (tx2opc[i] == T2_IDENTIFIER) {
            curInfoP = tx2op1[i];
            if (TestInfoFlag(F_AUTOMATIC))
                wAF54[T2_CALLVAR] = 3;
            else
                wAF54[T2_CALLVAR] = 4;
        } else if (tx2op3[i] == wB53C[procCallDepth]) {
            wAF54[T2_CALLVAR] = 5;
            wB528[procCallDepth] = wB528[procCallDepth] - 1;
        } else
            wAF54[T2_CALLVAR] = 2;

        pb_C2EB = &b44F7[wAF54[T2_CALLVAR]];
        Sub_9EAA((byte)tx2op1[tx2qp], 0);
        Sub_9EAA((byte)tx2op2[tx2qp], 1);
        Sub_9BB0();
        parInStack = wB528[procCallDepth];
    }
    procCallDepth = procCallDepth - 1;
}




void m2_doBEGMOVE()
{
    procCallDepth = 1;
    Sub_9BB0();
    wB53C[procCallDepth] = parInStack;
}



void m2_doCASE()
{
    if (EnterBlk())
        blkCurInfo[blkSP] = wC1CF;
}


void m2_doENDCASE()
{
    word p, q;
    p = q = (word)blkCurInfo[blkSP];
    if (ExitBlk()) {
        while (p < wC1CF) {
            cfCmdBuf[0] = 14;
            cfCmdBuf[1] = WordP(botMem)[p];
            EncodeFragData(CF_DW);
            curPC = curPC + 2;
            p = p + 1;
        }
        if (wC1CF == q) {
            Tx2SyntaxError(ERR201); /*  Invalid() do CASE block, */
                        /*  at least on case required */
            EmitTopItem();
        }
        wC1CF = q;
    }
}



void m2_doENDPROC()
{
    if (ExitBlk()) {
        curInfoP = blkCurInfo[procChainId];
        if (! boC1CC)
        {
            procRestoreRegs();
            EncodeFragData(CF_RET);
            curPC = curPC + 1;
        }
        if (TestInfoFlag(F_INTERRUPT))
            curSP = curSP + 8;

        SetDimension(curPC);
        SetBaseVal(curSP + wC1C7);
        curPC = stackPC[procChainId = procChainNext[procChainId]];
        lenBufTx1 = 0;
        PutTx1Byte(0xa4);
        PutTx1Word(blkCurInfo[procChainId] - botInfo);
        PutTx1Word(curPC);
        WrFragData();
        parInStack = wB4B0[procChainId];
        curSP = stackSP[procChainId];
        wC1C7 = 0;
        curExtProcId = extProcId[procChainId];
    }
}

void m2_doLENGTH(byte arg1b)
{
    word p;
    curInfoP = tx2op1[tx2qp] + botInfo;
    p = GetDimension() - arg1b;
    if (p < 0x100)
        m2_setNumOrIdn(p, 0, 0, 8);
    else
        m2_setNumOrIdn(p, 0, 1, 8);
}


void m2_doSIZE()
{
    word p;
    p = getIdnSize(tx2op1[tx2qp] + botInfo);
    if (p < 0x100)
        m2_setNumOrIdn(p, 0, 0, 8);
    else
        m2_setNumOrIdn(p, 0, 1, 8);
}

void m2_doBEGCALL()
{
    procCallDepth = procCallDepth + 1;
    if (procCallDepth <= 10)
    {
        Sub_5E66(0xf);
        wB528[procCallDepth] = parInStack;
        wB53C[procCallDepth] = parInStack;

    } else if (procCallDepth == 11) {
        Tx2SyntaxError(ERR203); /*  LIMIT EXCEEDED: NESTING OF TYPED */
                    /*  procedure CALLS */
        EmitTopItem();
    }
}




static void Sub_A266()
{
    byte i;

    boC1CD = false;
    for (i = 0; i <= 3; i++) {
        bC045[i] = 0xc;
        bC04E[i] = 0;
        boC057[i] = false;
    }
}

void m2_Parse3()
{
    Sub_A266();
    for (tx2qp = 4; tx2qp <= tx2lastPos - 1; tx2qp++) {
        curOp = tx2opc[tx2qp];
        curOpFlags = flagsT2[curOp];
        switch (curOpFlags >> 6)
        {
            case 0:
                if (curOp == T2_CALL)
                    m2p3_doCALL();
                else if (curOp == T2_CALLVAR)
                    m2p3_doCALLVAR();
                else if (curOp == T2_BEGMOVE)
                    m2_doBEGMOVE();
                else
                    Sub_9BB0();
                break;
            case 1:
                if (curOp == T2_LENGTH)
                    m2_doLENGTH(0);
                else if (curOp == T2_LAST)
                    m2_doLENGTH(1);
                else if (curOp == T2_SIZE)
                    m2_doSIZE();
                break;
            case 2:
                if (curOp == T2_PROCEDURE)
                    m2p3_doPROCEDURE();
                else
                    cf_994D();
                break;
            case 3:
                if (curOp == T2_CASE)
                    m2_doCASE();
                else if (curOp == T2_ENDCASE)
                    m2_doENDCASE();
                else if (curOp == T2_ENDPROC)
                    m2_doENDPROC();
                else if (curOp == T2_BEGCALL)
                    m2_doBEGCALL();
                break;
        }

        tx2op3[tx2qp] = 0;
    }
    cf_5795(0);
    boC1CC = boC1CD;
}


