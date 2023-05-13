/****************************************************************************
 *  Link v 4.0                                                              *
 *  Copyright (C) 2023 Andrey Hlus                                          *
 *                                                                          *
 *  Created based on:                                                       *
 *  C port of Intel's LINK v3.0                                             *
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
 *                                                                          *
 ****************************************************************************/


/*
 * vim:ts=4:shiftwidth=4:expandtab:
 */
#include "link.h"

word inFile;
word tofilefd;
word printFileNo;
word pad_4565;
word tmpfilefd;
word statusIO;
word actRead;
byte inFileName[FULL_NAME_LEN+2];   // длина имени + байт длины + завершающий ноль
byte toFileName[FULL_NAME_LEN+2];
byte printFileName[FULL_NAME_LEN+2];
byte filePath[FULL_NAME_LEN+2];
byte linkTmpFile[FULL_NAME_LEN+2];
bool mapWanted;
byte outModuleName[32];
byte modEndModTyp;
byte outTranId;
byte outTranVn;
byte modEndSegId;
word modEndOffset;
word segLen[6];
byte alignType[6];
byte segmap[256];
pointer membot;
pointer topHeap;
record_t *inRecordP;
pointer erecP;
pointer inP;
word recNum;
word recLen;
word npbuf;
pointer sbufP;
pointer bufP;
pointer ebufP;
pointer soutP;
pointer outP;
pointer eoutP;
library_t *objFileHead;
library_t *curObjFile;
module_t *curModule;
symbol_t *hashTab[128];
symbol_t *headSegOrderLink;
symbol_t *comdefInfoP;
symbol_t *symbolP;
word unresolved;
word maxExternCnt;
symbol_t *headUnresolved;
byte CRLF[2] = "\r\n";
byte recErrMsg[] =  " RECORD TYPE XXH, RECORD NUMBER *****\r\n";
                    //         13-^               32-^
word inBlk;
word inByt;
pointer inbP;
byte inCRC;

byte COPYRIGHT[] = "[C] 1976, 1977, 1979 INTEL CORP'";
byte VERSION[] = "V4.0";
byte DUMMYREC[] = {0,0,0};

/* EXTERNALS */
extern byte overlayVersion[4];

void ConOutStr(pointer pstr, word count)
{
    Write(0, pstr, count, &statusIO);
} /* ConOutStr() */

void fatal_Error(byte errCode)
{
    ConOutStr(" ", 1);
    ConOutStr(&inFileName[1], inFileName[0]);
    if (curModule)
    {
        ConOutStr("(", 1);
        ConOutStr(&curModule->name[1], curModule->name[0]);
        ConOutStr(")", 1);
    }
    ConOutStr(",", 1);
    ReportError(errCode);
    BinAsc(inRecordP->rectyp, 16, '0', &recErrMsg[13], 2);
    if (recNum > 0 )
        BinAsc(recNum, 10, ' ', &recErrMsg[32], 5);
    ConOutStr(recErrMsg, sizeof(recErrMsg) - 1);
    Exit();
} /* fatal_Error() */

void fatal_RecFormat()
{
    fatal_Error(ERR218);   /* Illegal() record format */
} /* fatal_RecFormat() */

void fatal_RelocRec()
{
    fatal_Error(ERR212);   /* Illegal() relo record */
} /* fatal_RelocRec() */

void fatal_RecSeq()
{
    fatal_Error(ERR224);   /* Bad() record sequence */
} /* fatal_RecSeq() */

void Pstrcpy(pointer psrc, pointer pdst)
{
    memmove(pdst, psrc, psrc[0] + 1);
} /* Pstrcpy() */

byte HashF(pointer pstr)
{
    byte i, j;

    j = 0;
    for (i = 0; i <= pstr[0]; i++) {
        j = RorB(j, 1) ^ pstr[i];
    }
    return j & 0x7F;
} /* HashF() */

bool Lookup(pointer pstr, symbol_t **pitemRef, byte mask)
{
    symbol_t *p;
    byte i;

    i = pstr[0] + 1;     /* Size() of string including length() byte */
    *pitemRef = (p = (symbol_t *)&hashTab[HashF(pstr)]);
    p = p->hashLink;
    while (p)
    {   /* chase down the list to look for the name */
        *pitemRef = p;
        if ((p->flags & mask) != AUNKNOWN ) /* ignore undef entries */
            if (Strequ(pstr, p->name, i) )
                return TRUE;
        p = p->hashLink;  /* next */
    }
    return false;
} /* Lookup() */

void WriteBytes(pointer bufP, word count)
{
    Write(printFileNo, bufP, count, &statusIO);
    fatal_FileIO(statusIO, &printFileName[1], TRUE);
} /* WriteBytes() */

void WriteCRLF()
{
    WriteBytes(CRLF, 2);
} /* WriteCRLF() */

void WriteAndEcho(pointer buffP, word count)
{

    WriteBytes(buffP, count);
    if (printFileNo > 0 )
        ConOutStr(buffP, count);
} /* WriteAndEcho() */

void WAEFnAndMod(pointer buffP, word count)
{
    WriteAndEcho(buffP, count);
    WriteAndEcho(&inFileName[1], inFileName[0]);
    WriteAndEcho("(", 1);
    WriteAndEcho(&curModule->name[1], curModule->name[0]);
    WriteAndEcho(")\r\n", 3);
} /* WAEFnAndMod */

int Start()
{
    ParseCmdLine();
    Phase1();
//    Load(&filePath[1], 0, 0, &actRead, &statusIO);  /* Load() the overlay */
//    fatal_FileIO(statusIO, &filePath[1], TRUE);
//    if (! Strequ(VERSION, overlayVersion, 4) )    /* make sure it is valid */
//        fatal_FileIO(ERR219, &filePath[1], TRUE);  /* phase Error() */
    Phase2();
    Close(printFileNo, &statusIO);
    ConOutStr("\n", strlen("\n"));      // переходим на новую строку перед выходом
    if (statusIO != ERROR_SUCCESS)
        return 0;
    return -1;
} /* Start */

