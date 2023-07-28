/****************************************************************************
 *  asm80 v 5.1                                                             *
 *  Copyright (C) 2023 Andrey Hlus                                          *
 *                                                                          *
 *  Created based on:                                                       *
 *  asm80: C port of ASM80 v4.1                                             *
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


#include "asm80.h"

typedef word offset;


#pragma off(unreferenced)
static const char cpyrite[] = "[C] 1976, 1977,1979 INTEL CORP\x2\x1";
#pragma on(unreferenced)
static const byte *headerMsg = "\r\nISIS-II ASSEMBLER SYMBOL CROSS REFERENCE, V2.1                     PAGE ";

struct {
    byte errCode;
    const byte *errStr;
} errStrs[] = {
        {2, "ILLEGAL AFTN ARGUMENT"},
        {4, "INCORRECTLY SPECIFIED FILE"},
        {5, "UNRECOGNIZED DEVICE NAME"},
        {6, "ATTEMPT TO WRITE TO INPUT DEVICE"},
        {8, "ATTEMPT TO READ FROM OUTPUT DEVICE"},
        {0xA, "NOT ON SAME DISK"},
        {0xB, "FILE ALREADY EXISTS"},
        {0xC, "FILE IS ALREADY OPEN"},
        {0xD, "NO SUCH FILE"},
        {0xE, "WRITE PROTECTED"},
        {0x11, "NOT A DISK FILE"},
        {0x12, "ILLEGAL ISIS COMMAND"},
        {0x13, "ATTEMPTED SEEK ON NON-DISK FILE"},
        {0x14, "ATTEMPTED BACK SEEK TOO FAR"},
        {0x15, "CAN''T RESCAN"},
        {0x16, "ILLEGAL ACCESS MODE TO OPEN"},
        {0x17, "MISSING FILENAME"},
        {0x19, "ILLEGAL ECHO FILE"},
        {0x1A, "ILLEGAL ATTRIBUTE IDENTIFIER"},
        {0x1B, "ILLEGAL SEEK COMMAND"},
        {0x1C, "MISSING EXTENSION"},
        {0x1F, "CAN''T SEEK ON WRITE ONLY FILE"},
        {0x20, "CAN''T DELETE OPEN FILE"},
        {0x23, "SEEK PAST EOF"},
        {0xC9, "UNRECOGNIZED SWITCH"},
        {0xCA, "UNRECOGNIZED DELIMITER"},
        {0xCB, "INVALID SYNTAX"},
        {0xCC, "PREMATURE EOF"},
        {0xCE, "ILLEGAL DISKETTE LABEL"},
        {0xD0, "CHECKSUM ERROR"},
        {0xD1, "RELO FILE SEQUENCE ERROR"},
        {0xD2, "INSUFFICIENT MEMORY"},
        {0xD3, "RECORD TOO LONG"},
        {0xD4, "ILLEGAL RELO RECORD"},
        {0xD5, "FIXUP BOUNDS ERROR"},
        {0xD6, "ILLEGAL SUBMIT PARAMETER"},
        {0xD7, "ARGUMENT TOO LONG"},
        {0xD8, "TOO MANY PARAMETERS"},
        {0xD9, "OBJECT RECORD TOO SHORT"},
        {0xDA, "ILLEGAL RECORD FORMAT"},
        {0xDB, "PHASE ERROR"},
        {0, ""}
};


typedef struct {
    char name[MAXSYMSIZE];
    offset head;            // offset to line ref chain
} xref_t;

typedef struct {
    word lineNum;
    offset next;
} line_t;

static inline byte *Offset2Ptr(offset off) {
    return MEMORY + off;
}
static inline xref_t *MkXrefPtr(offset off) {
    return (xref_t *)Offset2Ptr(off);
}
static inline line_t *MkLinePtr(offset off) {
    return (line_t *)Offset2Ptr(off);
}

word topLowHeap;
offset startMarker;
word tmpAFTN;
offset topHighHeap;
word connP;
word itemCount;
word botHighHeap;
word status;
byte outputLine[256];
byte col;
byte path[17];
byte row;
byte pageLength;
byte pageWidth;
byte recordType;
bool haveFileInfo;
bool paging;




static void ReportError(word scode)
{
    word status;
    int i;

    if (scode)
    {
        for (i = 0; errStrs[i].errCode && scode > errStrs[i].errCode; i++)
            ;
        if (scode == errStrs[i].errCode) {
            Write(0, " ", 1, &status);
            Write(0, errStrs[i].errStr, (word)strlen(errStrs[i].errStr), &status);
            Write(0, "\r\n", 2, &status);
        }
        else
            Error(scode);
    }
}

static void StatusChk(word scode)
{
    if (scode != 0)
    {
        ReportError(scode);
        Close(tmpAFTN, &status);
        Close(connP, &status);
        Exit();
    }
}

static void OpenTmp()
{
    word status;

    Open(&tmpAFTN, asxrefTmp, READ_MODE, 0, &status);
    StatusChk(status);
}


static void OpenListFile()
{
    dword dummy;

    if (path[0] == ':' && (path[1] & 0xdf) != 'F')    /* test as UCase */
        Open(&connP, path, WRITE_MODE, 0, &status);    /* device */
    else
        Open(&connP, path, UPDATE_MODE, 0, &status);    /* file */

    StatusChk(status);
    Seek(connP, SEEKEND, &dummy, &status);    /* seek end */
    if (status == 0x13)    /* bad seek on non file is ok */
        return;
    StatusChk(status);
}

static bool ReadTmp(byte *buf, byte cnt)
{
    word actual;

    Read(tmpAFTN, buf, cnt, &actual, &status);
    StatusChk(status);
    return actual == cnt;
}


static void WriteStr(const byte *str, word len)
{
    Write(connP, str, len, &status);
    StatusChk(status);
}

/*
static void WriteCStr(const byte *str) {
    WriteStr(str, (word)strlen(str));
}
*/

static void WriteCRLF()
{
    WriteStr("\r\n", 2);
}


static void ToNextPage()
{
    WriteStr("\f\r\n\r\n\r\n", 7);
}


static void OutStrCRLF(const byte *str, byte len)
{
    Write(0, str, len, &status);
    Write(0, "\r\n", 2, &status);
}


static NORETURN(FatalError(byte err))
{
    if (err == 0)
        OutStrCRLF("INPUT FILE FORMAT ERROR", 23);
    else
        OutStrCRLF("NOT ENOUGH MEMORY", 17);
    Close(tmpAFTN, &status);
    Exit();
}


static offset AllocLineRef()
{
    topLowHeap = topLowHeap + sizeof(line_t);
    if (topLowHeap <= botHighHeap)
        return topLowHeap - sizeof(line_t);
    FatalError(1);
    return 0;
}

static offset AllocXref()
{
    botHighHeap = botHighHeap - sizeof(xref_t);
    if (botHighHeap >= topLowHeap)
        return botHighHeap + 1;
    FatalError(1);
    return 0;
}

static offset FirstXref()
{
    return topHighHeap - sizeof(xref_t) + 1;
}


static bool TestMoreXrefs(offset ptr)
{
    return ptr > botHighHeap;
}

static offset NextXref(offset ptr)
{
    return ptr - sizeof(xref_t);
}

static bool CmpXrefNames(const char *str1, const char *str2)
{
    return memcmp(str1, str2, MAXSYMSIZE) == 0;
}

static offset FindXref(const char *name, bool *pFound)
{
    offset pTestXref, pNewXref;

    *pFound = false;
    pTestXref = FirstXref();
    while (!*pFound && TestMoreXrefs(pTestXref)) {
        if (CmpXrefNames(name, MkXrefPtr(pTestXref)->name))
            *pFound = true;
        else
            pTestXref = NextXref(pTestXref);
    }

    if (*pFound)
        return pTestXref;
    else
    {
        pNewXref = AllocXref();
        memcpy(MkXrefPtr(pNewXref)->name, name, MAXSYMSIZE);   // copy name to new entry
        itemCount++;
        return pNewXref;
    }
}


static void InsertXref(bool isDef, const char *name, word lineNum)
{
    bool found;

    xref_t *xrefP = MkXrefPtr(FindXref(name, &found));
    offset linePRef = AllocLineRef();
    line_t *lineP = MkLinePtr(linePRef);

    if (isDef)
        lineNum |= 0x8000;

    lineP->lineNum = lineNum;
    if (found)
    {
        lineP->next = MkLinePtr(xrefP->head)->next;
        MkLinePtr(xrefP->head)->next = linePRef;
    }
    else
        lineP->next = linePRef;

    xrefP->head = linePRef;
}

static offset GetPLineRef(byte from, offset ptr, bool *pMoreLineRefs)
{
    if (from == 0)    /* ptr -> Xref() */
    {
        *pMoreLineRefs = true;
        return startMarker = MkLinePtr(MkXrefPtr(ptr)->head)->next;
    }
    else
    {            /* ptr -> LineRef() */
        *pMoreLineRefs = startMarker != MkLinePtr(ptr)->next;
        return MkLinePtr(ptr)->next;
    }
}

static word GetLineNum(offset pLineRef, bool *pIsDef)
{
    word lineRef = MkLinePtr(pLineRef)->lineNum;
    if (lineRef & 0x8000)
        *pIsDef = true;
    else
        *pIsDef = false;
    return 0x7fff & lineRef;
}


/*
static void CopyXref(xref_t *pXref1, xref_t *pXref2) {
    *pXref2 = *pXref1;
}
*/

static xref_t *GetPXref(word n) {    /* get offset of Xref(n) */
    return MkXrefPtr(topHighHeap - n * sizeof(xref_t) + 1);
}


static byte GetTmpRecord()
{
    byte type;

    if (!ReadTmp(outputLine, 1) || outputLine[0] == '3')
        return XREF_EOF;

    type = outputLine[0] - '0';

    if (type > XREF_FIN || !ReadTmp(&outputLine[1], type == XREF_FIN ? 22 : 22))
        FatalError(0);
    return type;
}


static word Htoi(const char *buf, byte n)
{
    word val;

    for (val = 0; n != 0; buf++, n--)
        val = (val * 16) + (*buf <= '9' ? *buf - '0' : *buf - ('A' - 10));
    return val;
}


static byte *GetPSymbol()
{
    return &outputLine[1];
}

static word GetLineNumber()
{
    return Htoi(&outputLine[19], 4);
}

static void ProcXrefRecord(byte isDef)
{
    InsertXref(isDef, GetPSymbol(), GetLineNumber());
}

static void CopyFileName()
{
    memcpy(path, outputLine + 1, 17);
}



static void ProcFileRecord()
{
    CopyFileName();
    paging = outputLine[18];
    pageLength = (byte)Htoi(&outputLine[19], 2);
    pageWidth = (byte)Htoi(&outputLine[21], 2);
    haveFileInfo = true;
}

static void SortXrefs()
{
    word i, j, k, n;
    short m;
    xref_t tmpXref;


    i = j = k = 1;

    while (k < itemCount) {
        i = j;
        j = k;
        k = k * 3 + 1;
    }

    k = i;

    while (k != 0) {
        n = k + 1;
        while (n <= itemCount) {
            m = n - k;
            tmpXref = *GetPXref(n);
            while (m > 0 && memcmp(tmpXref.name, GetPXref(m)->name, MAXSYMSIZE) < 0) {
                *GetPXref(m + k) = *GetPXref(m);
                m = m - k;
            }
            *GetPXref(m + k) = tmpXref;
            n++;
        }
        k = (k - 1) / 3;
    }
}


static void Num2Ascii(word val, char *buf, byte len)
{
    char *s;

    memset(buf, ' ', len);
    s = buf + len - 1;

    *s = '0';
    while (val && s >= buf) {
        *s-- = val % 10 + '0';
        val /= 10;
    }
}

static void blankOutputLine()
{
    memset(outputLine, ' ', sizeof(outputLine));
}

static void PageHeader(byte pageNum)
{
    char asciiPageNum[3];

    Num2Ascii(pageNum, asciiPageNum, 3);
    ToNextPage();
    WriteStr(headerMsg + 2, 72);
    WriteStr(asciiPageNum, 3);
    WriteCRLF();
    WriteCRLF();
    row = 7;
}



static void NameToOutputLine(offset pXref)
{
    memcpy(&outputLine[col], MkXrefPtr(pXref)->name, MAXSYMSIZE);
    col += MAXSYMSIZE;
}

static void OutputLineRef(offset pLineRef)
{
    bool isDefinition;
    word lineNum;

    lineNum = GetLineNum(pLineRef, &isDefinition);
    Num2Ascii(lineNum, &outputLine[col], 4);
    col += 4;
    if (isDefinition)
        outputLine[col] = '#';
    col += 3;
}

static void OutputXref()
{
    byte refsPerLine, refsCnt, pageNum;

    offset pLineRef, pXref;
    bool moreLineRefs;

    refsPerLine = (pageWidth - 6) / 7;
    pageNum = 1;
    PageHeader(1);
    pXref = FirstXref();

    while (TestMoreXrefs(pXref)) {
        blankOutputLine();
        col = 0;
        NameToOutputLine(pXref);
        pLineRef = GetPLineRef(0, pXref, &moreLineRefs);
        while (moreLineRefs) {
            col = 19;
            refsCnt = 0;
            while (++refsCnt <= refsPerLine && moreLineRefs) {
                OutputLineRef(pLineRef);
                pLineRef = GetPLineRef(1, pLineRef, &moreLineRefs);
            }
            WriteStr(outputLine, col - 1);
            WriteCRLF();
            if (paging)
                if (++row == pageLength - 2)
                    PageHeader(++pageNum);
            if (moreLineRefs)
                blankOutputLine();
        }
        pXref = NextXref(pXref);
    }
}

void GenAsxref() {      // will reuse the asxrefTmp file from the main routine so no need to set drive
    itemCount = 0;
    topLowHeap = 0;
    botHighHeap = topHighHeap = (offset)(MemCk() - MEMORY);

    haveFileInfo = false;
    OpenTmp();
    OutStrCRLF(headerMsg, 51);

    while (1) {
        recordType = GetTmpRecord();
        if (recordType == XREF_DEF || recordType == XREF_REF)
            ProcXrefRecord(recordType == XREF_DEF);
        else if (recordType == XREF_FIN)
            ProcFileRecord();
        else
        {
            if (!haveFileInfo)
                FatalError(0);
            SortXrefs();
            OpenListFile();
            OutputXref();
            WriteCRLF();
            WriteStr("CROSS REFERENCE COMPLETE", 24);
            WriteCRLF();
            Close(tmpAFTN, &status);
            Close(connP, &status);
            Delete(asxrefTmp, &status);
            return;
        }
    }
}
