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

byte aExtents[] = " LSTOBJ";
static byte aDebug[] = "DEBUG";


/* skip white space in command line */
void CmdSkipWhite()
{
    while ((*cmdchP == ' ' || *cmdchP == TAB) && cmdchP != actRead.bp) {
        cmdchP++;
    }
}

/*
    return drive ('0'-'9') of current program.
    skips DEBUG if present
    returns '0' if no drive specified
*/
byte GetDrive()
{
    if (*cmdchP == ':') {
        cmdchP += 2;
        return *cmdchP;
    } else
        for (ii = 0; ii <= 4; ii++) {        /* case insensitive compare to DEBUG */
        if (*cmdchP != aDebug[ii] && aDebug[ii] + 0x20 != *cmdchP)
                return '0';    /* must be a file name so drive 0 */
            cmdchP++;
        }
    CmdSkipWhite();
    if (*cmdchP != ':')
        return '0';
    cmdchP += 2;
    return *cmdchP;
}

void AddExtents()
{
    for (ii = 1; ii <= 3; ii++) {
        lstFile[kk + ii] = aExtents[ii];
        objFile[kk + ii] = aExtents[ii+3];
    }
}


/* inits usage include overlay file initiatisation */
bool IsWhiteOrCr(byte c)
{
    return c == ' ' || c == TAB || c == CR;
}


/*
  convert ISIS file name to asciiz string file name
*/
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

void GetAsmFile()
{
    char fname[256];

    /* select key words depending on whether macro version or not */
    symTab[TID_KEYWORD] = (tokensym_t *) extKeywords;    /* extended key words */
    /* set location of symbol table */
    endSymTab[TID_KEYWORD] = symTab[TID_SYMBOL] = endSymTab[TID_SYMBOL] = (tokensym_t *)(symHighMark = MEMORY);
    Rescan(1, &statusIO);    /* get the command line */
    IoErrChk();
    Read(1, cmdLineBuf, 128, &actRead.w, &statusIO);
    IoErrChk();
    actRead.bp = actRead.w + cmdLineBuf;    /* convert to pointer */
    scanCmdLine = true;        /* scanning command line */

    Write(0, signonMsg, 0x29, &statusIO);
    Write(0, signonMsg, 2, &statusIO);
    IoErrChk();

    CmdSkipWhite();
    asxref[2] = GetDrive();     /* asxref tmp file defaults to current drive */

    while (! IsWhiteOrCr(*cmdchP)) {    /* skip to end of program name */
        cmdchP++;
    }

    CmdSkipWhite();
    if (*cmdchP == CR)        /* no args !! */
        RuntimeError(RTE_FILE);

    // �뢮��� ��� ��室���� 䠩��
    isis2dos(cmdchP, fname);
    Write(0, fname, strlen(fname), &statusIO);

    /* Open() file for reading */
    infd = SafeOpen(cmdchP, READ_MODE);
    rootfd = srcfd = infd;
    ii = true;      /* copying name */

    kk = 0;     /* length of file name */
    while (! IsWhiteOrCr(*cmdchP)) {    /* copy file name over to the files list */
        files[0].name[kk] = *cmdchP;
        if (ii)        /* and the name for the lst && obj files */
            lstFile[kk] = objFile[kk] = *cmdchP;
        if (*cmdchP == '.')
        {
            AddExtents();    /* add lst and obj file extents */
            // PMO ii = false moved to after AddExtents as AddExtents leaves ii = 4 which is false in plm
            ii = false; /* don't copy extent for lst & obj files */
        }
        kk = kk + 1;
        cmdchP = cmdchP + 1;
    }
    controlsP = cmdchP;        /* controls start after file name */
    if (ii)            /* no extent in source file */
    {
        lstFile[kk] = '.';    /* add the . and the extents */
        objFile[kk] = '.';
        AddExtents();
    }

    files[0].name[kk] = ' ';    /* append trailing space */
    /* set drive for macro and asxref tmp files if specified in source file */
    if (lstFile[0] == ':' && lstFile[2] != '0')
    {
        asmacRef[2] = lstFile[2];
        asxrefTmp[2] = lstFile[2];
    }
}


void ResetData()
{    /* extended initialisation */

    InitLine();

    b6B33 = scanCmdLine = skipIf[0] = b6B2C = inElse[0] = finished =
        segDeclared[0] = segDeclared[1] = inComment = expandingMacro = expandingMacroParameter = mSpoolMode =
        hasVarRef = pendingInclude = bZERO;
    noOpsYet = primaryValid = controls.list = ctlListChanged = bTRUE;
    controls.gen = bTRUE;
    controls.cond = bTRUE;
    macroDepth = macroSpoolNestDepth = macroCondStk[0] = macroCondSP = bZERO;
    saveIdx = lookAhead = activeSeg = ifDepth = opSP = opStack[0] = bZERO;
    macroBlkCnt = wZERO;
    segLocation[SEG_ABS] = segLocation[SEG_CODE] = segLocation[SEG_DATA] =
        maxSegSize[SEG_ABS] = maxSegSize[SEG_CODE] = maxSegSize[SEG_DATA] =
        effectiveAddr.w = localIdCnt = externId = errCnt = wZERO;
    passCnt++;
//#pragma warning(disable:4244)
    srcLineCnt = curOp = pageCnt = pageLineCnt = 1;
//#pragma warning(default: 4224)
    b68AE = false;
    curChar = ' ';
    for (ii = 0; ii <= 11; ii++)          /* reset all the control seen flags */
        controlSeen[ii] = false;

    curMacroBlk = 0xFFFF;
    if (! IsPhase1()) {   /* close any Open() include file */
        if (fileIdx != 0) {
            CloseF(srcfd);
            IoErrChk();
            srcfd = rootfd;
        }

        fileIdx = bZERO;    /* reset files for another pass */
        endInBufP = inBuf;
        inChP = endInBufP - 1;
        startLineP = inBuf;
        Seek(infd, SEEKABS, &azero, &statusIO);    /* rewind */
        IoErrChk();
    }

    baseMacroTbl = Physmem() + 0xBF;
    endOutBuf = &outbuf[OUT_BUF_SIZE];
}

void InitRecTypes()
{
    rContent.type = OMF_CONTENT;
    rContent.len = 3;
    rPublics.type = OMF_RELOC;
    rPublics.len = 1;
    rInterseg.type = OMF_INTERSEG;
    rInterseg.len = 2;
    rExtref.type = OMF_EXTREF;
    rExtref.len = 1;
}

