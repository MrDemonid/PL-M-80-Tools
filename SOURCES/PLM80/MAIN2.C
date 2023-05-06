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
#include <setjmp.h>

word wAF54[] = {
    0x11B, 0x14B, 0x12B, 0x12B, 0x11B, 0x14B,  0x60,  0x60,   //   0
     0x62,  0x62,  0x5B,  0x62,   0xB, 0x1E4, 0x1E7, 0x1E8,   //   8
    0x1EE, 0x1F1,  0x8D,  0xCF, 0x10B,   0xE,  0x12,  0x14,   //  16
     0xEB,  0xEB,  0xEB,  0xA9,  0x9A,  0x96,  0xA1,  0x69,   //  24
     0x68,  0x70,  0x76,  0x90,  0xEB, 0x10B, 0x15B, 0x18B,   //  32
    0x1BB, 0x19B, 0x17B, 0x16B,  0x58,  0x19,  0x20,  0x27,   //  40
     0x19,  0x20,  0x27,  0x16,  0x17,  0x18,  0x16,  0x17,   //  48
     0x18,   0xA,  0x38,  0x3E,  0x67,  0x2A,     6,   0xC,   //  56
    0x1DE,  0x80,  0x43,  0x45,  0x47, 0x1DB,  0x49,  0xC9,   //  64
     0xCD,  0xCE,  0xB7,  0xBA,  0xBD,  0x41,  0x42,  0x44,   //  72
     0x46,  0x48,  0x4A,  0x4B,  0x4C,  0x41,  0x42,  0x44,   //  80
     0x46,  0x48,  0x4A,  0x4B,  0x4C,  0x7C,  0x42,  0x44,   //  88
     0x46,  0x48,  0x48,  0x46,  0x44,  0x42,  0x7C,  0x42,   //  96
     0x44,  0x46,  0x48,  0x48,  0x46,  0x44,  0x42,  0x4F,   // 104
     0x42,  0x44,  0x46,  0x48,  0x4A,  0x4D,  0x4E,  0x57,   // 112
     0x50,  0x51,  0x52,  0x53,  0x54,  0x55,  0x56,  0xC0,   // 120
     0xC3,  0xC6,  0xCF, 0x1DC,     0,     0 };               // 128
     /* wB05C & wB05E assumed at end */


//static byte tx2Buf[512]; use larger buf in main6.c
// static byte tx1Buf[512]; use larger buffer in plm0a.c
word cfCmdBuf[5];               // буфер для формирования CF-последовательностей
offset_t blkCurInfo[20];
word stackPC[20];               // стек pc (programm counter)
word wB4B0[20];
word stackSP[20];               // стек SP
byte extProcId[20];
byte procChainNext[20];
word wB528[10];
word wB53C[10];
byte tx2opc[255] = {T2_SEMICOLON, T2_LOCALLABEL, T2_SEMICOLON, T2_SEMICOLON};
byte tx2Aux1b[255] = {12, 9};
byte tx2Aux2b[255];
word tx2op1[255];
word tx2op2[255];
word tx2op3[255] = {0, 0, 0, 0};
word tx2Auxw[255] = {0, 1};     // количество упоминаний переменной или числа в одном блоке???
byte bC045[9];
byte bC04E[9];
bool boC057[9];
bool boC060[9];
bool boC069[9];
bool boC072[9];
bool boC07B[9];
word wC084[9];
word wC096[9];
byte bC0A8[9];
byte bC0B1;
byte bC0B2;
byte curTx2aux1b[2];    // aub1b для каждого параметра операнда (на генераторе CF)
byte curTx2aux2b[2];    // aub2b для каждого параметра операнда (на генераторе CF)
byte curOpParams[2];    // параметры операнда (op1 & op2) (на генераторе CF)
byte bC0B9[2];
byte bC0BB[2];
byte bC0BD[2];
byte bC0BF[2];
byte bC0C1[2];
byte bC0C3[125];
byte bC140[125];
byte bC1BD = 0;
byte tx2qp;             // позиция для записи в буфер
byte tx2lastPos = 4;    // последняя позиция в буфере, пригодная для обработки данных
byte tx2qEnd = 4;       // позиция последних считанных данных в буфере
word curPC = 0;         // текущее значение регистра "PC" (в текущем блоке)
word parInStack = 0;    // количество временных значений, сохраненных в стеке
word curSP = 0;         // текущее значение регистра "SP" (в текущем блоке)
word wC1C7 = 0;
byte blkSP = 0;
byte blkOverCnt = 0;
byte procCallDepth = 0;
bool boC1CC = false;
bool boC1CD;
bool eofSeen = false;
word wC1CF = 0;
byte curOp;                     // текущая операция T2
byte curOpFlags;                // и ее флаги
byte padC1D3;
byte curExtProcId = 1;
byte procChainId = 0;
bool boC1D8 = false;
byte bC1D9;
byte cfrag1;
byte cfCmdBufPos;       // позиция в буфере cfCmdBuf[] при его заполнении
byte lenBufTx1;
byte recBufTx1[34];
byte bC209[] = {4, 5, 3, 2, 0, 1};
bool boC20F = false;




static void Sub_3F27()
{
    byte emsg[] = "COMPILER ERROR: INSUFFICIENT MEMORY FOR CODE GENERATION";
    MEMORY = 0xC3BD;  // to align with ov2

    botMem = MEMORY + 256;
    if (w3822 < botMem)
        Fatal(emsg, Length(emsg));
    CreatF(&tx1File, tx1Buf, 512, 2);
    CreatF(&tx2File, tx2Buf, 512, 1);
    memset(cfCmdBuf, 0, 10);
    blkCurInfo[0] = procInfo[1] + botInfo;
    programErrCnt = 0;
} /* Sub_3F27() */


static void Sub_3F7D()
{
    curInfoP = procInfo[1] + botInfo;
    SetDimension(curPC);
    SetBaseVal(curSP);
    Fflush(&tx1File);
} /* Sub_3F7D() */

word Start2()
{
    if (setjmp(exception) == 0)
    {
        Sub_3F27();
        while (1)
        {
            FillTx2Q();
            setTx2lastPos();
            if (tx2opc[4] == T2_EOF)
                break;
            m2_Parse1();
            m2_Parse2();
            m2_Parse3();
        }
    } else {
        errorLevel = 1;
    }
    /* longjmp comes here */
    Sub_3F7D();
    return 3; // Chain(overlay[3]);
}
