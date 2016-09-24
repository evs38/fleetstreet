/* COMMON.H */

#define STRUCT_MAJ_REV4   4
#define STRUCT_MIN_REV4   1

#define STRUCT_MAJ_REV5   5
#define STRUCT_MIN_REV5   9
#define STRUCT_MIN_REV13  13
#define STRUCT_MIN_REV14  14

#define STRUCT_MAJ_REV6   6

/* Anfang der Struktur, m. Versionskennung */

#pragma pack(1)
typedef struct
{
   unsigned char im_ver_maj;
   unsigned char im_ver_min;
   unsigned char struct_maj;
   unsigned char struct_min;
} IMAIL_VER;
#pragma pack()

void AddNetmailArea(PAREALIST pRetList, char *pchAreaPath, PFTNADDRESS pAddress, char *pchUsername);

/* Ende COMMON.H */
