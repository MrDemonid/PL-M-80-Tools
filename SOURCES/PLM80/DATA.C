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

offset_t topMem;
offset_t botMem;
offset_t botInfo;
offset_t topInfo;
offset_t topSymbol;
offset_t botSymbol;
offset_t curSymbolP;
offset_t curInfoP;
word offFirstChM1;
word LEFTMARGIN;
word localLabelCnt;
word srcFileIdx;
offset_t hashChains[64]; // offset is to pointer to array of offsets
word blockDepth;
offset_t localLabelsP;
offset_t w381E;
offset_t helpersP;
offset_t w3822;
word linesRead;
word programErrCnt;
word procCnt;
word cmdLineCaptured = 0;
word dsegSize = 0;
word csegSize = 0;
word objBlk;
word objByte;
word srcFileTable[66];
file_t srcFil;
file_t lstFil;
file_t objFile;
file_t conFile;
file_t tx1File;
file_t tx2File;
file_t atFile;
file_t nmsFile;
file_t xrfFile;
file_t ixiFile;
word procChains[35];
word procInfo[255];
word blk1Used = 400;
word blk2Used = 400;
offset_t ov1Boundary = 0x9F00;    // last address of ov1 rounded up to page boundary + 0x100
word blkSize1 = 0xC400;     // last address of ov2 rounded up to page boundary
word blkSize2 = 0xA400;     // last address of ov4 rounded up to page boundary
byte srcStemLen;
bool standAlone = true;
bool IXREFSet = true;
bool PRINTSet = true;
bool OBJECTSet = true;
bool afterEOF = false;
bool haveModuleLevelUnit = false;
byte fatalErrorCode = 0;
//byte pad3C43 = 1;
offset_t ov0Boundary = 0xA000;        // last address of ov0 rounded up to page boundary
byte controls[8];
//byte pad_3C4E[2];
byte srcStemName[12];
bool debugSwitches[26];
cmd_t *cmdLineP;
cmd_t *startCmdLineP;
//byte overlay[7][FILE_NAME_LEN] = { ":F0:PLM80 .OV0 ", ":F0:PLM80 .OV1 ", ":F0:PLM80 .OV2 ", ":F0:PLM80 .OV3 ",
//                                   ":F0:PLM80 .OV4 ", ":F0:PLM80 .OV5 ", ":F0:PLM80 .OV6 "};
byte ixiFileName[FILE_NAME_LEN];
byte lstFileName[FILE_NAME_LEN];
byte objFileName[FILE_NAME_LEN];
word pageNo = 0;
byte b3CF2;
pointer lBufP = &b3CF2;
word lChCnt = 0;
word lBufSz = 0;
bool lfOpen = false;
word linLft = 0;
byte wrapMarkerCol, wrapMarker, wrapTextCol;
byte col = 0;
byte skipCnt = 0;
byte tWidth = 0;
byte TITLELEN = 1;
word PAGELEN = 60;
byte PWIDTH = 120;
byte margin = 0xFF;
byte DATE[9];
byte plm80Compiler[] = "PL/M-80 COMPILER    ";
byte TITLE[60] = " ";
//word ISIS = 0x40;
word REBOOTVECTOR = 0;



byte intVecNum = 8;
word intVecLoc = 0;
bool hasErrors = false;
//byte overlay6[]  = ":F0:PLM80 ";
//byte ov6[] = ".OV6 ";
byte szVersion[] = "X000";
//byte pad3DA1;
//byte invokeName[] = ":F0:PLM80 ";
//byte ov0[] =  ".OV0 ";


#pragma off(unreferenced)
byte copyRight[] = "[C] 1976, 1977, 1982 INTEL CORP";
#pragma on(unreferenced)
