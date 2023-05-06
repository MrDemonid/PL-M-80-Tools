/****************************************************************************
 *  objcpm: C port of Digital Research OBJCPM.COM for CP/M
 *  Copyright (C) 2022 Andrey Hlus                                          *
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

#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "objcpm.h"

/*
  типы записей в объектном файле
*/
#define R_MODHDR 2
#define R_MODEND 4
#define R_MODDAT 6
#define R_LINNUM 8
#define R_MODEOF 0xE
#define R_ANCEST 0x10
#define R_LOCDEF 0x12
#define R_PUBDEF 0x16
#define R_EXTNAM 0x18
#define R_FIXEXT 0x20
#define R_FIXLOC 0x22
#define R_FIXSEG 0x24
#define R_LIBLOC 0x26
#define R_LIBNAM 0x28
#define R_LIBDIC 0x2A
#define R_LIBHDR 0x2C
#define R_COMDEF 0x2E


#define OUT_SYM 0
#define OUT_LIN 2
#define OUT_SCR 1


/*
  флаги вывода на устройства
*/
char isMakeCOM;
char isMakeLIN;
char isMakeSYM;
char isBinary;
char isOutPrinter;

/*
  имена файлов
*/
char szFileObj[_MAX_PATH];
char szFileSym[_MAX_PATH];
char szFileLin[_MAX_PATH];
char szFileCom[_MAX_PATH];
char szFileName[_MAX_PATH];     // имя входящего файла без расширения

/*
  данные входящего файла
*/
FILE *hObj;
unsigned int maxPosBufObj;
unsigned int posBufObj;
char BufferObj[2048] ;

/*
  данные для SYM-файла
*/
FILE *hSym;
int maxPosBufSym;
int posBufSym;
char BufferSym[2048] ;

/*
  данные для LIN-файла
*/
FILE *hLin;
int maxPosBufLin;
int posBufLin;
char BufferLin[2048] ;

/*
  данные для COM-файла
*/
FILE *hCom;
int maxPosBufCom;
int posBufCom;
char BufferCom[2048] ;

unsigned int EmptyAddress;      // свободный адрес за концом конечного файла (программы)
unsigned int BaseAddress;       // адрес загрузки конечного файла (программы)
unsigned int StartingAddress;   // адрес точки входа в программу

/*
  парсер
*/
char prnLineCount;  // количество выведенных символов в текущей линии принтера
char outDevType;    // устройство вывода: 0 - SYM, 1 - screen, 2 - LIN
char tabCount;      // количество табуляций для вывода (они кэшируются
                    // и выводятся при случае скопом)


char tabCount;      // количество табуляций для вывода (кешируются и выводятся скопом)
char symTabCount;   // то же для SYM?
char linTabCount;   // то же для LIN?
char recsInSYMline; // для выравнивая строк в SYM файле
char recsInLINline; // количество записей в текущей строке файла LIN
char prnLineCount;  // количество символов в распечатанной строке

unsigned int word_BD1;
char byte_BD3;




void closeFiles();


struct {
    char *dev;
    unsigned char devtype;
} deviceMap[] = {
    { "F0", 3 },{ "F1", 3 },{ "F2", 3 },{ "F3", 3 },    // 0 - 3
    { "F4", 3 },{ "F5", 3 },{ "TI", 0 },{ "TO", 1 },    // 4 - 7
    { "VI", 0 },{ "VO", 1 },{ "I1", 0 },{ "O1", 1 },    // 8 - 11
    { "TR", 0 },{ "HR", 0 },{ "R1", 0 },{ "R2", 0 },    // 12 - 15
    { "TP", 1 },{ "HP", 1 },{ "P1", 1 },{ "P2", 1 },    // 16 - 19
    { "LP", 1 },{ "L1", 1 },{ "BB", 2 },{ "CI", 0 },    // 20 - 23
    { "CO", 1 },{ "F6", 3 },{ "F7", 3 },{ "F8", 3 },    // 24 - 27
    { "F9", 3 } };                                                                              // 28

const int nDevices = (sizeof(deviceMap) / sizeof(deviceMap[0]));

/* preps the deviceId and filename of an spath info record
   returns standard error codes as appropriate
*/
static int ParseIsisName(spath_t *pInfo, const char *isisPath)
{
    int i;
    char dev[3];

    strcpy(dev,"F0");   // default device is F0

    memset(pInfo, 0, sizeof(spath_t));
    pInfo->deviceId = 0xff;             // prefill incase of error

    if (isisPath[0] == ':') {   // check for :XX:
        if (strlen(isisPath) < 4 || isisPath[3] != ':')
            return ERROR_BADFILENAME;
        dev[0] = toupper(isisPath[1]);
        dev[1] = toupper(isisPath[2]);
        isisPath += 4;
    }

    for (i = 0; i < nDevices; i++) // look up device
        if (strcmp(dev, deviceMap[i].dev) == 0)
            break;
    if (i >= nDevices)
        return ERROR_BADDEVICE;
    pInfo->deviceId = i;
    pInfo->deviceType = deviceMap[i].devtype;

    if (pInfo->deviceType == 3) { // parse file name if file device
        for (i = 0; i < 6 && isalnum(*isisPath); i++)
            pInfo->name[i] = toupper(*isisPath++);
        if (i == 0)
            return ERROR_BADFILENAME;
        if (*isisPath == '.') {
            isisPath++;
            for (i = 0; i < 3 && isalnum(*isisPath); i++)
                pInfo->ext[i] = toupper(*isisPath++);
            if (i == 0)
                return ERROR_BADEXT;
        }
    }
    return isalnum(*isisPath) ? ERROR_BADFILENAME : ERROR_SUCCESS;
}


/*
    map an isis file into a real file
    the realFile contains is prefixed with two bytes
    deviceId and deviceType
    return isis error status
 */

static int MapFile(osfile_t *osfileP, const char *isisPath)
{
    spath_t info;
    const char *src;
    char buf[8];
    int status;
    static unsigned char modes[] = { READ_MODE, WRITE_MODE, UPDATE_MODE, UPDATE_MODE + RANDOM_ACCESS };

    if ((status = ParseIsisName(&info, isisPath)) != ERROR_SUCCESS) // get canocial name
        return status;

    osfileP->deviceId = info.deviceId;
    osfileP->modes = modes[info.deviceType];

    if (22 <= info.deviceId && info.deviceId <= 24) // BB, CI, CO
        return ERROR_SUCCESS;

    /* see if user has provided a device map */
    sprintf(buf, ":%s:", deviceMap[info.deviceId].dev);      // look for any mapping provided
    src = getenv(buf);

    if (!src && info.deviceId != 0) { // no mapping and not :F0:
        fprintf(stderr, "No mapping for :%s:\n", deviceMap[info.deviceId].dev);
        return ERROR_NOTREADY;
    }
    if (src && *src) {
        strcpy(osfileP->name, src);
        /* for files append a path separator if there isn't one */
        if (info.deviceType == 3)
            if (strchr(osfileP->name, 0)[-1] != '/' && strchr(osfileP->name, 0)[-1] != '\\')
                strcat(osfileP->name, "\\");
    }
    else
        osfileP->name[0] = 0;
    if (info.deviceType == 3) { // add file name
        char *s = strchr(osfileP->name, 0);             // get end of string
        int i;
        for (i = 0; i < 6 && info.name[i]; i++)
            *s++ = tolower(info.name[i]);
        if (info.ext[0]) {
            *s++ = '.';
            for (i = 0; i < 3 && info.ext[i]; i++)
                *s++ = tolower(info.ext[i]);
        }
        *s = 0;
    }
    return ERROR_SUCCESS;
}



void doHelp()
{
    printf("Convert ISIS-II OBJ file to CP/M COM file.  ver. 1.1\n");
    printf("Usage: OBJCPM in_file [${[+-][key],...}]\n");
    printf("    params:\n");
    printf("        $    - signature of block parameters.\n");
    printf("        [+-] - switch on/off params.\n");
    printf("    keys:\n");
    printf("        C    - produce COM.\n");
    printf("        P    - out on printer, else on file SYM.\n");
    printf("        L    - make LIN file.\n");
    printf("        B    - \n");
}

/*
  прерывание программы по критической ошибке
*/
void doExit(char *str)
{
    printf("Error: %s\n", str);
    closeFiles();
    exit(1);
}



/*****************************************************************************

  параметры командной строки

 *****************************************************************************/

char cmdLine[128];
char isSecondParam;     // второй параметр: 0  - имя какого-то модуля
                        //                  FF - начинается с '$' или отсутствует
char szSecondParam[8];  // собственно второй параметр
                        // (символ '$' меняется на пробел)



void getSecondParam(int argc, char *argv[])
{
    memset(szSecondParam, ' ', 8);
    isSecondParam = 0xFF;

    if (argc < 3)
        return;

    if (strlen(argv[2]) > 8)
        memcpy(szSecondParam, argv[2], 8);
    else
        memcpy(szSecondParam, argv[2], strlen(argv[2]));

    if (szSecondParam[0] == '$' || szSecondParam[0] == ' ')
    {
        isSecondParam = 0xFF;
        szSecondParam[0] = ' ';
    } else {
        isSecondParam = 0;
    }
}

void parseParam(int argc, char *argv[])
{
    char *ptr;
    char flag;
    int len, i;
    osfile_t osfile;

    if (strlen(argv[1]) > 16)
        doExit("bad object file name!");

    // получаем входящий файл
    printf("INPUT: %s\n", argv[1]);

    if (MapFile(&osfile, argv[1]) != ERROR_SUCCESS)
    {
        doExit("bad file name!");
    }
    strcpy(szFileObj, osfile.name);
    strcpy(szFileName, osfile.name);

    ptr = strrchr(szFileName, '.');
    if (ptr)
        *ptr = '\0';
    // сводим все параметры в одну строку
    len = 0;
    for (i=2; i < argc && (len+strlen(argv[i])+1) < sizeof(cmdLine)-2; i++)
    {
        //strcat(cmdLine, " ");
        strcat(cmdLine, argv[i]);
        len += strlen(argv[i]) + 1;
    }
    if (i < argc)
        printf("WARNING! Command line truncated\n");

    // теперь разбираем параметры
    isMakeCOM = -1;
    isMakeLIN = -1;
    isMakeSYM = -1;
    isBinary = -1;
    isOutPrinter = 0;

    ptr = &cmdLine;
    while (*ptr != '$')
    {
        if (*ptr == 0)
            return;
        ptr++;
    }
    flag = -1;
    do {
        ptr++;
        if (*ptr == '-')
            flag = 0;
        else if (*ptr == '+')
            flag = -1;
        else {
            switch (*ptr)
            {
                case 'C': isMakeCOM = flag;
                          break;
                case 'P': isOutPrinter = flag;
                          break;
                case 'L': isMakeLIN = flag;
                          break;
                case 'B': isBinary = flag;
                          break;
            }
        }
    } while (*ptr != 0);

    if (isOutPrinter != 0)
        isMakeSYM = 0;
}

/*****************************************************************************

 OBJ

 *****************************************************************************/
int obj_Open()
{
    maxPosBufObj = 2048;
    posBufObj = 2048;

    hObj = fopen(szFileObj, "rb");
    if (hObj == NULL)
    {
        printf("ERROR: Can't open object file '%s'!\n", szFileObj);
        return 0;
    }
    return 1;
}


/*
  читает байт с потока входного файла
  на выходе:
      0 - закончили чтение
     -1 - считали очередной байт
*/
char obj_GetCh(char *res)
{
    int result;

    if (posBufObj >= maxPosBufObj)
    {
        // буфер пуст, пора подгружать из файла
        posBufObj = 0;
        while (posBufObj < maxPosBufObj)
        {
            result = fread(&BufferObj[posBufObj], 1, 128, hObj);
            if ( result != 128)
            {
                maxPosBufObj = posBufObj + result;
                break;
            }
            posBufObj += 128;
        }
        posBufObj = 0;
    }
    if (maxPosBufObj == 0)
    {
        return 0;                 // EOF
    }
    *res = BufferObj[posBufObj];
    posBufObj++;
    return -1;
}

/*
  чтение байта с исходника
  res - считанный байт
  c - возможно CRC
  d - декрементирующий счетчик
  на выходе:
      0 - закончили чтение
     -1 - успешное чтение
*/

char obj_GetByte(char *res, char *c, unsigned int *d)
{
    char ch;

    if (!obj_GetCh(&ch))
        doExit("unexpected end of file!");
    *c += ch;
    *d = *d - 1;
    *res = ch;
    return -1;
}

unsigned int obj_GetWord(char *c, unsigned int *d)
{
    char hi, lo;
    unsigned int res;

    if (!obj_GetByte(&lo, c, d))
        doExit("unexpected end of file!");
    if (*d == 0)
        doExit("record too long!");
    if (!obj_GetByte(&hi, c, d))
        doExit("unexpected end of file!");
    if (*d == 0)
        doExit("record too long!");
    res = (((unsigned int) hi << 8) + lo) & 0xFFFF;
    return res;
}




/*****************************************************************************

 SYM

 *****************************************************************************/
int sym_Open()
{
    if (isMakeSYM)
    {
        strcpy(szFileSym, szFileName);
        strcat(szFileSym, ".sym");
        maxPosBufSym = 2048;
        posBufSym = 0;
        hSym = fopen(szFileSym, "wb");
        if (hSym == NULL)
        {
            printf("ERROR: Can't create SYM file '%s'\n", szFileSym);
            return 0;
        }
    } else
        hSym = NULL;

//    fwrite("TEST SYM FILE", 1, 14, hSym);
    return 1;
}

int sym_PutByte(char ch)
{
    int res;

    if (posBufSym >= maxPosBufSym)
    {
        // буфер полон, пора скидывать на диск
        posBufSym = 0;
        while (posBufSym < maxPosBufSym)
        {
            res = fwrite(&BufferSym[posBufSym], 1, 128, hSym);
            if (res != 128)
            {
                printf("ERROR: write to file '%s'\n", szFileSym);
//                return 0;
            }
            posBufSym += 128;
        }
        // завершаем сброс на диск
        posBufSym = 0;
    }
    BufferSym[posBufSym] = ch;
    posBufSym++;
    return -1;
}


/*****************************************************************************

 LIN

 *****************************************************************************/
int lin_Open()
{
    if (isMakeLIN)
    {
        strcpy(szFileLin, szFileName);
        strcat(szFileLin, ".lin");
        maxPosBufLin = 2048;
        posBufLin = 0;
        hLin = fopen(szFileLin, "wb");
        if (hLin == NULL)
        {
            printf("ERROR: Can't create file '%s'\n", szFileLin);
            return 0;
        }
    } else
        hLin = NULL;

    return 1;
}

int lin_PutByte(char ch)
{
    if (posBufLin >= maxPosBufLin)
    {
        // буфер полон, пора скидывать на диск
        posBufLin = 0;
        while (posBufLin < maxPosBufLin)
        {
            if (fwrite(&BufferLin[posBufLin], 1, 128, hLin) != 128)
            {
                printf("ERROR: write to file '%s'\n", szFileLin);
                //return 0;
            }
            posBufLin += 128;
        }
        // завершаем сброс на диск
        posBufLin = 0;
    }
    BufferLin[posBufLin] = ch;
    posBufLin++;
    return 1;
}


/*****************************************************************************

 COM

 *****************************************************************************/
int com_Open()
{
    if (isMakeCOM)
    {
        strcpy(szFileCom, szFileName);
        strcat(szFileCom, ".com");
        maxPosBufCom = 2048;
        posBufCom = 0;
        hCom = fopen(szFileCom, "wb");
        if (hCom == NULL)
        {
            printf("ERROR: Can't create COM file '%s'\n", szFileCom);
            return 0;
        }
    } else
        hCom = NULL;
    return 1;
}

int com_PutByte(char ch)
{
    if (posBufCom >= maxPosBufCom)
    {
        // буфер полон, пора скидывать на диск
        posBufCom = 0;
        while (posBufCom < maxPosBufCom)
        {
            if (fwrite(&BufferCom[posBufCom], 1, 128, hCom) != 128)
            {
                printf("ERROR: write to file '%s'\n", szFileCom);
                return 0;
            }
            posBufCom += 128;
        }
        // завершаем сброс на диск
        posBufCom = 0;
    }
    BufferCom[posBufCom] = ch;
    posBufCom++;
    return 1;
}


/*****************************************************************************

 Парсер

 *****************************************************************************/

void put_Byte(char ch)
{
    if (isOutPrinter)
    {
        if (ch == 9)
        {
            do {
                printf(" ");
                prnLineCount++;

            } while ((prnLineCount & 7) != 0);
        } else {
            if (ch == 0x0A)
                prnLineCount = 0xFF;
            printf("%c", ch);
            prnLineCount++;
        }
    }
    // теперь выводим в файл или на экран
    switch (outDevType)
    {
        case OUT_SYM:
            if (isMakeSYM)
                sym_PutByte(ch);
            break;
        case OUT_SCR:
            printf("%c", ch);
            break;
        case OUT_LIN:
            if (isMakeLIN)
                lin_PutByte(ch);
            break;
    }
}

/*
  вывод байта в файл или экран, с кэшированием табуляций
*/
void out_Byte(char ch)
{
    if (ch == 9)
    {
        tabCount++;
        return;
    }
    if (ch < ' ')
    {
        tabCount = 0;
        put_Byte(ch);
        return;
    }
    while (tabCount)
    {
        put_Byte(9);
        tabCount--;
    }
    put_Byte(ch);
}


void out_Tab()
{
    out_Byte(9);
}

void out_Space()
{
    out_Byte(' ');
}


void out_CrLf()
{
    out_Byte(0x0D);
    out_Byte(0x0A);
}


void out_String(char *s)
{
    while (*s != 0)
    {
        out_Byte(*s);
        s++;
    }
}

/*
  вывод байта в символьном шестнадцатеричном виде на текущее устройство
*/
void out_HexByte(char ch)
{
    char buf[8];

    sprintf(&buf, "%02X", ch);
    out_Byte(buf[0]);
    out_Byte(buf[1]);
}


/*
  вывод слова в символьном шестнадцатеричном виде на текущее устройство
*/
void out_HexWord(unsigned int w)
{
    out_HexByte(w >> 8);
    out_HexByte(w);
}


void out_Decimal(unsigned int num)
{
    char buf[16];
    char *p;
    int len;

    sprintf(buf, "%-3u", num);
    len = strlen(buf);
    p = &buf;

    while (len)
    {
        out_Byte(*p);
        p++;
        len--;
    }
}


/*
int Parse()
{
    char ch;
    char crc;
    unsigned int len, tmp;
    char res;

    do {
        crc = 0;
        len = 0xffff;
        if ( (res = obj_GetByte(&ch, &crc, &len)) == 0 )
        {
            printf("found EOF1\n");
            return 0;
        }
        tmp = obj_GetWord(&crc, &len);
        len = tmp;
        //printf("type: 0x%02X\n", ch);
        //printf("size: %u\n", len & 0xFFFF);
        while (len)
        {
            if ( (res = obj_GetByte(&ch, &crc, &len)) == 0 )
            {
                printf("found EOF\n");
                return 0;
            }
        }
    } while (res);
    return -1;
}
*/

int load_MODHDR(char * crc, unsigned int *len)
{
    char ch;
    unsigned int tmp;
    char szName[16];
    char lenName;

    obj_GetByte(&lenName, crc, len);
    // считываем имя
    if (lenName >= 16)
        lenName = 16;
    for (tmp = 0; tmp < lenName; tmp++)
    {
        obj_GetByte(&ch, crc, len);
        szName[tmp] = ch;
    }
    // заголовок в SYM
    outDevType = OUT_SYM;
    if (recsInSYMline)
        out_CrLf();
    symTabCount = 0;
    out_HexWord(EmptyAddress);
    out_Space();
    for (tmp = 0; tmp < lenName; tmp++)
    {
        out_Byte(szName[tmp]);
    }
    out_CrLf();

    // заголовок в LIN
    outDevType = OUT_LIN;
    if (recsInLINline)
        out_CrLf();
    linTabCount = 0;
    out_HexWord(EmptyAddress);
    out_Space();
    for (tmp = 0; tmp < lenName; tmp++)
    {
        out_Byte(szName[tmp]);
    }
    out_Byte('#');
    out_CrLf();
    recsInSYMline = 0;
    if (szSecondParam[0] != ' ')
    {
        isSecondParam = 0;
        for (tmp = 0; tmp < lenName; tmp++)
        {
            if (szSecondParam[tmp] != szName[tmp])
                return -1;
        }
        isSecondParam = -1;
    }
    return -1;
}



int Parse()
{
    char ch;
    char crc;
    unsigned int len, tmp, adr;
    char res;

    BaseAddress = 0;
    StartingAddress = 0;
    EmptyAddress = 0;

    do {
        crc = 0;
        len = 0xffff;
        // читаем тип записи
        obj_GetByte(&ch, &crc, &len);
        // читаем длину записи
        if ((tmp = obj_GetWord(&crc, &len)) == 0)
            doExit("bad record!");
        len = tmp;

        // ch - символ
        // len - длина записи
        switch (ch)
        {
            case R_MODHDR:
            case R_ANCEST:
                load_MODHDR(&crc, &len);
                break;

            case R_LINNUM:
                if (isSecondParam != 0)
                {
                    outDevType = OUT_LIN;
                    tabCount = linTabCount;
                    obj_GetByte(&ch, &crc, &len);
                    // @@put_LineInfo:
                    while (len > 1)
                    {
                        if (recsInLINline >= 5)
                        {
                            out_CrLf();
                            recsInLINline = 0;
                        }
                        recsInLINline++;
                        tmp = obj_GetWord(&crc, &len);
                        out_HexWord(tmp);
                        out_Space();
                        tmp = obj_GetWord(&crc, &len);
                        out_Decimal(tmp);
                        out_Tab();
                    } // while (len != 1);
                    linTabCount = tabCount;
                }
                break;



            case R_LOCDEF:             /* выводим имя метки и её адрес в SYM */
                if (isSecondParam != 0)
                {
                    outDevType = OUT_SYM;
                    tabCount = symTabCount;
                    obj_GetByte(&ch, &crc, &len);
                    while (len > 1)
                    {
                        tmp = obj_GetWord(&crc, &len);
                        obj_GetByte(&res, &crc, &len);
                        if (recsInSYMline)
                        {
                            out_Byte(9);
                            recsInSYMline = (recsInSYMline & 0xF8) + 8;
                            if (recsInSYMline & 0x0F)
                            {
                                recsInSYMline += 8;
                                out_Byte(9);
                            }
                        }
                        if ((recsInSYMline + res + 5) >= 80)
                        {
                            out_CrLf();
                            recsInSYMline = 0;
                        }
                        // выводим адрес метки
                        out_HexWord(tmp);
                        out_Space();
                        recsInSYMline = recsInSYMline + res + 5;
                        // выводим имя метки
                        while (res)
                        {
                            obj_GetByte(&ch, &crc, &len);
                            out_Byte(ch);
                            res--;
                        }
                        obj_GetByte(&ch, &crc, &len);
                    } // while (len != 1);
                    symTabCount = tabCount;
                }
                break;


            case R_MODDAT:
                if (isMakeCOM)
                {
                    obj_GetByte(&ch, &crc, &len);
                    if (ch)
                        doExit("bad record!");
                    tmp = obj_GetWord(&crc, &len);
                    adr = tmp;
                    if (tmp+len >= EmptyAddress)
                        EmptyAddress = tmp+len;
                    if (byte_BD3 == 0)
                    {
                        byte_BD3 = -1;
                        if (isBinary)
                        {
                            BaseAddress = tmp;
                            word_BD1 = tmp;
                            if ((tmp & 0xFF) == 3)
                            {
                                printf("Insert jump to entry point\n");
                                com_PutByte(0xC3);
                                com_PutByte(0);
                                com_PutByte(0);
                            }
                        }
                    }
                    if (tmp >= BaseAddress)
                    {
                        // меняем местами BaseAddress и текущий адрес
                        tmp = BaseAddress;
                        BaseAddress = adr;
                    }
                    if (tmp < word_BD1)
                        doExit("bad record!");
                    while (tmp != word_BD1)
                    {
                        com_PutByte(0);
                        word_BD1++;
                    }
                    word_BD1 = EmptyAddress - 1;

                    do {
                        obj_GetByte(&ch, &crc, &len);
                        if (len == 0)
                            break;
                        com_PutByte(ch);
                    } while (1);
                }
                break;



            case R_MODEND:
                obj_GetByte(&ch, &crc, &len);
                if (ch == 1)
                {
                    obj_GetByte(&ch, &crc, &len);
                    StartingAddress = obj_GetWord(&crc, &len);
                    if (StartingAddress == 0)
                        doExit("bad module!");
                }
                break;

            case R_MODEOF:
                if (isMakeCOM)
                {
                    if (byte_BD3 == 0)
                        return 0;
                    else
                        return -1;
                }
                break;


            case R_PUBDEF:
            case R_EXTNAM:
            default:
                break;
        }
        // пропускаем остаток данных или неизвестные типы записей
        while (len)
        {
            obj_GetByte(&ch, &crc, &len);
        }
    } while (crc == 0);
    doExit("bad record CRC!");
    return 0;
}





void closeFiles()
{
    int res;

    if (isMakeSYM)
    {
        if (hSym)
        {

            // закрываем SYM файл
            do {
                res = posBufSym & 0x7F;
                if (res == 0)
                    maxPosBufSym = posBufSym; // достигли выравнивания по 128 байт, выходим
                sym_PutByte(0x1A);
            } while (res);
            fclose(hSym);
            hSym = NULL;
        }
    }
    if (isMakeLIN)
    {
        if (hLin)
        {
            // закрываем LIN файл
            do {
                res = posBufLin & 0x7F;
                if (res == 0)
                    maxPosBufLin = posBufLin; // достигли выравнивания по 128 байт, выходим
                lin_PutByte(0x1A);
            } while (res);
            fclose(hLin);
            hLin = NULL;
        }
    }
    if (isMakeCOM)
    {
        if (hCom)
        {
            // закрываем COM файл
            do {
                res = posBufCom & 0x7F;
                if (res == 0)
                    maxPosBufCom = posBufCom; // достигли выравнивания по 128 байт, выходим
                com_PutByte(0x1A);
            } while (res);
            fclose(hCom);
            hCom = NULL;
        }
    }
}





int Start()
{
    int res, ret;

    if (!obj_Open())
        return 1;
    if (!sym_Open())
        return 1;
    if (!lin_Open())
        return 1;
    if (!com_Open())
        return 1;

    recsInSYMline = 0;
    prnLineCount = 0;
    symTabCount = 0;
    linTabCount = 0;
    recsInLINline = 0;
    byte_BD3 = 0;

    ret = Parse();
    fclose(hObj);

    outDevType = OUT_SYM;
    out_CrLf();
    outDevType = OUT_LIN;
    out_CrLf();
    outDevType = OUT_SCR;
    if (res == 0)
        out_String("ERROR: Bad load module.\n");

    // закрываем файлы
    closeFiles();
    // корректируем переход на точку входа в программу
    if (isMakeCOM)
    {
        if ((BaseAddress & 0xFF) == 3)
        {
            printf("Correct jump to entry point\n");
            hCom = fopen(szFileCom, "r+b");
            fseek(hCom, 0, SEEK_SET);
            res = fread(&BufferCom, 1, 3, hCom);
            if (res == 3)
            {
                fseek(hCom, 0, SEEK_SET);
                BufferCom[1] = StartingAddress & 0xFF;
                BufferCom[2] = StartingAddress >> 8;
                if (fwrite(&BufferCom, 1, 3, hCom) != 3)
                {
                    printf("ERROR: write to disk.\n");
                }
            }
            fclose(hCom);
        }
    }

    // выводим информацию
    out_CrLf();
    out_HexWord(BaseAddress);
    out_String(" = BASE ADDRESS");
    out_CrLf();
    out_HexWord(StartingAddress);
    out_String(" = STARTING ADDRESS");
    out_CrLf();
    out_HexWord(EmptyAddress);
    out_String(" = NEXT EMPTY ADDRESS");
    out_CrLf();

    return ret;
}




int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        doHelp();
        return 1;
    }
    getSecondParam(argc, argv);
    parseParam(argc, argv);

    return Start();
}
