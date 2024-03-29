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

void Sub_3F96()
{
    if (PRINT) {
        NewPageNextChLst();
        Xputstr2cLst("ISIS-II PL/M-80 ", 0);
        XwrnstrLst(szVersion, 4);
        Xputstr2cLst(" COMPILATION OF MODULE ", 0);
        curInfoP = botInfo + procInfo[1];
        curSymbolP = GetSymbol();
        if (curSymbolP != 0)
            XwrnstrLst(&SymbolP(curSymbolP)->name[1], SymbolP(curSymbolP)->name[0]);
        NewLineLst();
        if (OBJECT)
            Xputstr2cLst("NO OBJECT MODULE GENERATED", 0);
        else
            Xputstr2cLst("NO OBJECT MODULE REQUESTED", 0);
        NewLineLst();
        Xputstr2cLst("COMPILER INVOKED by:  ", 0);
        cmdLineP = startCmdLineP;
        while (cmdLineP != NULL) {
            TabLst(-23);
            Xputstr2cLst(&cmdLineP->pstr[1], '\r');
            cmdLineP = cmdLineP->link;
        }
        NewLineLst();
        SetSkipLst(3);
    }
}

byte tx2Buf[2048];
byte nmsBuf[2048];
byte lstBuf_6[2048];

void Sub_404A()
{
    if (PRINT) {
        lBufP = lstBuf_6;
        lBufSz = 2047;
    }
    b7AD9 = PRINT | OBJECT;
    if (OBJECT)
        DeletF(&objFile);
    if (! lfOpen && PRINTSet) {
        DeletF(&lstFil);
        PRINTSet = false;
    }
    CloseF(&tx1File);
#ifdef _DEBUG
    copyFile(tx1File.fNam, "plmtx1.tmp_main6");
#endif
    DeletF(&tx1File);
    CreatF(&tx2File, tx2Buf, 0x800, 1);
    if (b7AD9 || IXREF)
        CreatF(&nmsFile, nmsBuf, 0x800, 1);
    stmtNo = 0;
    if (PRINT) {
        srcFileIdx = 0;
        InitF(&srcFil, "SOURCE", (pointer)&srcFileTable[srcFileIdx]); /* note word array used */
        OpenF(&srcFil, 1);
    }
    curInfoP = procInfo[1] + botInfo;
    SetSkipLst(3);
    SetMarkerInfo(11, '-', 15);
    if (fatalErrorCode > 0) {
        errData.stmt = errData.info = 0;
        errData.num = fatalErrorCode;
        EmitError();
        SetSkipLst(2);
    }
    listing = PRINT;
    listOff = false;
    codeOn = false;
    programErrCnt = linesRead = csegSize = 0;
}

void Sub_4149()     // similar to Sub_4201 in main3.c
{
    topSymbol = localLabelsP - 3;
    curSymbolP = topSymbol - 1;
    Fread(&nmsFile, &b7ADA, 1);
    while (b7ADA != 0) {
        curSymbolP = curSymbolP - b7ADA - 1;
        SymbolP(curSymbolP)->name[0] = b7ADA;
        Fread(&nmsFile, &SymbolP(curSymbolP)->name[1], b7ADA);
        Fread(&nmsFile, &b7ADA, 1);
    }
    botSymbol = curSymbolP + 4;
    // botMem = botSymbol;
}


void Sub_41B6()
{
    CloseF(&atFile);
#ifdef _DEBUG
    copyFile(atFile.fNam, "plmat.tmp_main6");
#endif
    DeletF(&atFile);
    CloseF(&tx2File);
#ifdef _DEBUG
    copyFile(tx2File.fNam, "plmtx2.tmp_main6");
#endif
    DeletF(&tx2File);
    if (b7AD9 || IXREF) {
        CloseF(&nmsFile);
#ifdef _DEBUG
        copyFile(nmsFile.fNam, "plmnms.tmp_main6");
#endif
        DeletF(&nmsFile);
    }
    linesRead = lineCnt;
    if (PRINT) {
        TellF(&srcFil, (loc_t *)&srcFileTable[srcFileIdx + 9]);
        Backup((loc_t *)&srcFileTable[srcFileIdx + 9], offLastCh - offCurCh);
        CloseF(&srcFil);
        FlushLstBuf();
    }
}

word Start6()
{
    if (setjmp(exception) == 0)
    {
        Sub_404A();
        if (b7AD9 || IXREF)
        {
            Sub_4149();
            #ifdef _DEBUG
                symMode = 2;
            #endif
        }
        Sub_3F96();
        while (b7AE4)
        {
            Sub_42E7();
        }

        EmitLinePrefix();
    } else {
        errorLevel = 1;
    }
    Sub_41B6();
    if (PRINT || IXREF)
    {
        if (XREF ||  SYMBOLS || IXREF)
            return 5; // Chain(&overlay5);
        else
            LstModuleInfo();
    }
    return 254;                 // �����蠥� �ணࠬ��
}
