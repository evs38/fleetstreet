/* FOLDERMAN.H */

/* Strukturen */

#pragma pack(4)
typedef struct folderdata
{
   struct folderdata *next;
   struct folderdata *prev;
   BOOL bDirty;

   LONG  FolderID;               /* 0 = root */
   PCHAR pchName;
   LONG  ParentFolder;
   ULONG ulFlags;
} AREAFOLDER, *PAREAFOLDER;

#define FOLDERID_ALL               (-1L)

#define FOLDER_SORT_UNSORTED       0x00UL
#define FOLDER_SORT_NAME           0x01UL
#define FOLDER_SORT_UNREAD         0x02UL
#define FOLDER_SORT_MASK           0x03UL

#define FOLDER_EXPANDED            0x04UL
#define FOLDER_AUTOSCAN            0x08UL

typedef struct
{
   PAREAFOLDER pList;
   PAREAFOLDER pListLast;
   BOOL bDirty;

   LONG  LastFolder;
   LONG  HighID;
   ULONG ulFlags;
   LONG  lSplit;
   ULONG ulNumFolders;
} FOLDERANCHOR, *PFOLDERANCHOR;

#define AREAFOLDERS_LARGEICONS   0x00UL
#define AREAFOLDERS_SMALLICONS   0x01UL
#define AREAFOLDERS_NOICONS      0x02UL

#define AREAFOLDERS_ICONMASK     0x03UL

#pragma pack()

/* Prototypen */

PAREAFOLDER FM_AddFolder(PFOLDERANCHOR pAnchor, PAREAFOLDER pNewFolder, ULONG ulOptions);
PAREAFOLDER FM_FindFolder(PFOLDERANCHOR pAnchor, LONG FolderID);
PAREAFOLDER FM_FindFolderWithParent(PFOLDERANCHOR pAnchor, PAREAFOLDER pLast, LONG ParentID);
BOOL        FM_DeleteFolder(PFOLDERANCHOR pAnchor, LONG FolderID);
BOOL        FM_DeleteFolderDirect(PFOLDERANCHOR pAnchor, PAREAFOLDER pFolder);

#define ADDFOLDER_HEAD        0UL
#define ADDFOLDER_TAIL        1UL
#define ADDFOLDER_NEWID       4UL
#define ADDFOLDER_MARKDIRTY   8UL

/* Ende FOLDERMAN.H */
