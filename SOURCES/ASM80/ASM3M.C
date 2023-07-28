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


// vim:ts=4:expandtab:shiftwidth=4:
#include "asm80.h"

            /* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
bool isExprOrMacroMap[] = {
           true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true,
           true, true, true, true, true, true, true, true, true, true, false,false,false,false,false,false,
           false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,
           false,false,false,false,false,false,true, true, true, true, true, true, true, true, false,true,
           true, true};
bool isInstrMap[] = {
           false,true, true, true, false,false,false,false,false,false,false,false,false,false,false,false,
           false,false,false,false,false,false,false,false,false,false,true, true, false,false,false,false,
           false,false,false,false,true, true, true, true, true, true, true, true, true, true, true, false,
           false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,
           true, true};

void ChkSegAlignment(byte seg)
{    /* seg = 0 => CSEG, seg = 1 => DSEG */

    if (segDeclared[seg]) {
        if (HaveTokens()) {
            accum1 = GetNumVal();                   // get align type
            if (alignTypes[seg] != accum1Lb)        // alignt type mis-match
                ExpressionError();
        }
        else if (alignTypes[seg] != 3)    /* no speficier - check byte algined */
            ExpressionError();
    } else {
        segDeclared[seg] = true;
        if (HaveTokens()) {
            accum1 = GetNumVal();               // get align type
            if (accum1 == 1 || accum1 == 2)    /* only allow inPage and Page */
                alignTypes[seg] = accum1 & 0xff;    // set algin type
            else
                ExpressionError();
        }
    }
}


void Cond2Acc(byte cond)
{    /* convert conditional result to accum1 */

    accum1 = cond;
    if (accum1)
        accum1 = 0xFFFF;
}

void UpdateHiLo(byte hilo)
{

    if ((acc1RelocFlags & (UF_EXTRN + UF_SEGMASK)) != 0)
        acc1RelocFlags = (acc1RelocFlags & ~UF_RBOTH) | hilo;
}

void HandleOp()
{
    switch (topOp)
    {
        case 0:
            break;

        case Y_CR:                              /* CR */
            FinishLine();
            break;

        case Y_LPAREN:                          /* ( */
        case Y_RPAREN:                          /* ) */
            if (! (topOp == T_LPAREN && curOp == T_RPAREN))
            {
                /*
                  incorrectly nested ()
                */
                BalanceError();
            }
            if (tokenType[0] == O_DATA)
            {
                tokenSize[0] = 1;
                tokenAttr[0] = 0;
                b6B36 = true;
            }

            expectOp = inNestedParen;
            if (curOp == T_RPAREN)
                b6B2C = true;
            break;

        case Y_STAR:                            /* * */
            accum1 *= accum2;
            break;

        case Y_PLUS:                            /* + */
            accum1 += accum2;
            break;

        case Y_COMMA:                           /* , */
            SyntaxError();
            PopToken();
            break;

        case Y_MINUS:                           /* - */
            accum1 -= accum2;
            break;

        case Y_UPLUS:                           /* unary + */
            break;

        case Y_SLASH:                           /* / */
            if (accum2 == 0)
            {
                /*
                  synthesise what 8085 does on / 0
                */
                ValueError();
                accum1 = 0xffff;
            }
            else
                accum1 /= accum2;
            break;

        case Y_UMINUS:                          /* unary - */
            accum1 = -accum1;
            break;

        case Y_EQ:                              /* EQ */
            Cond2Acc(accum1 == accum2);
            break;

        case Y_LT:                              /* LT */
            Cond2Acc(accum1 < accum2);
            break;

        case Y_LE:                              /* LE */
            Cond2Acc(accum1 <= accum2);
            break;

        case Y_GT:                              /* GT */
            Cond2Acc(accum1 > accum2);
            break;

        case Y_GE:                              /* GE */
            Cond2Acc(accum1 >= accum2);
            break;

        case Y_NE:                              /* NE */
            Cond2Acc(accum1 != accum2);
            break;

        case Y_NOT:                             /* NOT */
            accum1 = ~accum1;
            break;

        case Y_AND:                             /* AND */
            accum1 &= accum2;
            break;

        case Y_OR:                              /* OR */
            accum1 |= accum2;
            break;

        case Y_XOR:                             /* XOR */
            accum1 ^= accum2;
            break;

        case Y_MOD:                             /* MOD */
            if (accum2 == 0)
            {
                ValueError();
                /*
                  this is what the 8080 mod code would do
                */
                accum1 = accum2;
            } else
                accum1 %= accum2;
            break;

        case Y_SHL:                             /* SHL */
            if (accum2Lb != 0)
                accum1 <<= accum2;
            break;

        case Y_SHR:                             /* SHR */
            if (accum2Lb != 0)
                accum1 >>= accum2;
            break;

        case Y_HIGH:                            /* HIGH */
            accum1 >>= 8;
            UpdateHiLo(UF_RHIGH);
            break;

        case Y_LOW:                             /* LOW */
            accum1 &= 0xff;
            UpdateHiLo(UF_RLOW);
            break;

        case Y_DB:                              /* DB */
            if (tokenType[0] != O_STRING)
            {
                accum1 = GetNumVal();
                if ((byte)(accum1Hb - 1) < 0xFE)   // ! 0 or FF
                    ValueError();
                /*
                  adjusted from PLM as revised code doesn't use rotate
                */
                curOpFlags = 0x44;
                if ((acc1RelocFlags & UF_RBOTH) == UF_RBOTH)
                {
                    /*
                      can't db a 16 bit relocatable
                    */
                    ValueError();
                    acc1RelocFlags = (acc1RelocFlags & ~UF_RBOTH) | UF_RLOW;    // treat as 8 bit relocatable
                }
            } else {
                /*
                  abs bytes
                */
                acc1RelocFlags = 0;
                tokenType[0] = O_DATA;
            }
            if (IsReg(acc1ValType))
            {
                /*
                  db of register is not valid
                */
                OperandError();
            }
            /*
              loook for more data
            */
            nextTokType = O_DATA;
            inDB = true;
            break;

        case Y_DW:                              /* DW */
            /*
              look for more data
            */
            nextTokType = O_DATA;
            inDW = true;
            break;

        case Y_DS:                              /* DS */
            /*
              bump current $ value
            */
            segLocation[activeSeg] += accum1;
            /*
              flag that new address should be shown
            */
            showAddr = true;
            break;

        case Y_EQU:                             /* EQU */
        case Y_SET:                             /* SET */
            showAddr = true;
            if ((acc1RelocFlags & UF_EXTRN) == UF_EXTRN)
            {
                /* cannot SET or EQU to external */
                ExpressionError();
                acc1RelocFlags = 0;
            }
            labelUse = L_SETEQU;
            /*
              4 for set, 5 for equ
            */
            UpdateSymbolEntry(accum1, (K_SET + O_SET) - topOp);
            expectingOperands = false;
            break;

        case Y_ORG:                             /* ORG */
            showAddr = true;
            /*
              check not ORG to extern
            */
            if ((acc1RelocFlags & UF_EXTRN) != UF_EXTRN)
            {
                /*
                  only org to abs or current seg 16 bit reloc
                */
                if ((acc1RelocFlags & UF_RBOTH) != 0)
                    if ((acc1RelocFlags & UF_SEGMASK) != activeSeg
                      || (acc1RelocFlags & UF_RBOTH) != UF_RBOTH)
                        ExpressionError();
            } else
                ExpressionError();

            /*
              if object file update maxSegSize for this segment
            */
            if (controls.object)
                if (segLocation[activeSeg] > maxSegSize[activeSeg])
                    maxSegSize[activeSeg] = segLocation[activeSeg];
            /*
              set new segLocation
            */
            segLocation[activeSeg] = accum1;
            break;

        case Y_END:                             /* END */
            if (tokenIdx > 0)
            {
                startOffset = GetNumVal();
                startDefined = 1;
                startSeg = acc1RelocFlags & 7;
                if ((acc1RelocFlags & UF_EXTRN) == UF_EXTRN)
                    ExpressionError();
                if (IsReg(acc1ValType))
                    OperandError();

                showAddr = true;
            }
            kk = mSpoolMode;
            mSpoolMode = 0;

            if (macroCondSP > 0 || (kk & 1))
                NestingError();
            if (curOp != T_CR)
                SyntaxError();
            if (expectOp)
                b6B33 = true;
            else
                SyntaxError();
            break;

        case Y_IF:                              /* IF */
            if (expectOp)
            {
                condAsmSeen = true;
                /*
                  push current skip/else status
                */
                Nest(2);
                /*
                  force any pending xref info to be written
                */
                xRefPending = true;
                if (!skipIf[0])
                {
                    /*
                      set new status
                    */
                    skipIf[0] = ! (accum1 & 1);
                }
                /*
                  ! in else at this nesting level
                */
                inElse[0] = false;
            }
            break;

        case Y_ELSE:                            /* ELSE */
            condAsmSeen = true;
            /*
              check ! mid macro nest
            */
            if (macroCondStk[0] != 2)
                NestingError();
            else if (! inElse[0])       // shouldn't be in else at this level
            {
                if (! skipIf[0])        // IF was active so ELSE forces skip
                    skipIf[0] = true;
                else                    // IF inactive so revert to previous skipping status
                    skipIf[0] = skipIf[ifDepth];
                inElse[0] = true;       // in else at this nesting level
            }
            else
                NestingError();         // multiple else !!
            break;

        case Y_ENDIF:                           /* ENDIF */
            if (expectOp)
            {
                /*
                  revert to previous status
                */
                condAsmSeen = true;
                UnNest(2);
            }
            break;

        case Y_LXI:                             /* LXI */
            /*
              in the following topOp = K_LXI and nextTokType = O_DATA
              except where noted on return from MkCode
            */
            if (nameLen == 1)
                if (tokName[0] == 'M')
                    SyntaxError();
            /*
              topOp = K_INRDCR, nextTokType unchanged on return
            */
            MkCode(0x85);
            break;

        case Y_REG16:                           /* REG16 ops:               */
            if (nameLen == 1)                   /*   POP DAD PUSH INX DCX   */
                if (tokName[0] == 'M')
                    SyntaxError();
            MkCode(5);
            break;

        case Y_LDSTAX:                          /* LDAX STAX */
            MkCode(7);
            break;

        case Y_ARITH:                           /* ARITH ops:               */
            MkCode(2);                          /*   ADC ADD SUB ORA SBB    */
            break;                              /*   XRA ANA CMP            */

        case Y_IMM8:                            /* IMM8 ops:                */
            MkCode(8);                          /*   ADI OUT SBI ORI IN CPI */
            break;                              /*   SUI XRI ANI ACI        */

        case Y_MVI:                             /* MVI, topOp = K_IMM8 on return */
            MkCode(0x46);
            break;

        case Y_INRDCR:                          /* INR DCR */
            MkCode(6);
            break;

        case Y_MOV:                             /* MOV   topOp = K_ARITH,   */
            MkCode(0x36);                       /* nextTokType unchanged on return */
            break;

        case Y_IMM16:                           /* IMM16 ops:               */
            MkCode(0);                          /*   CZ CNZ JZ STA JNZ JNC  */
                                                /*   LHLD CP JC SHLD CPE    */
                                                /*   CPO CM LDA JP JM JPE   */
            break;                              /*   CALL JPO CC CNC JMP    */

        case Y_SINGLE:                          /* SINGLE ops:              */
            MkCode(0);                          /*   RNZ STC DAA DI SIM     */
                                                /*   SPHL RLC RP RAL HLT RM */
                                                /*   RAR RPE RET RIM PCHL   */
                                                /*   CMA CNC RPO EI XTHL    */
            break;                              /*   NOP RC RNX XCHG RZ RRC */

        case Y_RST:                             /* RST */
            MkCode(6);
            break;

        case Y_ASEG:                            /* ASEG */
            activeSeg = 0;
            break;

        case Y_CSEG:                            /* CSEG */
            activeSeg = 1;
            ChkSegAlignment(0);
            break;

        case Y_DSEG:                            /* DSEG */
            activeSeg = 2;
            ChkSegAlignment(1);
            break;

        case Y_PUBLIC:                          /* PUBLIC */
            inPublic = true;
            labelUse = L_REF;
            UpdateSymbolEntry(0, O_REF);
            break;

        case Y_EXTRN:                           /* EXTRN */
            inExtrn = true;
            if (externId == 0 && IsPhase1() && controls.object)
                WriteModhdr();
            labelUse = L_REF;
            UpdateSymbolEntry(externId, O_TARGET);
            if (IsPhase1() && controls.object && ! badExtrn)
                WriteExtName();
            if (! badExtrn)
                externId++;
            badExtrn = false;
            break;

        case Y_NAME:                            /* NAME */
            if (tokenIdx != 0 && noOpsYet)
            {
                /*
                  set the module name in the header - padded to 6 chars
                */
                move(6, spaces6, aModulePage);
                move(moduleNameLen = nameLen, tokName, aModulePage);
            } else
                SourceError('R');
            /*
              consume the token
            */
            PopToken();
            break;

        case Y_STKLN:                           /* STKLN */
            segLocation[SEG_STACK] = accum1;
            break;

        case Y_MACRO:                           /* MACRO */
            DoMacro();
            break;

        case Y_MACROPARAM:                      /* MACRO BODY */
            DoMacroBody();
            break;

        case Y_ENDM:                            /* ENDM */
            DoEndm();
            break;

        case Y_EXITM:                           /* EXITM */
            DoExitm();
            break;

        case Y_MACRONAME:                       /* MACRONAME */
            macro.top.mtype = M_INVOKE;
            initMacroParam();
            break;

        case Y_IRP:                             /* IRP */
            DoIrpX(M_IRP);
            break;

        case Y_IRPC:                            /* IRPC */
            DoIrpX(M_IRPC);
            break;

        case Y_ITERPARAM:                       /* MACRO PARAMETER */
            DoIterParam();
            break;

        case Y_REPT:                            /* REPT */
            DoRept();
            break;

        case Y_LOCAL:                           /* LOCAL */
            DoLocal();
            break;

        case Y_NULVAL:                          /* optVal */
            Sub78CE();
            break;

        case Y_NUL:                             /* NUL */
            Cond2Acc(tokenType[0] == K_NUL);
            PopToken();
            acc1RelocFlags = 0;
    }

    if (topOp != T_CR)
        noOpsYet = false;
}


static byte IsExpressionOp()    // returns true if op is valid in an expression
{
    if (yyType > T_RPAREN)
        if (yyType != T_COMMA)
            if (yyType < K_DB)
                return true;
    return false;
}

static byte IsVar(byte type)
{
    return type == O_NAME || type == O_SETEQUNAME;
}


static void UpdateIsInstr()
{
    if (! isInstrMap[topOp])
        isInstr = false;
}


void Parse()
{
    while (1) {
        /* nothing to parse if
            skipping if and not cr or if related keyword
            or spooling and not macro related keyword or in quotes
        */
#ifdef _DEBUG
        printf("Parse: ");
        ShowYYType();
#endif
        if (((! (yyType == T_CR ||( K_END <= yyType && yyType <= K_ENDIF))) && skipIf[0])
            || ((opFlags[yyType] < 128 || inQuotes) && (mSpoolMode & 1))) { // if spooling skip non macro and in quotes
            needsAbsValue = false;
            PopToken();
            return;
        }

        if (phase != 1)
            if (inExpression)
                if (IsExpressionOp())
                    if (GetPrec(yyType) <= GetPrec(opStack[opSP]))
                        ExpressionError();

        if (GetPrec(curOp = yyType) > GetPrec(topOp = opStack[opSP]) || curOp == T_LPAREN) {    /* SHIFT */
            if (opSP >= 16) {
                opSP = 0;
                StackError();
            } else
                opStack[++opSP] = curOp;
            if (curOp == T_LPAREN) {
                inNestedParen = expectOp;
                expectOp = true;
            }
#if _DEBUG
            printf("\n>>>> Shift\n");
            DumpOpStack();
            DumpTokenStack(false);
            printf("\n");
#endif
            if (phase > 1)
                inExpression = IsExpressionOp();
            return;
        }

    /* REDUCE */
#if _DEBUG
        printf("\n<<<< Start - reduce\n");
        DumpOpStack();
        DumpTokenStack(false);
#endif
        inExpression = false;
        if ((! expectOp) && topOp > T_RPAREN)
            SyntaxError();

        if (topOp == T_VALUE) /* topOp used so set to curOp */
            topOp = curOp;
        else
            opSP--;    /* pop Op */


        if ((curOpFlags = opFlags[topOp]) & 1) {     /* load rhs operand into acc1 and acc2) */
            accum2 = GetNumVal();
            acc2RelocFlags = acc1RelocFlags;
            acc2RelocVal = acc1RelocVal;
            acc2ValType = acc1ValType;
        }

        if (curOpFlags & 2) /* removed ror */ /* has a lhs operand so load into acc1 */
            accum1 = GetNumVal();

        if (! hasVarRef)
            hasVarRef = IsVar(acc1ValType) || IsVar(acc2ValType);

        nextTokType = O_NUMBER;            // assume next token is a number, HandleOp may change
        if (topOp > T_RPAREN && topOp < K_DB)    /* expression topOp */
            ResultType();                       // check and set result type
        else {
            UpdateIsInstr();
            ChkInvalidRegOperand();
        }

        HandleOp();
        if (! isExprOrMacroMap[topOp])
            expectOp = false;

        if (b6B2C) {
            b6B2C = false;
            return;
        }

        if (topOp != K_DS && showAddr)
            effectiveAddr.w = accum1;

        if ((curOpFlags & 0x3C))       /* -xxxxx-- -> collect list or bytes */
            PushToken(nextTokType);

        for (ii = 0; ii <= 3; ii++)
            if (curOpFlags & (4 << ii))  /* --xxxx-- -> collect high/low acc1/acc2 */
                CollectByte(((byte *)&accum1)[ii]);

        tokenAttr[0] = acc1RelocFlags;
        tokenSymId[0] = acc1RelocVal;
        if (curOpFlags & 0x40)          /* -x------ -> list */
            if (curOp == T_COMMA) {     // if comma then make the operator (topOp) as
                yyType = topOp;         // the next item to read and mark as operator
                expectOp = true;
            }
#if _DEBUG
        printf("\n<<<< End - reduce\n");
        DumpOpStack();
        DumpTokenStack(false);
        printf("\n");
#endif
    }
}



void DoPass()
{
    while (!finished) {
        Tokenise();
//              DumpOpStack();
//              DumpTokenStack();
        Parse();
    }
}

