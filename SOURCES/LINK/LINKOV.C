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

#pragma off(unreferenced)
static byte COPYRIGHT[] = "[C] 1976, 1977, 1979 INTEL CORP";
#pragma on(unreferenced)
byte OVERLAYVERSION[] = "V3.0";
static word zero = 0;
static byte fakeModhdr[1] = {6};
static byte space = ' ';

byte msgrefin[] = " - REFERENCED IN ";

byte modName[32];
static byte ancestorNameNotSet;
static byte fixType;
static word segIdx;     // converted to word to handle 0-255 loops
#pragma off(unreferenced)
static byte pad[3];
#pragma on(unreferenced)
static word segId;
static word outRelocOffset;
static symbol_t **extMapP = 0;
static word externsCount = 0;
static word externsBase = 0;
static word externsEnd = 0;
static bool haveTmpFile = 0;
record_t *outRecordP;


void SeekExtMap()
{
    word blk;

    blk = externsBase / 128 * sizeof(*extMapP);  /* changed as pointers may be > 2 bytes */
    Seek(tmpfilefd, 2, &blk, &zero, &statusIO);
    fatal_FileIO(statusIO, &linkTmpFile[1], true);
} /* SeekExtMap() */

void PageOutExtMap()
{
    if (externsBase >= externsEnd) /* update end boundary in case of partial page */
        externsEnd = externsEnd + 128;
    SeekExtMap();
    Write(tmpfilefd, (pointer)extMapP, 128 * sizeof(*extMapP), &statusIO);
    fatal_FileIO(statusIO, &linkTmpFile[1], true);
} /* PageOutExtMap() */

void PageInExtMap()
{
    SeekExtMap();
    Read(tmpfilefd, (pointer)extMapP, 128 * sizeof(*extMapP), &actRead, &statusIO);
    fatal_FileIO(statusIO, &linkTmpFile[1], true);
    if (actRead != 256)
        fatal_FileIO(ERR204, &linkTmpFile[1], true);
} /* PageInExtMap() */

void AddExtMap(symbol_t *symP)
{
    if ((externsCount & 0xFF80) != externsBase)   /* memory cache full */
    {
        if (! haveTmpFile)             /* if no tmp file create it */
        {
            Delete(&linkTmpFile[1], &statusIO);
            Open(&tmpfilefd, &linkTmpFile[1], 3, 0, &statusIO);
            fatal_FileIO(statusIO, &linkTmpFile[1], true);
            haveTmpFile = true;
        }
        PageOutExtMap();
        if ((externsBase = externsCount & 0xFF80) < externsEnd)
            PageInExtMap();         /* if (the page exists get it from disk */
    }
    extMapP[externsCount - externsBase] = symP;  /* record the symbol mapping */
    externsCount = externsCount + 1;
} /* AddExtMap() */

symbol_t *GetSymbolP(word symId)
{

    if (symId >= externsCount) /* out of range */
        fatal_RecFormat();
    if ((symId & 0xFF80) != externsBase)  /* ! in memory */
    {
        PageOutExtMap();            /* Write() out current */
        externsBase = symId & 0xFF80;     /* set new base */
        PageInExtMap();         /* get relevant page */
    }
    return extMapP[symId - externsBase];     /* return the symbolP */
} /* GetSymbolP() */

void InitExternsMap()
{
    if (extMapP == 0)  /* ! already allocated */
    {
        if (maxExternCnt > 128)    /* max 128 extern entries in memory */
            extMapP = (symbol_t **)GetLow(128 * sizeof(symbol_t **));
        else
            extMapP = (symbol_t **)GetLow(maxExternCnt * sizeof(symbol_t **));
    }
    externsCount = externsBase = externsEnd = 0;
} /* InitExternsMap() */

void FlushTo()
{
    Write(tofilefd, soutP, (word)(outP - soutP), &statusIO);
    fatal_FileIO(statusIO, &toFileName[1], true);
    outP = soutP;
} /* FlushTo() */

void InitRecord(byte type)
{
    if (eoutP - outP < 1028)
        FlushTo();
    outRecordP = (record_t *)outP;
    outRecordP->rectyp = type;
    outP = outP + 3;
} /* InitRecord() */

void EndRecord()
{
    byte crc;
    pointer pch;

    if ((outRecordP->reclen = (word)(outP - &outRecordP->rectyp - 2)) > 1025)
        fatal_FileIO(ERR211, &toFileName[1], true);    /* Record() to long */
    crc = 0;
    for (pch = &outRecordP->rectyp; pch <= outP - 1; pch++) {   /* calculate and insert crc */
        crc = crc + *pch;
    }
    *outP = -crc;   // add the crc
    outP = outP + 1;
} /* EndRecord() */

bool ExtendRec(word cnt)
{
    byte type;

    if (outP + cnt - &outRecordP->rectyp < 1028)   /* room in buffer */
        return false;
    type = outRecordP->rectyp;  /* type for extension record */
    EndRecord();        /* Close() off the current record */
    InitRecord(type);   /* and prepare another */
    return true;
} /* ExtendRec() */


void static  EmitMODHDRComSegInfo(byte segId, word len, byte combine)
{
        if (ExtendRec(sizeof(segdef_t))) /* make sure enough room */
            fatal_FileIO(ERR226, &toFileName[1], true);    /* mod hdr too long */
        ((segdef_t *)outP)->segId = segId;     /* emit segid, name, combine and Size() */
        ((segdef_t *)outP)->len = len;
        ((segdef_t *)outP)->combine = combine;
        outP = outP + sizeof(segdef_t);
} /* EmitMODHDRComSegInfo() */



void EmitMODHDR()
{
    InitRecord(R_MODHDR);
    Pstrcpy(outModuleName, outP);   /* copy the module name */
    outP = outP + outModuleName[0] + 1;
    outP[0] = outTranId;        /* the two reserved bytes */
    outP[1] = outTranVn;
    outP = outP + 2;
    for (segIdx = SEG_CODE; segIdx <= SEG_MEMORY; segIdx++) {   /* regular segments */
        if (segLen[segIdx] > 0)
            EmitMODHDRComSegInfo((byte)segIdx, segLen[segIdx], alignType[segIdx]);
    }
    if (segLen[0] > 0)     /* unamed common segment */
        EmitMODHDRComSegInfo(SEG_BLANK, segLen[0], alignType[0]);
    comdefInfoP = headSegOrderLink;
    while (comdefInfoP)
    {   /* named common segments */
        EmitMODHDRComSegInfo(comdefInfoP->linkedSeg, comdefInfoP->len, comdefInfoP->flags);
        comdefInfoP = comdefInfoP->nxtSymbol;
    }
    EndRecord();
} /* EmitMODHDR() */

void EmitEnding()
{
    InitRecord(R_MODEND);   /* init the record */
    ((modend_t *)outP)->modType = modEndModTyp; /* fill in the mod type, start seg and offset */
    ((modend_t *)outP)->segId = modEndSegId;
    ((modend_t *)outP)->offset = modEndOffset;
    outP = outP + sizeof(modend_t);  /* advance past the data inserted */
    EndRecord();            /* finalise */
    InitRecord(R_MODEOF);      /* emit and eof record as well */
    EndRecord();
    FlushTo();          /* make sure all on disk */
} /* EmitEnding() */

void EmitCOMDEF()
{
    if (headSegOrderLink == 0) /* no named common */
        return;
    InitRecord(R_COMDEF);   /* prep the record */
    comdefInfoP = headSegOrderLink; /* chase down the definitions in seg order */
    while (comdefInfoP) {
        if (ExtendRec(2 + comdefInfoP->name[0])) /* overflow to another record if needed */
            ;       /* if (really not needed here as there are no issues with overflowing */
        *outP = comdefInfoP->linkedSeg;   /* copy the seg and name */
        Pstrcpy(comdefInfoP->name, outP + 1);
        outP = outP + sizeof(comnam_t) + comdefInfoP->name[0]; /* advance output pointer */
        comdefInfoP = comdefInfoP->nxtSymbol;  /* pickup next entry */
    }
    EndRecord();        /* finalise */
} /* EmitCOMDEF() */

void EmitPUBLICS()
{
    for (segIdx = 0; segIdx <= 255; segIdx++) {         /* scan all segs */
        if (segmap[segIdx])        /* seg used */
        {
            InitRecord(R_PUBDEF);  /* init the record */
            *outP = (byte)segIdx;      /* seg needed */
            outP = outP + 1;
            curObjFile = objFileHead;   /* scan all files */
            while (curObjFile)
            {
                curModule = curObjFile->modList;
                while (curModule)
                {   /* && all modules */
                    symbolP = curModule->symlist;
                    while (symbolP)
                    {   /* && all symbols */
                        if (symbolP->linkedSeg == segIdx)    /* this symbol in the right seg */
                        {
                            if (ExtendRec(4 + symbolP->name[0])) /* makes sure enough room in record */
                            {
                                *outP = (byte)segIdx;  /* overflowed to new record so add the segid */
                                outP = outP + 1;
                            }
                            ((def_t *)outP)->offset = symbolP->offsetOrSym;  /* Write() the offset */
                            Pstrcpy(symbolP->name, ((def_t *)outP)->name); /* and the name */
                            outP[symbolP->name[0] + 3] = 0;       /* add the extra 0 reserved byte */
                            outP = outP + 4 + symbolP->name[0];   /* account for data added */
                        }
                        symbolP = symbolP->nxtSymbol; /* loop to next symbol */
                     }
                    curModule = curModule->link;    /* next module*/
                }
                curObjFile = curObjFile->link;  /* next file */
            }
            EndRecord();                /* finish any Open() record */
        }
    }
} /* EmitPUBLICS() */

void EmitEXTNAMES()
{
    if (headUnresolved == 0)       /* no unresolved */
        return;
    InitRecord(R_EXTNAM);     /* init the record */
    unresolved = 0;
    symbolP = headUnresolved;
    while (symbolP)
    {
        if (ExtendRec(2 + symbolP->name[0])) /* check room for len, symbol && 0 */
        {
            /* no need for special action on extend */
        }
        Pstrcpy(symbolP->name, outP); /* copy the len + symbol */
        outP[symbolP->name[0] + 1] = 0;            /* add a 0 */
        outP = outP + 2 + symbolP->name[0];       /* advance past inserted data */
        symbolP->offsetOrSym = unresolved;        /* record the final ext sym id */
        unresolved = unresolved + 1;            /* for next symbol */
        symbolP = symbolP->nxtSymbol;
    }
    EndRecord();                        /* clean closure of record */
} /* EmitEXTNAMES() */

void Emit_ANCESTOR()
{
    if (ancestorNameNotSet)                    /* we have a module name to use */
    {
        InitRecord(R_ANCEST);         /* init the record */
        Pstrcpy(modName, outP);     /* copy name */
        outP = outP + modName[0] + 1;
        EndRecord();
        ancestorNameNotSet = 0;             /* it is now set */
    }
} /* Emit_ANCESTOR() */

byte SelectOutSeg(byte seg)
{
    outRelocOffset = 0;     /* only code and data modules are relative to module location */
    if (seg == SEG_CODE)
        outRelocOffset = curModule->scode;
    else if (seg == SEG_DATA)
        outRelocOffset = curModule->sdata;
    return segmap[seg];     /* return seg mapping */
} /* SelectOutSeg() */

void Pass2MODHDR()
{
    Pstrcpy(inP, modName);  /* Read() in the module name */
    ancestorNameNotSet = true;      /* note the ancestor record has not been written */
    for (segId = 0; segId <= 255; segId++) {            /* init the segment mapping */
        segmap[segId] = (byte)segId;
    }
    GetRecord();
} /* Pass2MODHDR() */

void Pass2COMDEF()
{
    while (inP < erecP) {       /* while more common definitions */
        if (! Lookup(((comnam_t *)inP)->name, &comdefInfoP, F_ALNMASK)) /* check found */
            fatal_Error(ERR219);   /* Phase() Error() */
        segmap[*inP] = comdefInfoP->linkedSeg;            /* record the final linked seg where this goes */
        inP = inP + 2 + ((comnam_t *)inP)->name[0];               /* past this def */
    }
    GetRecord();
} /* Pass2COMDEF() */

void Pass2EXTNAMES()
{
    while (inP < erecP) {       /* while more external definitions */
        if (! Lookup(inP, &symbolP, F_SCOPEMASK))  /* get the name */
            fatal_Error(ERR219);   /* phase Error() - didn't Lookup() !!! */
        AddExtMap(symbolP);
        if (symbolP->flags == F_EXTERN)  /* still an extern */
        {               /* Write() the unresolved reference info */
            WriteAndEcho(&space, 1);
            WriteAndEcho(&symbolP->name[1], symbolP->name[0]);
            WAEFnAndMod(msgrefin, 17); /* ' - REFERENCED IN ' */
        }
        inP = inP + 2 + *inP;  /* 2 for len and extra 0 */
    }
    GetRecord();
} /* Pass2EXTNAMES() */

/* the names below are made global to support nested procedures
 * rather than adding additional arguments to function calls
 */

static word inContentStart, inContentEnd;
static fixup_t *fixupP, *headRelocP;
static reloc_t *relocP;
word outContentRelocOffset;

static void BoundsChk(word addr)
{
        if (addr < inContentStart || inContentEnd < addr)
                fatal_Error(ERR213);   /* fixup bounds Error() */
}

static void GetTypeAndSegHead(fixup_t *afixupP, word typeAndSeg)
{
        fixupP = afixupP->link;   /* chase down the fixup chain matching seg and fixup type */
        while (fixupP)
        {
            if (fixupP->typeAndSeg == typeAndSeg)    /* found existing list */
                return;
            afixupP = fixupP;          /* step along */
            fixupP = fixupP->link;
        }
        fixupP = (fixup_t *)GetHigh(sizeof(fixup_t));         /* add to the list */
        fixupP->link = afixupP->link;
        afixupP->link = fixupP;
        fixupP->typeAndSeg = typeAndSeg;          /* save the typeAndSeg */
        fixupP->relocList = 0;
} /* GetTypeAndSegHead() */



static void AddRelocFixup(byte seg, word addr)
{
        GetTypeAndSegHead((fixup_t *)&headRelocP, seg * 256 + fixType);    /* add to reloc list */
        relocP = (reloc_t *)GetHigh(sizeof(reloc_t));
        relocP->link = fixupP->relocList;
        fixupP->relocList = relocP;
        relocP->offset = addr + outContentRelocOffset;
    } /* AddRelocFixup() */

void Pass2CONTENT()
{
    byte outContentSeg, crc;
    pointer p;
    pointer savedOutP;
    word savedRecLen;
    word bytes2Read;
    pointer markheap = 0;   // compiler complains
    fixup_t *headexternP;

    inRecordP = (record_t *)fakeModhdr;    /* keep inRecordP pointing to a modhdr */
    InitRecord(R_MODDAT);  /* init record */
    outRecordP->reclen = savedRecLen = recLen;    /* output length() same as Input() length() */
    savedOutP = outP;       /* saved start of record */
    crc = High(recLen) + R_MODDAT + Low(recLen);   /* initialise crc */
    bufP = inP;

    while (recLen > 0) {        /* process all of the record */
        if (savedRecLen > 1025)    /* flush current output buf */
            FlushTo();
        if ((bytes2Read = recLen) > npbuf)    /* Read() in at most npbuf bytes */
            bytes2Read = npbuf;
        ChkRead(bytes2Read);    /* Load() them into memory */
        memmove(outP, bufP, bytes2Read); /* copy to the output buf */
        for (p = outP; p <= outP + bytes2Read - 1; p++) {   /* update the CRC */
            crc = crc + *p;
        }
        bufP = bufP + bytes2Read;   /* advance the pointers */
        outP = outP + bytes2Read;
        recLen = recLen - bytes2Read;
    }
    if (crc != 0)
        fatal_Error(ERR208);   /* Checksum() Error() */
    GetRecord();            /* prime next record */
    if (savedRecLen > 1025)    /* we can't fix up a big record */
        return;

    headexternP = headRelocP = 0;    /* initialise the fixup chains */
    savedOutP = (outP = savedOutP) + 3;        /* skip seg and offset */
    outContentSeg = SelectOutSeg(((moddat_t *)outP)->segId);   /* get the mapped link segment */
    outContentRelocOffset = outRelocOffset;     /* and the content reloc base */
    inContentEnd = (inContentStart = ((moddat_t *)outP)->offset) + savedRecLen - 5; /* the address range */
    ((moddat_t *)outP)->segId = outContentSeg;         /* update the out seg & address values */
    ((moddat_t *)outP)->offset = inContentStart + outContentRelocOffset;
    markheap = GetHigh(0);              /* get heap marker */

    /* process the relocate records */
    while (inRecordP->rectyp == R_FIXEXT || inRecordP->rectyp == R_FIXLOC || inRecordP->rectyp == R_FIXSEG) {
        if (inRecordP->rectyp == R_FIXEXT)
        {
            if ((fixType = *inP) - 1 > 2)    /* make sure combine is valid */
                fatal_RecFormat();
            inP = inP + 1;      /* past the record byte */
            while (inP < erecP) {       /* process all of the extref fixups */
                BoundsChk(((extref_t *)inP)->offset); /* check fixup valid */
                if (fixType == FBOTH)
                    BoundsChk(((extref_t *)inP)->offset + 1); /* check upper byte also in range */
                symbolP = GetSymbolP(((extref_t *)inP)->symId);     /* get symbol entry for the ext symid */
                if (symbolP->flags == F_PUBLIC)          /* is a public so resolved */
                {
                    p = ((extref_t *)inP)->offset - inContentStart + savedOutP;   /* Lookup() fixup address in buffer */
                    switch(fixType) {            /* relocate to segs current base */
                    case 1: *p = *p + Low(symbolP->offsetOrSym); break;
                    case 2: *p = *p + High(symbolP->offsetOrSym); break;
                    case 3: *(wpointer)p = *(wpointer)p + symbolP->offsetOrSym; break;
                    }
                    if (symbolP->linkedSeg != SEG_ABS)       /* if ! ABS add a fixup entry */
                        AddRelocFixup(symbolP->linkedSeg, ((extref_t *)inP)->offset);
                }
                else                        /* is an extern still */
                {
                    GetTypeAndSegHead((fixup_t *)&headexternP, fixType);   /* add to extern list, seg not needed */
                    relocP = (reloc_t *)GetHigh(sizeof(extfixup_t));   /* allocate the extFixup descriptor */
                    relocP->link = fixupP->relocList;        /* chain it in */
                    fixupP->relocList = relocP;
                    ((extfixup_t *)relocP)->offset = ((extref_t *)inP)->offset + outContentRelocOffset;  /* add in the location */
                    ((extfixup_t *)relocP)->symId = symbolP->symId;  /* and the symbol id */
                }
                inP = inP + 4;
            }
        }
        else    /* reloc or interseg */
        {
            segIdx = outContentSeg;         /* get default reloc seg */
            outRelocOffset = outContentRelocOffset; /* and reloc base to that of the content record */
            if (inRecordP->rectyp == R_FIXSEG)   /* if we are interseg then update the reloc seg */
            {
                segIdx = SelectOutSeg(*inP); /* also updates the outRelocOffset */
                inP = inP + 1;
            }
            if (segIdx == 0)           /* ABS is illegal */
                fatal_RecFormat();
            if ((fixType = *inP) - 1 > 2)    /* bad fix up type ? */
                fatal_RecFormat();
            inP = inP + 1;          /* past fixup */
            while (inP < erecP) {           /* process all of the relocates */
                BoundsChk(*(wpointer)inP);    /* fixup in range */
                if (fixType == FBOTH)   /* && 2nd byte for both byte fixup */
                    BoundsChk(*(wpointer)inP + 1);
                p = *(wpointer)inP - inContentStart + savedOutP;  /* location of fixup */
                switch(fixType) {        /* relocate to seg current base */
                case FLOW: *p = *p + Low(outRelocOffset); break;
                case FHIGH: *p = *p + High(outRelocOffset); break;
                case FBOTH: *(wpointer)p = *(wpointer)p + outRelocOffset; break;
                }
                AddRelocFixup((byte)segIdx, *(wpointer)inP);    /* add a new reloc fixup */
                inP = inP + 2;
            }
        }
        GetRecord();    /* next record */
    }
    outP = outP + savedRecLen - 1;
    EndRecord();                /* finish the content record */
    fixupP = headexternP;           /* process the extern lists */
    while (fixupP)
    {
        InitRecord(R_FIXEXT);   /* create a extref record */
        *outP = Low(fixupP->typeAndSeg); /* set the fix type */
        outP = outP + 1;
        relocP = fixupP->relocList;   /* process all of the extref of this fixtype */
        while (relocP)
        {
            if (ExtendRec(4))  /* make sure we have room */
            {           /* if (! add the fixtype to the newly created follow on record */
                *outP = Low(fixupP->typeAndSeg);
                outP = outP + 1;
            }
            ((extref_t *)outP)->symId = ((extfixup_t *)relocP)->symId;   /* put the sym number */
            ((extref_t *)outP)->offset = ((extfixup_t *)relocP)->offset; /* and the fixup location */
            outP = outP + 4;        /* update to reflect 4 bytes written */
            relocP = relocP->link; /* chase the list */
        }
        EndRecord();            /* Close() any Open() record */
        fixupP = fixupP->link;        /* look for next fixtype list */
    }
    fixupP = headRelocP;            /* now do the relocates */
    while (fixupP)
    {
        InitRecord(R_FIXSEG); /* interseg record */
        ((interseg_t *)outP)->segId = High(fixupP->typeAndSeg);   /* fill in segment */
        ((interseg_t *)outP)->fixType = Low(fixupP->typeAndSeg);    /* and fixtype */
        outP = outP + 2;
        relocP = fixupP->relocList;   /* chase down the references */
        while (relocP)
        {
            if (ExtendRec(2))  /* two bytes || create follow on record */
            {           /* fill in follow on record */
                ((interseg_t *)outP)->segId = High(fixupP->typeAndSeg);
                ((interseg_t *)outP)->fixType = Low(fixupP->typeAndSeg);
                outP = outP + 2;
            }
            *(wpointer)outP = ((extfixup_t *)relocP)->offset;    /* set the fill the offset */
            outP = outP + 2;
            relocP = relocP->link; /* next record */
        }
        EndRecord();
        fixupP = fixupP->link;
    }
    membot = markheap;      /* return heap */

} /* Pass2CONTENT() */

void Pass2LINENO()
{
    Emit_ANCESTOR();        /* make sure we have a valid ancestor record */
    InitRecord(R_LINNUM);
    *outP = SelectOutSeg(*inP);   /* add the seg id info */
    outP = outP + 1;
    inP = inP + 1;
    while (inP < erecP) {       /* while more public definitions */
        ((line_t *)outP)->offset = outRelocOffset + ((line_t *)inP)->offset;    /* relocate the offset */
        ((line_t *)outP)->linNum = ((line_t *)inP)->linNum;         /* copy the line number */
        outP = outP + 4;
        inP = inP + 4;
    }
    EndRecord();
    GetRecord();
} /* Pass2LINENO() */

void Pass2ANCESTOR()
{
    Pstrcpy(outP, modName);  /* copy the module name over and mark as valid */
    ancestorNameNotSet = true;      /* note it isn't written yet */
    GetRecord();
} /* Pass2ANCESTOR() */

void Pass2LOCALS()
{
    Emit_ANCESTOR();            /* emit ancestor if (needed */
    InitRecord(R_LOCDEF);       /* init locals record */
    *outP = SelectOutSeg(*inP);   /* map the segment and set up relocation base */
    outP = outP + 1;
    inP = inP + 1;
    /* note the code below relies on the source file having records <1025 */
    while (inP < erecP) {       /* while more local definitions */
        ((def_t *)outP)->offset = outRelocOffset + ((def_t *)inP)->offset;    /* Write() offset and symbol */
        Pstrcpy(((def_t *)inP)->name, ((def_t *)outP)->name);
        ((def_t *)outP)->name[((def_t *)inP)->name[0] + 1] = 0;
        outP = outP + 4 + ((def_t *)inP)->name[0];         /* advance out and in pointers */
        inP = inP + 4 + ((def_t *)inP)->name[0];
    }
    EndRecord();            /* clean end */
    GetRecord();            /* next record */
} /* Pass2LOCALS() */

/* process pass 2 records */
void Phase2()
{
//    if ((membot = MEMORY) > topHeap)             /* check that memory still ok after overlay */
//        fatal_FileIO(ERR210, &toFileName[1], true);    /* insufficient memory */
    soutP = outP = GetLow(npbuf);                /* reserve the output buffer */
    eoutP = soutP + npbuf;
    InitExternsMap();
    Open(&tofilefd, &toFileName[1], 2, 0, &statusIO);       /* target file */
    fatal_FileIO(statusIO, &toFileName[1], true);
    // ������� ��� ��������� �����
    ConOutStr("  OUT: ", 7);
    ConOutStr(&toFileName[1], toFileName[0]);
    ConOutStr("\n", strlen("\n"));

    EmitMODHDR();                       /* process the simple records */
    EmitCOMDEF();
    EmitPUBLICS();
    EmitEXTNAMES();
    curObjFile = objFileHead;                   /* process all files */
    while (curObjFile)
    {
        if (curObjFile->publicsMode)               /* don't need to do anything more for publics only file */
            curObjFile = curObjFile->link;
        else
        {
            OpenObjFile(FALSE);             /* Open() file */
            curModule = curObjFile->modList;            /* for each module in the file */
            while (curModule)
            {
                GetRecord();                /* Read() modhdr */
                if (curObjFile->hasModules)        /* if we have modules Seek() to the current module's location */
                {
                    Position(curModule->blk, curModule->byt);
                    GetRecord();            /* and Load() its modhdr */
                }
                if (inRecordP->rectyp != R_MODHDR)
                    fatal_Error(ERR219); /* phase Error() */
                InitExternsMap();           /* prepare for processing this module's extdef records */
                while (inRecordP->rectyp != R_MODEND) { /* run through the whole module */
                    switch (inRecordP->rectyp) {
                    case 0x00: fatal_RelocRec(); break;
                    case 0x02: Pass2MODHDR(); break;  /* R_MODHDR */
                    case 0x04: ; break;           /* R_MODEND */
                    case 0x06: Pass2CONTENT(); break; /* R_CONTENT */
                    case 0x08: Pass2LINENO(); break;
                    case 0x0A: fatal_RelocRec(); break;
                    case 0x0C: fatal_RelocRec(); break;
                    case 0x0E: fatal_FileIO(ERR204, &inFileName[1], true); break; /* 0E Premature() eof */
                    case 0x10: Pass2ANCESTOR(); break;
                    case 0x12: Pass2LOCALS(); break;
                    case 0x14: fatal_RelocRec(); break;
                    case 0x16: GetRecord(); break;
                    case 0x18: Pass2EXTNAMES(); break;
                    case 0x1A: fatal_RelocRec(); break;
                    case 0x1C: fatal_RelocRec(); break;
                    case 0x1E: fatal_RelocRec(); break;
                    case 0x20: fatal_RecSeq(); break;
                    case 0x22: fatal_RecSeq(); break;
                    case 0x24: fatal_RecSeq(); break;
                    case 0x26: fatal_RecSeq(); break;
                    case 0x28: fatal_RecSeq(); break;
                    case 0x2A: fatal_RecSeq(); break;
                    case 0x2C: fatal_RecSeq(); break;
                    case 0x2E: Pass2COMDEF(); break;
                    }
                }
                curModule = curModule->link;
            }
            CloseObjFile();
        } /* of else */
    }   /* of do while */
    EmitEnding();   /* Write() final modend and eof record */
    Close(tofilefd, &statusIO);
    fatal_FileIO(statusIO, &toFileName[1], true);
    if (haveTmpFile)       /* clean any tmp file up */
    {
        Close(tmpfilefd, &statusIO);
        fatal_FileIO(statusIO, &linkTmpFile[1], true);
        Delete(&linkTmpFile[1], &statusIO);
    }
} /* Phase2() */

