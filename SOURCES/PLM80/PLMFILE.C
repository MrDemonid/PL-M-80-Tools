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

void CloseF(file_t *fileP)
{
    word status;

    Close(fileP->aftn, &status);
    if (status != 0 )
        FatlIO(fileP, status);
}

/* CreatF(fileP, bufP, sizeW, modeB)
   set files buffer and buffer Size
   Read() into buffer if modeB =  1
*/

void CreatF(file_t *fp, pointer buf, word bsize, byte mode)
{
    fp->bufP = buf;
    fp->bsize = bsize;
    if (mode == 1 )
        ReadF(fp, buf, bsize, &fp->actual);
    fp->curoff = 0;
}

void DeletF(file_t *fileP)
{
    word status;

    Delete(fileP->fNam, &status);
}

void Fflush(file_t *fp)
{
    if (fp->curoff != 0 )
    {
        WriteF(fp, fp->bufP, fp->curoff);
        fp->curoff = 0;
    }
}

/*
  ���१�஢����� �⥭�� �� 䠩��
*/
void Fread(file_t *fp, pointer buf, word cnt) {

//      if (fread(buf, 1, cnt, fp->aftn) != cnt)
//              fatalIO(fp, IOERR_254);

    word inbuf = fp->actual - fp->curoff;

    for (;;) {
        word avail = cnt > inbuf ? inbuf : cnt;
        if (fp->actual == 0)
            FatlIO(fp, IOERR_254);              // ATTEMPT TO READ PAST EOF
        memmove(buf, &fp->bufP[fp->curoff], avail);
        fp->curoff += avail;
        if((cnt -= avail) == 0)
            break;
        buf += avail;
        if (fp->actual <= fp->curoff) {
            ReadF(fp, fp->bufP, fp->bsize, &fp->actual);
            fp->curoff = 0;
        }
        inbuf = fp->actual;
    }
}

/*
  ���१�஢����� ������ � 䠩�
*/
void Fwrite(file_t * fp, pointer buf, word len)
{
//      if (fwrite(buf, 1, len, fp->aftn) != len)
//              fatalIO(fp, errno);

    word wcnt;
    word spaceInBuf = fp->bsize - fp->curoff;
#ifdef _DEBUG
    if (fp == &tx1File) {
        for (int i = 0; i < len; i++) {
            if (buf[i] == 178)
                break;
        }
    }
#endif
    for (;;) {
        wcnt = spaceInBuf >= len ? len : spaceInBuf;
        memmove(&fp->bufP[fp->curoff], buf, wcnt);
        fp->curoff += wcnt;
        len -= wcnt;
        if (len == 0)
            break;
        buf += wcnt;
        if (fp->curoff >= fp->bsize)
            Fflush(fp);
        spaceInBuf = fp->bsize;
    }
}

void InitF(file_t *fileP, pointer sNam, pointer fNam)
{
    fileP->aftn = 0;
    memset(fileP->sNam, ' ', 6);
    memset(fileP->fNam, ' ', 18);
    CpyTill(sNam, fileP->sNam, 6, ' ');
    CpyTill(fNam, fileP->fNam, 16, ' '); /* ������ ���� 16, � �� 17, � �ਣ����� �뫠 �訡�� */
    fileP->bufP = NULL;
    fileP->bsize = 0;
    fileP->actual = 0;
    fileP->curoff = 0;
} /* InitF() */

void OpenF(file_t *fileP, byte access)
{
    word status;

    Open(&fileP->aftn, fileP->fNam, access, 0, &status);
    if (status != 0 )
        FatlIO(fileP, status);
} /* OpenF() */

void ReadF(file_t *fileP, pointer bufP, word len, wpointer actualP)
{
    word status;

    Read(fileP->aftn, bufP, len, actualP, &status);
    if (status != 0 )
        FatlIO(fileP, status);
}


void Rewind(file_t *fileP)
{
    loc_t loc;

    loc.blk = loc.byt = 0;
    SeekF(fileP, &loc);
}


void SeekF(file_t *fileP, loc_t *locP)
{
    word status;

    Seek(fileP->aftn, 2, &locP->blk, &locP->byt, &status);
    if (status != 0 )
        FatlIO(fileP, status);
}

void SekEnd(file_t *fileP)
{
    word status, tmp;

    Seek(fileP->aftn, 4, &tmp, &tmp, &status);
    if (status != 0)
        FatlIO(fileP, status);
}

void TellF(file_t *fileP, loc_t *locP)
{
    word status;

    Seek(fileP->aftn, 0, &locP->blk, &locP->byt, &status);
    if (status != 0 )
        FatlIO(fileP, status);
}

void WriteF(file_t *fp, pointer buf, word count)
{
 word status;

 Write(fp->aftn, buf, count, &status);
 if (status != 0)
  FatlIO(fp, status);
}

