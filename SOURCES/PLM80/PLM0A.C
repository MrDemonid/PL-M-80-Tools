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


byte cClass[] = {
    CC_NONPRINT, CC_NONPRINT, CC_NONPRINT, CC_NONPRINT,
    CC_NONPRINT, CC_NONPRINT, CC_NONPRINT, CC_NONPRINT,
    CC_NONPRINT, CC_WSPACE, CC_NEWLINE, CC_NONPRINT,
    CC_NONPRINT, CC_WSPACE, CC_NONPRINT, CC_NONPRINT,
    CC_NONPRINT, CC_NONPRINT, CC_NONPRINT, CC_NONPRINT,
    CC_NONPRINT, CC_NONPRINT, CC_NONPRINT, CC_NONPRINT,
    CC_NONPRINT, CC_NONPRINT, CC_NONPRINT, CC_NONPRINT,
    CC_NONPRINT, CC_NONPRINT, CC_NONPRINT, CC_NONPRINT,
    CC_WSPACE, CC_INVALID, CC_INVALID, CC_INVALID,
    CC_DOLLAR, CC_INVALID, CC_INVALID, CC_QUOTE,
    CC_LPAREN,  CC_RPAREN,  CC_STAR,  CC_PLUS,
    CC_COMMA,  CC_MINUS, CC_PERIOD,  CC_SLASH,
    CC_BINDIGIT, CC_BINDIGIT, CC_OCTDIGIT, CC_OCTDIGIT,
    CC_OCTDIGIT, CC_OCTDIGIT, CC_OCTDIGIT, CC_OCTDIGIT,
    CC_DECDIGIT, CC_DECDIGIT, CC_COLON, CC_SEMICOLON,
    CC_LESS, CC_EQUALS, CC_GREATER, CC_INVALID,
    CC_INVALID, CC_HEXCHAR, CC_HEXCHAR, CC_HEXCHAR,
    CC_HEXCHAR, CC_HEXCHAR, CC_HEXCHAR, CC_ALPHA,
    CC_ALPHA, CC_ALPHA, CC_ALPHA, CC_ALPHA,
    CC_ALPHA, CC_ALPHA, CC_ALPHA, CC_ALPHA,
    CC_ALPHA, CC_ALPHA, CC_ALPHA, CC_ALPHA,
    CC_ALPHA, CC_ALPHA, CC_ALPHA, CC_ALPHA,
    CC_ALPHA, CC_ALPHA, CC_ALPHA, CC_INVALID,
    CC_INVALID, CC_INVALID, CC_INVALID, /* _ */ CC_ALPHA,
    CC_INVALID, CC_HEXCHAR, CC_HEXCHAR, CC_HEXCHAR,
    CC_HEXCHAR, CC_HEXCHAR, CC_HEXCHAR, CC_ALPHA,
    CC_ALPHA, CC_ALPHA, CC_ALPHA, CC_ALPHA,
    CC_ALPHA, CC_ALPHA, CC_ALPHA, CC_ALPHA,
    CC_ALPHA, CC_ALPHA, CC_ALPHA, CC_ALPHA,
    CC_ALPHA, CC_ALPHA, CC_ALPHA, CC_ALPHA,
    CC_ALPHA, CC_ALPHA, CC_ALPHA, CC_INVALID,
    CC_INVALID, CC_INVALID, CC_INVALID, CC_NONPRINT };

byte tok2oprMap[] = {
    L_IDENTIFIER, L_NUMBER, L_STRING, L_PLUSSIGN,
    L_MINUSSIGN, L_STAR, L_SLASH, L_MOD,
    L_PLUS, L_MINUS, L_AND, L_OR,
    L_XOR, L_NOT, 0, L_LT,
    L_LE, L_EQ, L_NE, L_GE,
    L_GT, L_COLONEQUALS, L_INVALID, L_INVALID,
    L_PERIOD, L_LPAREN, L_RPAREN, L_COMMA,
    L_CALL, L_INVALID, L_DISABLE, L_INVALID,
    L_ENABLE, L_END, L_GO, L_GOTO,
    L_HALT, L_IF, L_PROCEDURE, L_RETURN,
    L_INVALID, L_INVALID, L_INVALID, L_INVALID,
    L_INVALID, L_INVALID, L_INVALID, L_INVALID,
    L_INVALID, L_INVALID, L_INVALID, L_INVALID,
    L_INVALID, L_BY, L_INVALID, L_INVALID,
    L_INVALID, L_INVALID, L_TO, L_INVALID,
    L_BREAK, L_CONTINUE };


/* public variables */
pointer macroPtrs[12]; /* six inChrP, infoP pairs stored as pointer not offset_t */
word macroDepth = 0;
word tokenVal;
pointer inChrP;         /* has to be pointer as it accesses data outside info/symbol space */
word stateStack[100];
word stateIdx;
offset_t stmtLabels[10];
word stmtLabelCnt;
//!!! word curProcInfoP; дубликат
word offCurCh = 0;
word offLastCh = 0;
word curStmtCnt = 0;
word curBlkCnt = 0;
offset_t curMacroInfoP = 0;
offset_t markedSymbolP = 0;
byte  lineBuf[128];
byte  inbuf[1280];
byte  tokenType;
byte  tokenStr[MAX_LIT_LENGTH+2];
word  tokenStrLen;        // реальная длина считанной строки
word  tokenMaxLen = 256;  // разрешенная длина для считываемой строки
byte  nextCh;
byte  stmtStartCode;
byte  stmtStartToken;
byte  startLexCode;
word doBlkCnt = 0;
byte  tx1Buf[1280];
offset_t stmtStartSymbol;
bool  lineInfoToWrite = false;
bool  isNonCtrlLine = false;
bool  yyAgain = false;
//!!! linfo_t linfo = { 0, 0 };
// byte  curDoBlkCnt, curProcId initial(0, 0);
byte curScope[2] = { 0, 0 };
wpointer curScopeP;
byte  state;
bool  skippingCOND = false;
word ifDepth = 0;


/*
  стек для сохранения позиций начала блоков в файле tx1
*/
unsigned long blocksStack[36];    // макс. вложенность блоков
int blocksSP = 0;


void SaveBlockPos()
{
    loc_t posBlock;
    unsigned long blk, byt;

    if (++blocksSP > 34)
    {
        FatalError(ERR27);      /* LIMIT EXCEEDED: NUMBER OF DO BLOCKS */
    }
    if (tx1File.aftn != 0)
    {
        TellF(&tx1File, &posBlock);
        blk = posBlock.blk;
        byt = posBlock.byt;
    } else {
        blk = 0;
        byt = 0;
    }
    blocksStack[blocksSP] = (blk * 128 + byt) + tx1File.curoff;
}

void RemoveBlockPos()
{
    if (blocksSP <= 0)
        FatalError(ERR205);      /* ILLEGAL NESTING OF BLOCKS, ENDS NOT BALANCED */
    blocksSP--;
}

/*
  осуществляет помену L_DO на L_REPEAT
*/
void DoToRepeat()
{
    loc_t curBlk;
    loc_t endBlk;
    unsigned long blk, byt, pos, fpos;
    char buf;

    if (blocksSP <= 0)
        FatalError(ERR205);      /* ILLEGAL NESTING OF BLOCKS, ENDS NOT BALANCED */

    if (tx1File.aftn != 0)
    {
        TellF(&tx1File, &curBlk);
        blk = curBlk.blk;
        byt = curBlk.byt;
        fpos = blk * 128 + byt;
    } else {
        fpos = 0;
    }
    pos = blocksStack[blocksSP];
    if (pos >= fpos)
    {
        /*
          код находится в буфере
        */
        pos -= fpos;
        if (tx1Buf[pos] != L_DO)
            FatalError(ERR214); /* COMPILER ERROR: OPERAND CANNOT BE TRANSFORMED */
        else
            tx1Buf[pos] = L_REPEAT;

    } else {
        /*
          код находится в файле
        */
        Fflush(&tx1File);           // сбрасываем остатки буфера в файл
        TellF(&tx1File, &endBlk);   // сохраняем текущую позицию в файле
        curBlk.blk = pos / 128;
        curBlk.byt = pos % 128;
        SeekF(&tx1File, &curBlk);
        ReadF(&tx1File, &buf, 1, &tx1File.actual);
        if (tx1File.actual)
        {
            buf = L_REPEAT;
            SeekF(&tx1File, &curBlk);
            WriteF(&tx1File, &buf, 1);
        } else
            FatalError(ERR214); /* COMPILER ERROR: OPERAND CANNOT BE TRANSFORMED */

        SeekF(&tx1File, &endBlk);
    }
}




void CreateTx1File()
{
    word tmp;

    tmp = tx1File.curoff;
    OpenF(&tx1File, 3);
    CreatF(&tx1File, tx1Buf, 1280, 2);
    tx1File.curoff = tmp;
} /* CreateTx1File() */

void WriteTx1(pointer buf, word len)
{
    if (tx1File.aftn == 0)
        if (tx1File.curoff > 1024)
            CreateTx1File();
        else {
            memmove(&tx1Buf[tx1File.curoff], buf, len);
            tx1File.curoff = tx1File.curoff + len;
            return;
        }
        Fwrite(&tx1File, buf, len);
} /* WriteTx1() */

void RewindTx1()
{
    if (tx1File.aftn == 0)
        CreateTx1File();
    Fflush(&tx1File);
    Rewind(&tx1File);
} /* RewindTx1() */


void WriteLineInfo()
{
    if (lineInfoToWrite) {
        WriteTx1((pointer)&linfo, 7);
        lineInfoToWrite = false;
    }
} /* WriteLineInfo() */


void WrBuf(pointer buf, word len)
{
    WriteLineInfo();
    WriteTx1(buf, len);
} /* WrBuf() */

void WrByte(byte v)
{
    WrBuf(&v, 1);
} /* WrByte() */



void WrWord(word v)
{
    WrBuf((pointer)&v, 2);
} /* WrWord() */


void WrInfoOffset(offset_t addr)
{
    WrWord(addr - botInfo);
} /* WrInfoOffset() */

void SyntaxError(byte err)
{
    hasErrors = true;
    WrByte(L_SYNTAXERROR);
    WrWord(err);
} /* SyntaxError() */


void TokenErrorAt(byte err)
{
    hasErrors = true;
    WrByte(L_TOKENERROR);
    WrWord(err);
    WrWord(markedSymbolP);
} /* TokenErrorAt() */


void TokenError(byte err, offset_t symP)
{
    hasErrors = true;
    WrByte(L_TOKENERROR);
    WrWord(err);
    WrWord(symP);
} /* TokenError() */

void FatalError_ov0(byte err)
{
    hasErrors = true;
    if (state != 20) {
        if (err == ERR83)       /* LIMIT EXCEEDED: DYNAMIC STORAGE */
            Fatal("DYNAMIC STORAGE OVERFLOW", 24);
        Fatal("UNKNOWN FATAL ERROR", 19);
    }
    TokenErrorAt(err);
    fatalErrorCode = err;
    longjmp(exception, -1);
}

void PushBlock(word idAndLevel)
{
    if (blockDepth == 34)
        FatalError(ERR84);      /* LIMIT EXCEEDED: BLOCK NESTING */
    else {
        procChains[++blockDepth] = idAndLevel;
        curBlkCnt++;
    }
} /* PushBlock() */

void PopBlock()
{
    if (blockDepth == 0)
        FatalError(ERR96);      /* COMPILER ERROR: SCOPE STACK UNDERFLOW */
    else {
        curBlkCnt--;
        *curScopeP = procChains[--blockDepth];
    }
} /* PopBlock() */



void WrLexToken()
{
    if (tok2oprMap[tokenType] == L_INVALID) {
        TokenErrorAt(ERR171);   /* INVALID USE OF DELIMITER OR RESERVED WORD IN Expression */
        return;
    }
    WrByte(tok2oprMap[tokenType]);
    if (tokenType == T_IDENTIFIER)
        WrWord(curSymbolP);
    else if (tokenType == T_NUMBER)
        WrWord(tokenVal);
    else if (tokenType == T_STRING) {
        WrWord(tokenStr[0]);
        WrBuf(&tokenStr[1], tokenStr[0]);
    }
} /* WrLexToken() */


void WrXrefUse()
{
    if (XREF) {
        WrByte(L_XREFUSE);
        WrInfoOffset(curInfoP);
    }
}


void WrXrefDef()
{
    if (XREF || IXREF || SYMBOLS) {
        WrByte(L_XREFDEF);
        WrInfoOffset(curInfoP);
    }
}

