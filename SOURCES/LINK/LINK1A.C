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


#include "link.h"

pointer GetHigh(word count)
{
    if (topHeap - membot < count)
        fatal_FileIO(ERR210, &toFileName[1], TRUE);    /* Insufficient() memory */
    return (membot = membot + count) - count;
} /* GetHigh() */

pointer GetLow(word count)
{
    if (topHeap - membot < count)
        fatal_FileIO(ERR210, &toFileName[1], TRUE);    /* Insufficient() memory */
    return (topHeap = topHeap - count);
} /* GetLow() */


void ChkRead(word cnt)
{   /* make sure next cnt bytes are in the Input() buffer */
    word bcnt;

    /* see if (enough bytes in the i/o buf. if ! shuffle down && reload more */
    if ((bcnt = (word)(ebufP - bufP)) < cnt)
    {
        memmove(sbufP, bufP, bcnt);
        Read(inFile, sbufP + bcnt, npbuf - bcnt, &actRead, &statusIO);
        fatal_FileIO(statusIO, &inFileName[1], TRUE);
        /* calculate new inBlk and inByt */
        inBlk = inBlk + (word)(inByt + bufP - sbufP) / 128;
        inByt = (inByt + bufP - sbufP) % 128;
        if ((bcnt = bcnt + actRead) < cnt)
            fatal_FileIO(ERR204, &inFileName[1], TRUE);    /* Premature() eof */
        /* mark the new end */
        ebufP = (bufP = sbufP) + bcnt;
    }
} /* ChkRead() */

void GetRecord()
{
    word bcnt;

    if ((bcnt = (word)(ebufP - bufP)) >= 4)
    {
        inRecordP = (record_t *)bufP;
        erecP = ((pointer)inRecordP) + inRecordP->reclen + 2;   /* type + data + crc */
    }
    else
    {
        inRecordP = (record_t *)DUMMYREC;
        erecP = NULL;           // isis sets to top of memory, doesn't work here
    }
    if (erecP == NULL || erecP >= ebufP)
        if (inRecordP->reclen <= 1025)  /* should be able to get all of record in buffer */
        {
            ChkRead(bcnt + 1);
            inRecordP = (record_t *)bufP;
            if ((erecP = ((pointer)inRecordP) + inRecordP->reclen + 2) >= ebufP) /* redundant - done in ChkRead() */
                fatal_FileIO(ERR204, &inFileName[1], TRUE);    /* premature eof */
        }
    recLen = inRecordP->reclen;
    inP =  inRecordP->record;
    bufP = erecP + 1;
    recNum = recNum + 1;
    if (inRecordP->rectyp > R_COMDEF || (inRecordP->rectyp & 1))    /* > 0x2E || odd */
        fatal_RelocRec();
    if (inRecordP->rectyp == R_MODDAT)          /* content handled specially */
        return;
    if (inRecordP->rectyp >= R_LIBLOC  && inRecordP->rectyp <= R_LIBDIC)
        return;                     /* library records handled specially */
    if (recLen > 1025)
        fatal_Error(ERR211);   /* record too long */
    inCRC = 0;                  /* test checksum */
    for (inbP = (pointer)inRecordP; inbP <= erecP; inbP++) {
        inCRC = inCRC + *inbP;
    }
    if (inCRC != 0)
        fatal_Error(ERR208);   /* checksum Error() */
} /* GetRecord() */

void Position(word blk, word byt)
{   /* Seek() in Input() buffer */
    /* check if (already in memory, if so update bufP only */
    if (inBlk <= blk && blk <= (inByt + (ebufP - sbufP))  / 128 + inBlk)
    {
        if ((bufP = sbufP + (blk - inBlk) * 128 + (byt - inByt))
            >= sbufP && bufP < ebufP)
            return;
    }
    Seek(inFile, 2, &blk, &byt, &statusIO);     /* Seek() on disk */
    fatal_FileIO(statusIO, &inFileName[1], true);
    recNum = 0;                     /* reset vars and Read() at least 1 byte */
    bufP = ebufP;
    ChkRead(1);
    inBlk = blk;
    inByt = byt;
} /* Position() */

void OpenObjFile(bool show)
{
    Pstrcpy(curObjFile->name, &inFileName[0]);      /* copy the user supplied file name */
    if (show)
    {
        ConOutStr("  + ", 4);
        ConOutStr(&inFileName[1], inFileName[0]);
        ConOutStr("\n", strlen("\n"));
    }
    inFileName[inFileName[0]+1] = ' ';          /* terminate with a space */
    Open(&inFile, &inFileName[1], 1, 0, &statusIO); /* Open() the file */
    fatal_FileIO(statusIO, &inFileName[1], TRUE);
    recNum = 0;
    curModule = 0;                  /* reset vars and Read() at least 1 byte */
    bufP = ebufP;
    ChkRead(1);
    inBlk = inByt = 0;
} /* OpenObjFile() */

void CloseObjFile()
{                   /* Close() file and link to next one */
    Close(inFile, &statusIO);
    fatal_FileIO(statusIO, &inFileName[1], TRUE);
    curObjFile = curObjFile->link;
} /* CloseObjFile() */

