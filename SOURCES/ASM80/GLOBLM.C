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

byte macroLine[129];
byte *macroP = macroLine;
bool inQuotes = false;
bool excludeCommentInExpansion;
bool inAngleBrackets;
byte expandingMacro;            // 0,1 or 0xff
bool expandingMacroParameter;
bool inMacroBody = false;
byte mSpoolMode;                // 0->normal, 1->spool (local ok), 2->capture locals, 0xfe->might not occur , 0xff->capture body
bool b9060;
bool nestMacro;
byte savedMtype;
byte macroDepth;
byte macroSpoolNestDepth;
byte paramCnt;
byte startNestCnt;
byte argNestCnt = 0;
byte *pMacro;
byte *macroInPtr;
/*
        mtype has the following values
        1 -> IRP
        2 -> IRPC
        3 -> DoRept
        4 -> MACRO Invocation
        5 -> ???
*/
#pragma pack(push, 1)
macroStk_t macro = { .top.blk = 0xffff };
#pragma pack(pop)
word curMacroBlk = 0xFFFF;
word nxtMacroBlk = 0;
word maxMacroBlk = 0;
word macroBlkCnt;
byte macroBuf[129];
byte *savedMacroBufP;
byte *pNextArg;
word localIdCnt;
byte *startMacroLine;
byte *startMacroToken;
byte irpcChr[3] = { 0, 0, 0x81 };               // where irpc char is built (0x81 is end marker)

byte localVarName[] = {'?', '?', 0, 0, 0, 0, 0x80};     // where DoLocal vars are constructed (0x80 is end marker)
/* ov4 compat 2C8C */
byte *contentBytePtr;
byte fixupSeg;
word fixOffset;
byte curFixupHiLoSegId;
byte curFixupType;
byte fixIdxs[4] = {0, 0, 0, 0};
#define fix22Idx        fixIdxs[0]
#define fix24Idx        fixIdxs[1]
#define fix20Idx        fixIdxs[2]
#define fix6Idx         fixIdxs[3]
byte extNamIdx = 0;
bool initFixupReq[4] = {true, true, true, true};
bool firstContent = true;
eof_t rEof = {0xE, 0};
extnames_t rExtnames = {0x18, 0, 0};
byte moduleNameLen = 6;
content_t rContent = {0x18, 0, 0};  /* seems odd would expect 6 not 0x18 */
publics_t rPublics = {0x16, 1, 0, 0, 0};
//rReloc RELOC_T at[&rPublics];
interseg_t rInterseg;
extref_t rExtref;
modend_t rModend = { 4, 4, 0};
word wZERO = 0;
bool inComment = false;
bool noOpsYet = false;
byte nameLen;
byte startSeg = 1;
byte activeSeg;
bool inPublic = false;
bool inExtrn = 0;
bool segDeclared[2];
byte alignTypes[4] = {3, 3, 3, 3};
word externId;
word itemOffset;
bool badExtrn = false;
byte startDefined = 0;
word startOffset = 0;
byte tokenIdx = 0;
byte lineBuf[128];
byte *tokStart[MAX_TOKENS_IDX+1] = {lineBuf};
#pragma pack(push, 1)
tokensymStk_t tokenSym;
#pragma pack(pop)
byte tokenType[MAX_TOKENS_IDX+1];
byte tokenSize[MAX_TOKENS_IDX+1] = {0};
byte tokenAttr[MAX_TOKENS_IDX+1];
word tokenSymId[MAX_TOKENS_IDX+1];
byte *endLineBuf = {lineBuf + 128};
byte ifDepth = 0;
bool skipIf[9];
bool inElse[9];
byte macroCondSP = 0;
byte macroCondStk[17];
byte opSP;
byte opStack[17];
word_t accum[2];
byte acc1RelocFlags;
byte acc2RelocFlags;
bool hasVarRef;
byte acc1ValType;
byte acc2ValType;
word acc1RelocVal;
word acc2RelocVal;
byte curChar = 0;
byte reget = false;
byte lookAhead;
tokensym_t *symTab[3];
tokensym_t *endSymTab[3];
byte *symHighMark;
byte *baseMacroTbl;
byte gotLabel = 0;
byte tokName[36];               // 墠⨫� �� � 12 ����, �� ����� ��ଠ� �� �ﭥ�
byte savName[36];               // 墠⨫� �� � 12 ����, �� ����� ��ଠ� �� �ﭥ�
bool haveNonLabelSymbol;                // true if we have seen a user symbol and confirmed that there is no :
bool haveUserSymbol;                    // true if we have seen a user symbol and  not yet seen a following :
bool xRefPending = false;
byte passCnt = 0;
bool createdUsrSym = false;
bool usrLookupIsID = false;
bool needsAbsValue = false;
word objfd;
word xreffd;
word infd;
word outfd;
word macrofd;
word statusIO;
word openStatus;  /* status of last open for Read */
byte asmErrCode = ' ';
bool spooledControl = false;
bool primaryValid = true;
byte tokI;
bool errorOnLine;
bool atStartLine;
byte curCol = 1;
byte *endItem;
byte *startItem;
word pageLineCnt;
address effectiveAddr;
word pageCnt;
bool showAddr;
bool lineNumberEmitted = false;
bool b68AE = false;
byte tokStr[36] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
word sizeInBuf = {IN_BUF_SIZE/2};
byte inBuf[IN_BUF_SIZE];
byte outbuf[OUT_BUF_SIZE+1];

byte *outP = {outbuf};
byte *endOutBuf;
byte objFile[17] = "                 "; /* 17 spaces */
byte lstFile[17] = "                 "; /* 17 spaces */
byte asxrefTmp[] = ":F0:ASXREF.TMP ";
byte asxref[] = ":F0:ASXREF ";
byte asmacRef[] = ":F0:ASMAC.TMP ";
word srcLineCnt = 1;
#pragma off(unreferenced)
static byte pad6A50[2] = "  ";        /* protects for very big files */
#pragma on(unreferenced)
byte asciiLineNo[4] = "   0";
byte spIdx;
byte lastErrorLine[4] = "   0";
controls_t controls = { .all = {false, false, false, true, true, false, false,
                         true, true, false, 120, 66, 0, false, 0, 0, 0,
                         true, true, true} };

bool ctlListChanged = true;
byte titleLen = {0};
bool controlSeen[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
byte saveStack[8][3];
byte saveIdx = 0;
byte titleStr[64];
byte tokBufLen;
byte tokType;
byte controlId;
byte tokBuf[64];
byte tokBufIdx = 0;
word tokNumVal;
bool isControlLine = false;
bool scanCmdLine;
bool inDB;
bool inDW;
bool inExpression;
bool has16bitOperand;
byte phase;
byte curOpFlags;
byte yyType;
byte curOp;
byte topOp;
bool b6B2C;
byte nextTokType;
bool finished;
bool inNestedParen;
bool expectingOperands;
bool expectingOpcode;
bool condAsmSeen;    /* true when IF, ELSE, ENDIF seen [also macro to check] */
bool b6B33;
bool isInstr = true;
bool expectOp = true;
bool b6B36 = false;
word segLocation[5] = {0, 0, 0, 0, 0};    /* ABS, CODE, DATA, STACK, MEMORY */
word maxSegSize[3] = {0, 0, 0};        /* seg is only ABS, CODE or DATA */
byte cmdLineBuf[129];
address actRead;
word errCnt;
byte *w6BCE;
dword azero = 0;
byte *cmdchP = cmdLineBuf;
byte *controlsP;
bool skipRuntimeError = false;
bool nestedMacroSeen;
byte ii;
byte jj;
byte kk;
byte *curFileNameP;

address aVar;

word controlFileType;     /* 1->INCLUDE 2->PRINT, 3->OBJECT or MACROFILE */
