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

bool pendingInclude = false;
bool includeOnCmdLine = false;
#pragma off(unreferenced)
static byte padb6C23;
#pragma on(unreferenced)
byte fileIdx = {0};
byte *endInBufP = inBuf;
bool missingEnd = false;
word srcfd;
word rootfd;
byte *inChP = inBuf - 1;
byte *startLineP = inBuf;
byte lineChCnt = 0;
file_t files[6];
static word seekIBlk;
static word seekIByte;
#pragma off(unreferenced)
static byte pad6CAD;
#pragma on(unreferenced)
static byte *savInBufP;
static byte *savEndInBufP;
#pragma off(unreferenced)
static word pad6CB2[4];
#pragma on(unreferenced)
static word readFActual;
#pragma off(unreferenced)
static word pad6CBC;
#pragma on(unreferenced)


void ReadF(byte conn, byte *buffP, word count)
{
    Read(conn, buffP, count, &readFActual, &statusIO);
    IoErrChk();
}

void SeekI(byte seekOp)
{
    Seek(srcfd, seekOp, &seekIBlk, &seekIByte, &statusIO);
    IoErrChk();
}


void ReadSrc(byte *bufLoc)
{
 //   byte pad;

    ReadF((byte)srcfd, bufLoc, (word)(&inBuf[sizeInBuf] - bufLoc));
    endInBufP = bufLoc + readFActual;
}



void CloseSrc() /* close current source file. Revert to any parent file */
{
    Close(srcfd, &statusIO);
    IoErrChk();
    if (fileIdx == 0) {         /* if it the original file we had no end statement so error */
        missingEnd = true;
        IoError(files[0].name);
        return;
    }
    fileIdx--;
    /* Open() the previous file */
    if (fileIdx == 0)           /* original source is kept open across include files */
        srcfd = rootfd;
    else
        srcfd = SafeOpen(files[fileIdx].name, READ_MODE);

    seekIByte = files[fileIdx].byt;    /* move to saved location */
    seekIBlk = files[fileIdx].blk;
    SeekI(SEEKABS);
    endInBufP = inBuf;        /* force Read() */
    inChP = inBuf - 1;
}


byte GetSrcCh() /* get next source character */
{
    byte *insertPt;
    while (1)
    {
        inChP++;

        if (inChP == endInBufP)
        {
            /* buffer all used */
            savInBufP = startLineP;
            savEndInBufP = endInBufP;
            /* copy the current line down to start of buffer */
            if (savEndInBufP - savInBufP > 0)
                memcpy(inBuf, startLineP, savEndInBufP - savInBufP);
            startLineP = inBuf;
            /* Read() in  characters to rest of inBuf */
            ReadSrc(insertPt = startLineP + (savEndInBufP - savInBufP));
            inChP = insertPt;
        }

        if (readFActual == 0)
        {
            /* end of file so close this one*/
            CloseSrc();
            continue;
        }
        break;
    }

    lineChCnt++;                        // track chars on this line
    //if (inComment)
    //   return *inChP;
    return *inChP;       // remove parity
}


void OpenSrc()
{
    byte curByteLoc;
    word curBlkLoc;

    pendingInclude = false;
    SeekI(SEEKTELL);
    if (seekIByte == 128) {        /* adjust for 128 boundary */
        seekIBlk++;
        seekIByte = 0;
    }

    curBlkLoc = (word)(endInBufP - startLineP);    /* un-used characters */
//x:                        /* forces code alignment */
    if ((curByteLoc = curBlkLoc % 128) > seekIByte) {
        seekIByte += 128;           /* adjust to allow for un-used chars */
        seekIBlk--;
    }
    /* save the current file location */
    files[fileIdx - 1].byt = seekIByte - curByteLoc;
    files[fileIdx - 1].blk = seekIBlk - curBlkLoc / 128;
    if (srcfd != rootfd)        /* close if include file */
    {
        Close(srcfd, &statusIO);
        IoErrChk();
    }

    endInBufP = inBuf;            /* force Read() */
    inChP = endInBufP - 1;
    startLineP = inBuf;
    files[fileIdx].blk = 0;            /* record at start of file */
    files[fileIdx].byt = 0;
    srcfd = SafeOpen(files[fileIdx].name, READ_MODE);    /* Open() the file */
}
