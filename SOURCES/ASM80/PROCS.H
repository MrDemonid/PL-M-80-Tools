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


void AsmComplete();
void BalanceError();
bool BlankAsmErrCode();
bool MPorNoErrCode();
bool ChkGenObj();
void ChkInvalidRegOperand();
void ChkLF();
void Close(word conn, word *statusP);
void CloseF(word conn);
void CollectByte(byte rc);
void CommandError();
void Delete(byte *pathP, word *statusP);
void DoPass();
void EmitXref(byte xrefMode, byte *name);
void Error(word ErrorNum);
NORETURN(Exit());
void ExpressionError();
void FileError();
void FinishAssembly();
void FinishLine();
void FinishPrint();
void FlushM();
void Flushout();
void GenAsxref();
void GetAsmFile();
byte GetCh();
byte GetChClass();
byte GetCmdCh();
void GetId(byte type);
void GetNum();
word GetNumVal();
byte GetPrec(byte topOp);
byte GetSrcCh();
void GetStr();
byte HaveTokens();
void IllegalCharError();
void InitLine();
void InitRecTypes();
void InsertByteInMacroTbl(byte c);
void InsertCharInMacroTbl(byte c);
void IoErrChk();
void IoError(byte *path);
bool IsComma();
bool IsCR();
bool IsGT();
bool IsLT();
bool IsPhase1();
bool IsPhase2Print();
bool IsReg(byte type);
bool IsRParen();
bool IsSkipping();
bool IsTab();
bool IsWhite();
void Load(byte *pathP, word LoadOffset, word swt, word entryP, word *statusP);
void LocationError();
byte Lookup(byte tableId);
byte *MemCk();
void MkCode(byte arg1b);
void MultipleDefError();
void Nest(byte arg1b);
void NestingError();
byte Nibble2Ascii(byte n);
byte NonHiddenSymbol();
byte NxtTokI();
void OperandError();
void Open(word *connP, const byte *pathP, word access, word echo, word *statusP);
void OpenSrc();
void Outch(byte c);
void OutStrN(byte *s, byte n);
void Ovl11();
void Ovl8();
void ParseControlLines();
void PackToken();
void ParseControls();
void PhaseError();
byte *Physmem();
void PopToken();
void PrintCmdLine();
void PrintDecimal(word n);
void PrintLine();
void PushToken(byte type);
void Read(word conn, byte *buffP, word count, word *actualP, word *statusP);
void ReadM(word blk);
void ReinitFixupRecs();
void Rescan(word conn, word *statusP);
void ResetData();
void RuntimeError(byte errCode);
word SafeOpen(byte *pathP, word access);
void Seek(word conn, word mode, word *blockP, word *byteP, word *statusP);
void SetExpectOperands();
bool ShowLine();
void Skip2EOL();
void Skip2NextLine();
void SkipWhite();
void SkipWhite_2();
void SourceError(byte errCh);
void StackError();
int Start();
bool StrUcEqu(byte *s, byte *t);
void ResultType();
void Sub546F();
void InsertMacroSym(word val, byte type);
void Sub7041_8447();
void DoIrpX(byte mtype);
void initMacroParam();
void GetMacroToken();
void DoMacro();
void DoMacroBody();
void DoEndm();
void DoExitm();
void DoIterParam();
void DoRept();
void DoLocal();
void Sub78CE();
void SyntaxError();
void sysInit(int argc, char **argv);
bool TestBit(byte bitIdx, byte *bitVector);
void Tokenise();
void UndefinedSymbolError();
void UnNest(byte arg1b);
void UnpackToken(word *src, byte *dst);
void UpdateSymbolEntry(word val, byte type);
void ValueError();
void Write(word conn, const byte *buffP, word count, word *statusP);
void WriteExtName();
void WriteM();
void WriteModend();
void WriteModhdr();
void WriteRec(byte *recP);

#define move(cnt, src, dst)     memcpy(dst, src, cnt)

void DumpSymbols(byte tableId);
void DumpOpStack();
void DumpTokenStack(bool pop);

void DumpTokenStackItem(int i, bool pop);
void ShowYYType();

void showVersion(FILE *fp, bool full);
