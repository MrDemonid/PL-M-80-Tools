#ifndef _OBJCPM_H_
#define _OBJCPM_H_

typedef struct {
    unsigned char deviceId;
    unsigned char name[6];
    unsigned char ext[3];
    unsigned char deviceType;
    unsigned char driveType;
} spath_t;

typedef struct {
    unsigned char deviceId; // isis device Id
    unsigned char modes;    // supported modes READ_MODE, WRITE_MODE, UPDATE_MODE and RANDOM_ACCESS
    unsigned char name[_MAX_PATH];
} osfile_t;

#define ERROR_SUCCESS      0x00 /* Operation succeeded */
#define ERROR_BADFILENAME  0x04 /* Invalid pathname */
#define ERROR_BADDEVICE    0x05 /* Bad device name in filename */
#define ERROR_BADEXT       0x1C /* Bad file extension */
#define ERROR_NOTREADY     0x1E /* Drive not ready */


/* file open modes */
#define READ_MODE   1
#define WRITE_MODE  2
#define UPDATE_MODE 3
#define RANDOM_ACCESS   4   // additional access mode


#endif
