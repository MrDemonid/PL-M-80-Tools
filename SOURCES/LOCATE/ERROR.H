/****************************************************************************
 *  Locate v 4.0                                                            *
 *  Copyright (C) 2023 Andrey Hlus                                          *
 *                                                                          *
 *  Created based on:                                                       *
 *  C port of Intel's Locate v3.0                                           *
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


#pragma once
/* ISIS error codes, based on
*       <http://www.bitsavers.org/pdf/intel/ISIS_II/ISIS_internals.pdf>:
*/

#define ERROR_SUCCESS      0x00 /* Operation succeeded */
#define ERROR_NOMEM        0x01 /* No memory available for buffer */
#define ERROR_NOTOPEN      0x02 /* File is not open */
#define ERROR_NOHANDLES    0x03 /* No more file handles available */
#define ERROR_BADFILENAME  0x04 /* Invalid pathname */
#define ERROR_BADDEVICE    0x05 /* Bad device name in filename */
#define ERROR_NOWRITE      0x06 /* Trying to write a file open for read */
#define ERROR_DISKFULL     0x07 /* Cannot write, disk full */
#define ERROR_NOREAD       0x08 /* Trying to read a file open for write */
#define ERROR_DIRFULL      0x09 /* Cannot create file: Directory full */
#define ERROR_RENACROSS    0x0A /* Cannot rename across devices */
#define ERROR_EXISTS       0x0B /* Cannot rename: other file already exists. */
#define ERROR_ALREADYOPEN  0x0C /* File already open */
#define ERROR_FILENOTFOUND 0x0D /* File not found */
#define ERROR_PERMISSIONS  0x0E /* File is write protected */
#define ERROR_OVERWRITING  0x0F /* Attempt to overwrite ISIS-II */
#define ERROR_BADIMAGE     0x10 /* Invalid executable image format */
#define ERROR_ISDEVICE     0x11 /* Attempt to rename/delete a device */
#define ERROR_BADFUNCTION  0x12 /* Invalid function number */
#define ERROR_CANTSEEKDEV  0x13 /* Can't seek on a device */
#define ERROR_OFFBEGINNING 0x14 /* Can't seek to before beginning of file */
#define ERROR_NOTLINEMODE  0x15 /* Can't rescan a file not opened in line mode*/
#define ERROR_BADACCESS    0x16 /* Bad access mode */
#define ERROR_NOFILENAME   0x17 /* No filename */
#define ERROR_DISKERROR    0x18 /* Disk I/O error */
#define ERROR_BADECHOFILE  0x19 /* Invalid echo file in OPEN */
#define ERROR_BADATTRIB    0x1A /* Incorrect attribute in ATTRIB */
#define ERROR_BADMODE      0x1B /* Incorrect mode in SEEK */
#define ERROR_BADEXT       0x1C /* Bad file extension */
#define ERROR_EOF          0x1D /* End of file on console input */
#define ERROR_NOTREADY     0x1E /* Drive not ready */
#define ERROR_SEEKWRITE    0x1F /* Can't seek on a write-only file */
#define ERROR_FILEINUSE    0x20 /* Can't delete an open file */
#define ERROR_BADPARAM     0x21 /* Bad system call parameter */
#define ERROR_BADLOADSW    0x22 /* Invalid nswitch argument to LOAD */
#define ERROR_SEEKPASTEOF  0x23 /* Seek past EOF on file open for read */
