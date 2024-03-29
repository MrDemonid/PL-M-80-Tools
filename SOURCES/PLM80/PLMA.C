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
#include <stdlib.h>


static byte signonMsg[] = "\r\nISIS-II PL/M-80 COMPILER ";
static byte noMemMsg[] = "NOT ENOUGH MEMORY FOR A COMPILATION";
static byte aIxi[] = ".IXI";
static byte aObj[] = ".OBJ";
static byte aLst[] = ".LST";
static byte aInvocationComm[] = "INVOCATION COMMAND DOES NOT END WITH <CR><LF>";
static byte aIncorrectDevice[] = "INCORRECT DEVICE SPEC";
static byte aSourceFileNotDisk[] = "SOURCE FILE NOT A DISKETTE FILE";
static byte aSourceFileName[] = "SOURCE FILE NAME INCORRECT";
static byte aSourceFileBadExt[] = "SOURCE FILE EXTENSION INCORRECT";
static byte aIllegalCommand[] = "ILLEGAL COMMAND TAIL SYNTAX";

static byte  ioBuffer[128];     // only 128 bytes read so 2048 was overkill
pointer cmdTextP;


static void SkipSpace()
{
    while (*cmdTextP == ' ' || *cmdTextP == '&') {
        if (*cmdTextP == ' ')
            cmdTextP++;
        else if (cmdLineP->link) {
            cmdLineP = cmdLineP->link;
            cmdTextP = &cmdLineP->pstr[1];
        }
    }
} /* SkipSpace() */

/*
  !!! ���㡠�� �� �������������

static bool TestToken(pointer str, byte len)
{
    pointer p;

    p = cmdTextP;
    while (len-- != 0) {
        if ((*cmdTextP++ & 0x5F) != *str++) {
            cmdTextP = p;
            return false;
        }
    }
    return true;
}
*/


static void SkipAlphaNum()
{
    while ('A' <= *cmdTextP && *cmdTextP <= 'Z' || 'a' <= *cmdTextP && *cmdTextP <= 'z'
                || '0' <= *cmdTextP && *cmdTextP <= '9' || *cmdTextP == '_')
        cmdTextP++;
} /* SkipAlphaNum() */



static void GetCmdLine()
{
    word actual, status;
    byte i;
    bool inQuote;
    cmd_t *ptr;

    Rescan(1, &status);     // no need for LocalRescan
    if (status != 0)
        FatlIO(&conFile, status);
    startCmdLineP = NULL;
    cmdLineP = NULL;

    for (;;) {
        ReadF(&conFile, ioBuffer, 128, &actual);
        if (ioBuffer[actual - 1] != '\n' || ioBuffer[actual - 2] != '\r')
            Fatal(aInvocationComm, Length(aInvocationComm));

        ptr = malloc(actual + 2 + sizeof(cmd_t));
        if (startCmdLineP == NULL)
            startCmdLineP = ptr;
        else
            cmdLineP->link = ptr;

        cmdLineP = ptr;
        cmdLineP->pstr[0] = (byte)actual;
        memmove(&cmdLineP->pstr[1], ioBuffer, actual);

        inQuote = false;
        for (i = 0; i < actual; i++) {
            if (ioBuffer[i] == QUOTE)
                inQuote = ! inQuote;
            else if (ioBuffer[i] == '&')
                if (! inQuote)
                    goto extend;
        }
        cmdLineP->link = NULL;
        cmdLineP = startCmdLineP;
        return;
    extend:
        PrintStr("**", 2);
    }
} /* GetCmdLine() */


static void ParseInvokeName()
{
    //pointer startP;
    //word len;
    //word p;

    SkipSpace();
//    debugFlag = TestToken("DEBUG", 5);        // cannot occur under windows
//    SkipSpace();
    //startP = cmdTextP;
    if (*cmdTextP == ':')
        cmdTextP += 4;    // skip drive
    SkipAlphaNum();
    //if ((len = (word)(cmdTextP - startP)) > 10)
    //    len = 10;
    /* plm had the overlay & invokename (ov0) split out
     * I have combined them to avoid spaces in address ranges
     * note invokeName is now overlay[0]
     * overlayN is now overlay[N]
     * In the end this code is not actually needed as overlays are
     * handled differently
     */
    //for (p = 0; p <= 6; p++) {
    //    memmove(overlay[p], startP, len);               // copy fileName
    //    memmove(overlay[p] + len, overlay[p] + 10, 5);  // move up the ext
    //}
} /* ParseInvokeName() */


void isis2dos(char *src, char *dst)
{
    char *s, *d;
    int i;

    s = src;
    d = dst;
    if (*s == ':')
    {
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
    }
    for (i = 0; i < 8; i++)
    {
        if (*s == '.')
            break;
        if (*s < ' ')
        {
            *d = 0;
            return;
        }
        *d++ = *s++;
    }
    for (i = 0; i < 4; i++)
    {
        if (*s <= ' ')
            break;
        *d++ = *s++;
    }
    *d = 0;
}



static void ParseSrcFileName()
{
    pointer fullName;
    pointer fileName;
    word nameLen;
    char fname[256];

    while (*cmdTextP != ' ' && *cmdTextP != '\r' && *cmdTextP != '&')
        cmdTextP++;
    SkipSpace();

    // �뢮��� ��� ��室���� 䠩��
    isis2dos(cmdTextP, fname);
    strcat(fname, "\n");
    PrintStr(fname, strlen(fname));

    fullName = cmdTextP;
    if (*cmdTextP == ':') {
        if (cmdTextP[3] != ':')
            Fatal(aIncorrectDevice, Length(aIncorrectDevice));
        if (cmdTextP[1] >= 'a')
            cmdTextP[1] &= 0x5F;
        if (cmdTextP[1] != 'F')
            Fatal(aSourceFileNotDisk, Length(aSourceFileNotDisk));
        cmdTextP += 4;
    }
    fileName = cmdTextP;
    SkipAlphaNum();
    if ((nameLen = (word)(cmdTextP - fileName)) == 0 || nameLen > 8)
        Fatal(aSourceFileName, Length(aSourceFileName));
    srcStemLen = (byte)(cmdTextP - fullName);
    memset(srcStemName, ' ', 12);
    memmove(srcStemName, fullName, srcStemLen);
    if (*cmdTextP == '.') {
        fileName = ++cmdTextP;
        SkipAlphaNum();
        if ((nameLen = (word)(cmdTextP - fileName)) == 0 || nameLen > 3)
            Fatal(aSourceFileBadExt, Length(aSourceFileBadExt));
    }
    nameLen = (word)(cmdTextP - fullName);
    srcFileIdx = 0;
    memset(srcFileTable, ' ', 18);
    memmove(srcFileTable, fullName, nameLen);
    memset(&srcFileTable[8], 0, 4);
    SkipSpace();
    if (*cmdTextP == '$')
        Fatal(aIllegalCommand, Length(aIllegalCommand));
    if (*cmdTextP == '\r')
        offFirstChM1 = 0;
    else
        offFirstChM1 = ((word)(cmdTextP - (pointer) cmdLineP)) - 1;
} /* ParseSrcFileName() */

static void InitFilesAndDefaults()
{
    LEFTMARGIN = 1;
    memset(ixiFileName, ' ', FILE_NAME_LEN);
    memmove(ixiFileName, srcStemName, srcStemLen);
    memmove(&ixiFileName[srcStemLen], aIxi, 4);
    InitF(&ixiFile, "IXREF ", ixiFileName);
    objBlk = objByte = 0;
    memset(objFileName, ' ', FILE_NAME_LEN);
    memmove(objFileName, srcStemName, srcStemLen);
    memmove(&objFileName[srcStemLen], aObj, 4);
    InitF(&objFile, "OBJECT", objFileName);
    memset(lstFileName, ' ', FILE_NAME_LEN);
    memmove(lstFileName, srcStemName, srcStemLen);
    memmove(&lstFileName[srcStemLen], aLst, 4);
    InitF(&lstFil, "LIST ", lstFileName);
    InitF(&tx1File, "UT1 ", ":F1:PLMTX1.TMP ");
    InitF(&tx2File, "UT2 ", ":F1:PLMTX2.TMP ");
    InitF(&atFile, "AT  ", ":F1:PLMAT.TMP ");
    InitF(&nmsFile, "NAMES ", ":F1:PLMNMS.TMP ");
    InitF(&xrfFile, "XREF ", ":F1:PLMXRF.TMP ");
    IXREF = false;
    IXREFSet = false;
    PRINT = true;
    PRINTSet = true;
    XREF = false;
    SYMBOLS = false;
    DEBUG = false;
    PAGING = true;
    OBJECT = true;
    OBJECTSet = true;
    OPTIMIZE = true;
    SetDate(" ", 1);
    SetPageLen(57);
    SetMarkerInfo(20, '-', 21);
    SetPageNo(0);
    SetMarginAndTabW(0xFF, 4);
    SetTitle(" ", 1);
    SetPageWidth(120);
} /* InitFilesAndDefaults() */

void SignOnAndGetSourceName()
{
    memmove(szVersion, verNo, 4);
    /*
      clear files structures
    */
    memset(&srcFil , 0, sizeof(file_t));
    memset(&lstFil , 0, sizeof(file_t));
    memset(&objFile, 0, sizeof(file_t));
    memset(&conFile, 0, sizeof(file_t));
    memset(&tx1File, 0, sizeof(file_t));
    memset(&tx2File, 0, sizeof(file_t));
    memset(&atFile , 0, sizeof(file_t));
    memset(&nmsFile, 0, sizeof(file_t));
    memset(&xrfFile, 0, sizeof(file_t));
    memset(&ixiFile, 0, sizeof(file_t));
    //
    InitF(&conFile, "CONSOL", ":CI: ");
    OpenF(&conFile, 1);
    topMem = MemCk() - 12;
    if (topMem < 0xC000)
        Fatal(noMemMsg, Length(noMemMsg));
    GetCmdLine();
    PrintStr(signonMsg, Length(signonMsg));
    PrintStr(szVersion, 4);
    PrintStr("\r\n", 2);
    cmdTextP = &cmdLineP->pstr[1];
    blkSize1 = topMem - blkSize1 - 256;
    blkSize2 = topMem - blkSize2 - 256;
    ParseInvokeName();
    ParseSrcFileName();
    InitFilesAndDefaults();
} /* SignOnAndGetSourceName() */
