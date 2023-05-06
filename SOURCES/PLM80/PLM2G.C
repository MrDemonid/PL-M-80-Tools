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

static byte curParamCnt;
static byte curReg;
static byte procParamIdx, bC2D3;
static word wC2D4;


void FindParamInfo(byte arg1b)
{
    curInfoP = blkCurInfo[blkSP];
    while (arg1b != 0) {
        AdvNxtInfo();
        arg1b = arg1b - 1;
    }
}


/*
  формирует код сохранения одного параметра функции во временную переменную
*/
void SaveOneProcPar()
{
    if (GetType() == ADDRESS_T)
    {
        cfCmdBuf[0] = curReg;
        EncodeFragData(CF_MOVMRPR); // MOV M, <hReg8> / DCX H / MOV M, <lReg8>
        curPC = curPC + 3;

    } else {
        cfCmdBuf[0] = curReg;
        EncodeFragData(CF_MOVMLR);  // MOV M, <lReg8>
        curPC = curPC + 1;
    }

    if (procParamIdx != curParamCnt)
    {
        EncodeFragData(CF_DCXHL);   // DCX H
        curPC = curPC + 1;
    }
}

/*
  формирует код сохранения всех параметров функции во временные переменную,
  для быстрого доступа к ним
*/
void SaveProcParams()
{
    byte i;

    if ((i = curParamCnt) == 1)
        curReg = REG_BC;
    else
        curReg = REG_DE;
    for (procParamIdx = 1; procParamIdx <= curParamCnt; procParamIdx++)
    {
        FindParamInfo(i);
        if (procParamIdx == 2)
        {
            curReg = REG_BC;

        } else if (procParamIdx == 3) {
            cfCmdBuf[0] = REG_DE;
            cfCmdBuf[1] = 8;
            EncodeFragData(CF_POP);     // POP D
            cfCmdBuf[0] = REG_BC;
            cfCmdBuf[1] = 8;
            EncodeFragData(CF_POP);     // POP B
            curPC = curPC + 2;

        } else if (procParamIdx > 3) {
            cfCmdBuf[0] = REG_BC;
            cfCmdBuf[1] = 8;
            EncodeFragData(CF_POP);     // POP B
            curPC = curPC + 1;
        }

        SaveOneProcPar();
        i = i - 1;
    }
    if (curParamCnt > 2)
    {
        cfCmdBuf[0] = REG_DE;
        cfCmdBuf[1] = 8;
        EncodeFragData(CF_PUSH);        // PUSH D
        curPC = curPC + 1;
    }
}

void cf_9624(word arg1w)
{
    cfCmdBuf[0] = 9;
    cfCmdBuf[1] = arg1w;
    EncodeFragData(CF_STACKPTR);
    curPC = curPC + 4;
}


void cf_9646(word arg1w)
{
    if ((arg1w >> 1) + (arg1w & 1) <= 5)
    {
        if (arg1w & 1)
        {
            EncodeFragData(CF_DCXSP);   // DCX SP
            curPC = curPC + 1;
        }
        while (arg1w > 1)
        {
            cfCmdBuf[0] = REG_HL;
            cfCmdBuf[1] = 8;
            EncodeFragData(CF_PUSH);    // PUSH H
            curPC = curPC + 1;
            arg1w = arg1w - 2;
        }
    }
    else {
        cf_9624(-arg1w);
        EncodeFragData(CF_SPHL);        // SPHL
        curPC = curPC + 1;
    }
}


void cfPutINXH()
{
    cfCmdBuf[0] = REG_HL;
    EncodeFragData(CF_INX);
    curPC = curPC + 1;
}


void cfPutOpBC(byte arg1b)
{
    cfCmdBuf[0] = REG_BC;
    EncodeFragData(arg1b);
    curPC = curPC + 1;
}

void cfPutOpDE(byte arg1b)
{
    cfCmdBuf[0] = REG_DE;
    EncodeFragData(arg1b);
    curPC = curPC + 1;
}


void cf_9706()
{
    cfPutINXH();                        // INX H        // INX H
    if (GetType() == ADDRESS_T)
    {
        cfPutOpBC(CF_MOVLRM);           // MOV C, M     // MOV C, M
        if (bC2D3 == 1)
            cfPutOpDE(CF_MOVMLR);       // MOV M, E

        cfPutINXH();                    // INX H        // INX H
        cfPutOpBC(CF_MOVHRM);           // MOV B, M     // MOV B, M

    } else {
        cfPutOpBC(CF_MOVHRM);           // MOV B, M     // MOV B, M
        if (bC2D3 == 1)
            cfPutOpDE(CF_MOVMLR);       // MOV M, E
        cfPutINXH();                    // INX H        // INX H
    }
    if (bC2D3 == 1)
        cfPutOpDE(CF_MOVMHR);           // MOV M, D
}



void cfPutMOVRPM()
{
    cfPutOpDE(CF_MOVRPM);
    curPC = curPC + 2;
}


void cf_975F()
{
    cfCmdBuf[0] = curReg;
    cfCmdBuf[1] = 8;
    EncodeFragData(CF_PUSH);            // PUSH <curReg>
    curPC = curPC + 1;
    if (GetType() == BYTE_T)
    {
        EncodeFragData(CF_INXSP);       // INX SP
        curPC = curPC + 1;
    }
}


void cf_978E()
{
    if ((bC2D3 = curParamCnt) > 2)
        cf_9624(wC2D4);
    if (curParamCnt == 1)
        curReg = REG_BC;
    else
        curReg = REG_DE;
    for (procParamIdx = 1; procParamIdx <= curParamCnt; procParamIdx++)
    {
        FindParamInfo(bC2D3);

        if (procParamIdx > 3)
            cf_9706();

        else if (procParamIdx == 3)
        {
            cfPutMOVRPM();
            cf_9706();

        } else if (GetType() == BYTE_T)
        {
            cfCmdBuf[0] = curReg;
            EncodeFragData(CF_MOVHRLR);
            curPC = curPC + 1;
        }
        cf_975F();
        curReg = REG_BC;
        bC2D3 = bC2D3 - 1;
    }
}


void makeEntryProc()
{
    byte i, j;
    curParamCnt = GetParamCnt();
    if (TestInfoFlag(F_INTERRUPT))
    {
        for (j = REG_PSW; j <= REG_HL; j++)
        {   /*
              сохраняем в стек регистры hl, de, bc, psw
            */
            cfCmdBuf[0] = REG_HL - j;    // 3 - H, 2 - D, 1 - B, 0 PSW
            cfCmdBuf[1] = 8;
            EncodeFragData(CF_PUSH);     // команда PUSH
            curPC = curPC + 1;
        }
    }

    if (TestInfoFlag(F_REENTRANT))
    {
        wC1C7 = GetParentVal(); /* or Size() */;
        if (curParamCnt > 0)
        {
            FindParamInfo(curParamCnt);
            wC2D4 = wC1C7 - GetLinkVal() - 1;
            if (GetType() == ADDRESS_T)
                wC2D4 = wC2D4 - 1;
            cf_9646(wC2D4);
            cf_978E();
        }
        else
            cf_9646(wC1C7);

        if (curParamCnt > 2)
            wC1C7 += (curParamCnt - 2) * 2;

        curSP = 0;

    } else {
        if (curParamCnt > 0)
        {
            /*
              формируем команду LXI H, param
              для последующего сохранения переданных в процедуру параметров
            */
            FindParamInfo(curParamCnt); /*  locate info for first param */
            if (GetType() == ADDRESS_T)
                i = 1;                  // lxi h, param+1
            else
                i = 0;                  // lxi h, param

            cfCmdBuf[0] = REG_HL;
            cfCmdBuf[1] = 0xB;
            cfCmdBuf[2] = i;            // смещение к адресу операнда (param+i)
            cfCmdBuf[3] = curInfoP - botInfo;  /*  info for first param */
            EncodeFragData(CF_LXI);
            // генерируем команды входа в процедуру
            SaveProcParams();
            curPC = curPC + 3;
        }
        wC1C7 = 0;
        if (curParamCnt > 2)
            curSP = (curParamCnt - 2) * 2;
        else
            curSP = 0;
    }
}

void cf_994D()
{
    byte i, j;

    if (curOp == T2_LABELDEF)
    {
        boC1CC = false;
        curInfoP = tx2op1[tx2qp] + botInfo;
        SetLinkVal(curPC);

    } else if (curOp == T2_LOCALLABEL)
    {
        boC1CC = false;
        WordP(localLabelsP)[tx2op1[tx2qp]] = curPC;
        ByteP(w381E)[tx2op1[tx2qp]] = curExtProcId;

    } else if (curOp == T2_CASELABEL)
    {
        WordP(localLabelsP)[tx2op1[tx2qp]] = curPC;
        ByteP(w381E)[tx2op1[tx2qp]] = curExtProcId;
        if ((w3822 - botMem) / 2 >= wC1CF)
        {
            WordP(botMem)[wC1CF] = tx2op1[tx2qp];
            wC1CF = wC1CF + 1;
        } else {
            EmitTopItem();
            Tx2SyntaxError(ERR202); /*  LIMIT EXCEEDED: NUMBER OF */
                        /*  ACTIVE CASES */
        }
    } else if (curOp == T2_JMP || curOp == T2_JNC || curOp == T2_JNZ || curOp == T2_GOTO)
    {
        i = tx2opc[tx2qp - 1];
        if (i == T2_RETURN || i == T2_RETURNBYTE || i == T2_RETURNWORD || i == T2_GOTO)
            return;
        cf_5795(0);

    } else if (curOp == T2_INPUT || (T2_SIGN <= curOp && curOp <= T2_CARRY))
    {
        curOpParams[0] = 0;
        curOpParams[1] = 0;
        curTx2aux2b[0] = 8;
        curTx2aux2b[1] = 8;
        Sub_597E();
        Sub_5D6B(0);
        bC045[0] = 0;
        bC04E[0] = tx2qp;
        boC057[0] = 0;
        bC0A8[0] = 0;
        tx2Aux1b[tx2qp] = 0;
        tx2Aux2b[tx2qp] = 9;

    } else if (curOp == T2_STMTCNT)
    {
        j = tx2qp + 1;

        while (tx2opc[j] != T2_STMTCNT && tx2opc[j] != T2_EOF && j < 0xFF)
        {
            if ((flagsT2[tx2opc[j]] & 0x20) == 0 || tx2opc[j] == T2_MODULE)
                goto L9B8D;
            j = j + 1;
        }
        curOp = CF_134;
        tx2opc[tx2qp] = CF_134;
    }
L9B8D:
    EmitTopItem();
    curPC = curPC + (cfTypeAndLen[curOp] & 0x1f);
}
