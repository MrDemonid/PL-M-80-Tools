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

void SyntaxError()
{
    SourceError('Q');
}

void DoubleDefError()
{
    SourceError('D');
}

void ExpressionError()
{
    SourceError('E');
}

void CommandError()
{
    SourceError('C');
}

void OpcodeOperandError()
{
    SourceError('O');
}

void NameError()
{
    SourceError('R');
}

void MultipleDefError()
{
    SourceError('M');
}

void ValueError()
{
    SourceError('V');
}

void NestingError()
{
    SourceError('N');
}

void PhaseError()
{
    SourceError('P');
}

void StackError()
{
    RuntimeError(RTE_STACK);
}

void FileError()
{
    RuntimeError(RTE_FILE);
}

void IllegalCharError()
{
    SourceError('I');
}

void BalanceError()
{
    SourceError('B');
}

void UndefinedSymbolError()
{
    SourceError('U');
}

void LocationError()
{
    SourceError('L');
}

void OperandError()
{
    SourceError('X');
}

byte HaveTokens()
{
    return tokenIdx != 0;
}


void PopToken()
{
#if _DEBUG
    DumpTokenStack(true);
#endif
    tokStart[0] = tokStart[tokenIdx];
    tokenSym.stack[0] = tokenSym.stack[tokenIdx];
    tokenType[0] = tokenType[tokenIdx];
    tokenAttr[0] = tokenAttr[tokenIdx];
    tokenSize[0] = tokenSize[tokenIdx];
    tokenSymId[0] = tokenSymId[tokenIdx];
    if (HaveTokens())
        tokenIdx--;
}



/* nest - sw == 1 -> nest macro sw == 2 -> nest if */
void Nest(byte sw)
{
    macroCondStk[++macroCondSP] = macroCondStk[0];
    /* record whether current nest is macro of if */
    if ((macroCondStk[0] = sw) == 1) {
        if (++macroDepth > 9) {
            StackError();
            macroDepth = 0;
        } else {
            macro.stack[macroDepth] = macro.stack[0];
            //memcpy(&macro.stack[macroDepth], &macro.stack[0], sizeof(macro_t));
            macro.top.condSP = macroCondSP;
            macro.top.ifDepth = ifDepth;
            nestMacro = true;
        }
    } else {
        if (++ifDepth > 8) {
            StackError();
            ifDepth = 0;
        } else {
            skipIf[ifDepth] = skipIf[0];
            inElse[ifDepth] = inElse[0];
        }
    }
}


void UnNest(byte sw)
{
    if (sw != macroCondStk[0]) {   /* check for unbalanced unnest */
        NestingError();
        if (sw == 2)          /* ! macro unnest */
            return;
        macroCondSP = macro.top.condSP;
        ifDepth = macro.top.ifDepth;
    }

    macroCondStk[0] = macroCondStk[macroCondSP];    /* restore macro stack */
    macroCondSP--;
    if (sw == 1) {         /* is unnest macro */
        macro.stack[0] = macro.stack[macroDepth];
        memcpy(&macro.stack[0], &macro.stack[macroDepth], sizeof(macro_t));
        ReadM(macro.top.blk);
        savedMtype = macro.top.mtype;
        if (--macroDepth == 0) { /* end of macro nest */
            expandingMacro = 0;     /* not expanding */
            baseMacroTbl = Physmem() + 0xBF;
        }
    } else {
        skipIf[0] = skipIf[ifDepth];    /* pop skipIf and inElse status */
        inElse[0] = inElse[ifDepth];
        ifDepth--;
    }
}

void PushToken(byte type)
{
    if (tokenIdx >= MAX_TOKENS_IDX)
        StackError();
    else {
        tokenIdx = tokenIdx + 1;
        tokStart[tokenIdx] = tokStart[0];
        tokenSym.stack[tokenIdx] = tokenSym.stack[0];
        tokenType[tokenIdx] = tokenType[0];
        tokenAttr[tokenIdx] = tokenAttr[0];
        tokenSize[tokenIdx] = tokenSize[0];
        tokenSymId[tokenIdx] = tokenSymId[0];
        tokStart[0] += tokenSize[0];    /* advance for next token */
        tokenType[0] = type;
        tokenAttr[0] = tokenSize[0] = bZERO;
        tokenSym.stack[0] = NULL; // (tokensym_t *)wZERO; to work around gcc complaint
        tokenSymId[0] = wZERO;
    }
#if _DEBUG
    DumpTokenStack(false);
#endif
}

void CollectByte(byte c)
{
    byte *s;

    if ((s = tokStart[0] + tokenSize[0]) < endLineBuf) {   /* check for lineBuf overrun */
        *s = c;
        tokenSize[0]++;
    }
    else
        StackError();
}

void GetId(byte type)
{
    PushToken(type);    /* save any previous token and initialise this one */
    reget = 1;        /* force re get of first character */

    while ((type = GetChClass()) == CC_DIG || type == CC_LET) {    /* digit || letter */
        if (curChar >= 'a' && curChar <= 'z')    /* make sure upper case */
            curChar = curChar & 0xDF;
        CollectByte(curChar);
    }
    reget = 1;        /* force re get of Exit() char */
}


void GetNum()
{
    word accum;
    byte radix, digit, i;
//    byte chrs based tokStart[0] [1];

    GetId(O_NUMBER);
    radix = tokStart[0][--tokenSize[0]];
    if (radix == 'H')
        radix = 16;

    if (radix == 'D')
        radix = 10;

    if (radix == 'O' || radix == 'Q')
        radix = 8;

    if (radix == 'B')
        radix = 2;

    if (radix > 16)
        radix = 10;
    else
        tokenSize[0]--;

    accum = 0;
    for (i = 0; i <= tokenSize[0]; i++) {
        if (tokStart[0][i] == '?' || tokStart[0][i] == '@') {
            IllegalCharError();
            digit = 0;
        } else {
            if ((digit = tokStart[0][i] - '0') > 9)
                digit = digit - 7;
            if (digit >= radix)
                if (! (tokenType[2] == O_NULVAL)) { /* bug? risk that may be random - tokenIdx may be < 2 */
                    IllegalCharError();
                    digit = 0;
                }
        }

        accum = accum * radix + digit;
    }
    /* replace with packed number */
    tokenSize[0] = 0;
    CollectByte((accum) & 0xff);
    CollectByte((accum) >> 8);
}

void GetStr()
{
    PushToken(O_STRING);

    while (GetCh() != CR) {
        if (curChar == '\'')
            if (GetCh() != '\'')        // if not '' then all done
                goto L6268;
        CollectByte(curChar);   // collect char - '' becomes '
    }

    BalanceError();                             // EOL seen before closing '

L6268:
    reget = 1;                                  // push back CR or char after '
}
