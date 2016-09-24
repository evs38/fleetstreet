/* INSTALL.H */

#define IDIC_MAIN       1
#define IDBM_MAIN       10

#define IDST_MSG_CREATEDIR         20
#define IDST_MSG_NOSCRIPT          21
#define IDST_MSG_INVALID           22
#define IDST_MSG_FILEWRITEERR      23
#define IDST_MSG_FILREADERR        24
#define IDST_MSG_SCRIPTERR         25
#define IDST_MSG_NOFIL             26
#define IDST_MSG_FILERROR          27
#define IDST_MSG_READMENOTFOUND    28
#define IDST_MSG_SCRIPTNOMATCH     29
#define IDST_MSG_NODIRENTERED      30
#define IDST_MSG_NOLANGSELECTED    31

#define IDST_STATUS_READFILES      51
#define IDST_STATUS_READREADME     52
#define IDST_STATUS_COPYING        53
#define IDST_STATUS_CREATEDIR      54
#define IDST_STATUS_COPYLANG       55
#define IDST_STATUS_CREATEOBJ      57
#define IDST_STATUS_DONE           58
#define IDST_STATUS_DOINSTALL      59
#define IDST_STATUS_INSTALLABORTED 60
#define IDST_STATUS_SCRIPTS        61
#define IDST_STATUS_OBSFILES       62

#define IDD_MAIN        100
#define IDD_README      200
#define IDD_DIRSELECT   300
#define IDD_ERROR       400

#define WM_FILNOTFOUND             (WM_USER+4)
#define WM_FILERROR                (WM_USER+5)
#define WM_FILESREADY              (WM_USER+7)
#define WM_READMEREAD              (WM_USER+8)
#define WM_READMENOTFOUND          (WM_USER+9)
#define WM_INSTALLABORTED          (WM_USER+10)
#define WM_CREATEDIR               (WM_USER+11)
#define WM_INSTALLDONE             (WM_USER+12)
#define WM_DIRERROR                (WM_USER+13)
#define WM_INSTALLFILE             (WM_USER+14)
#define WM_INSTALLLANG             (WM_USER+15)
#define WM_INSTALLSCR              (WM_USER+16)
#define WM_CREATEOBJ               (WM_USER+17)
#define WM_DELOBSFILES             (WM_USER+18)

