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
static dword seekOffs;
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
    Seek(srcfd, seekOp, &seekOffs, &statusIO);
    IoErrChk();
}


/*
  подгрузка текста в буфер, с корректировкой строк до PC-формата.
*/
void ReadSrc(byte *bufLoc)
{
    byte buff[IN_BUF_SIZE];
    int size;
    char *src;
    char *dst;
    static bool expected_LF = false;

    ReadF((byte)srcfd, buff, (word)(&inBuf[sizeInBuf] - bufLoc));
    size = readFActual;
    src = &buff;
    dst = bufLoc;

    while (size)
    {
        switch (*src)
        {
            case CR:
                if (expected_LF)
                {
                    // второй подряд CR (0x0D), вставляем LF (0x0A) после первого
                    *dst++ = LF;
                    readFActual++;
                }
                expected_LF = true;
                break;

            case LF:
                if (!expected_LF)
                {
                    // коенц строки формата UNIX, преобразуем в PC-формат
                    *dst++ = CR;
                    readFActual++;
                }
                expected_LF = false;
                break;

            default:
                if (expected_LF)
                {
                    // редкий вид строк без LF (0x0A)
                    *dst++ = LF;
                    readFActual++;
                    expected_LF = false;
                }
                break;
        }
        *dst++ = *src++;
        size--;
    }
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

    /*
      восстанавливаем входной буфер и позицию в файле
    */
    seekOffs = files[fileIdx].offs;
    SeekI(SEEKABS);
    memcpy(inBuf, files[fileIdx].buff, IN_BUF_SIZE);
    endInBufP   = files[fileIdx].pEndBuf;
    startLineP  = files[fileIdx].pStartLine;
    inChP       = files[fileIdx].pCurCh;
    readFActual = files[fileIdx].nReaded;
    lineChCnt   = files[fileIdx].lineCnt;
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
    pendingInclude = false;
    SeekI(SEEKTELL);
    /*
      сохраняем текущий буфер и позицию в файле
    */
    files[fileIdx - 1].offs = seekOffs;
    memcpy(files[fileIdx - 1].buff, inBuf, IN_BUF_SIZE);
    files[fileIdx - 1].pEndBuf   = endInBufP;
    files[fileIdx - 1].pStartLine  = startLineP;
    files[fileIdx - 1].pCurCh       = inChP;
    files[fileIdx - 1].nReaded = readFActual;
    files[fileIdx - 1].lineCnt   = lineChCnt;
    if (srcfd != rootfd)        /* close if include file */
    {
        Close(srcfd, &statusIO);
        IoErrChk();
    }

    /*
      включаем принудительное чтение первой порции исходника
    */
    endInBufP = inBuf;            /* force Read() */
    inChP = endInBufP - 1;
    startLineP = inBuf;
    files[fileIdx].offs = 0;            /* record at start of file */
    srcfd = SafeOpen(files[fileIdx].name, READ_MODE);    /* Open() the file */
}
