/***************************************************************************
 *                                                                         *
 *  MSGAPI Source Code, Version 2.00                                       *
 *  Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.          *
 *                                                                         *
 *  PROG.H prototypes                                                      *
 *                                                                         *
 *  For complete details of the licensing restrictions, please refer to    *
 *  the licence agreement, which is published in its entirety in           *
 *  README.1ST.                                                            *
 *                                                                         *
 *  USE OF THIS FILE IS SUBJECT TO THE RESTRICTIONS CONTAINED IN THE       *
 *  MSGAPI LICENSING AGREEMENT.  IF YOU DO NOT FIND THE TEXT OF THIS       *
 *  AGREEMENT IN ANY OF THE AFOREMENTIONED FILES, OR IF YOU DO NOT HAVE    *
 *  THESE FILES, YOU SHOULD IMMEDIATELY CONTACT THE AUTHOR AT ONE OF THE   *
 *  ADDRESSES LISTED BELOW.  IN NO EVENT SHOULD YOU PROCEED TO USE THIS    *
 *  FILE WITHOUT HAVING ACCEPTED THE TERMS OF THE MSGAPI LICENSING         *
 *  AGREEMENT, OR SUCH OTHER AGREEMENT AS YOU ARE ABLE TO REACH WITH THE   *
 *  AUTHOR.                                                                *
 *                                                                         *
 *  You can contact the author at one of the address listed below:         *
 *                                                                         *
 *  Scott Dudley           FidoNet  1:249/106                              *
 *  777 Downing St.        Internet f106.n249.z1.fidonet.org               *
 *  Kingston, Ont.         BBS      (613) 389-8315   HST/14.4k, 24hrs      *
 *  Canada - K7M 5N3                                                       *
 *                                                                         *
 ***************************************************************************/

/* $Id: progprot.h_v 1.0 1991/11/16 16:16:51 sjd Rel sjd $ */

void pascal far flush_handle2(int fd);
void _fast flush_handle(FILE *fp);
int _fast Save_Dir(char *orig_disk,char *orig_path[],char *path);
void _fast Restore_Dir(char *orig_disk,char *orig_path[]);
char * _fast Area_Name(int area);
int _fast Set_Area_Name(char *aname);
char * _fast Priv_Level(int priv);
int _fast fexist(char *filename);
long _fast fsize(char *filename);
int _fast lcopy(char *fromfile,char *tofile);
int _stdc hprintf(int handle,char *format,...);
long _stdc zfree(char *drive);
int _fast getword(char *strng,char *dest,char *delim,int findword);
int _fast getwordq(char *strng,char *dest,char *delim,char quote,int findword);
byte * _fast avt2ansi(sword new, sword old, char *ansi);
char * _stdc fancy_str(char *value);
char * _fast soundex(char *string);
int _fast any2dec(char *str,unsigned int radix);
char * _fast dec2any(unsigned int number,unsigned int radix);
int _fast isleap(int year);
char * _fast strrcat(char *string1,char *string2);
char * _fast make_fullfname(char *path);
char * _fast firstchar(char *strng,char *delim,int findword);
int _stdc xprintf(char *format,...);
void _stdc xputs(char *s);
void _stdc xputch(char ch);
void pascal xputchr(char ch);
void _stdc putss(char *string);
char * _fast stristr(char *string,char *search);
int _fast stristrm(char *string,char *search[],int stopbit);
int _fast stricmpm(char *string,char *search[],int stopbit);
long _fast stristrml(char *string,char *search[],long stopbit);
int _fast nullptr(void);
long _stdc timerset(unsigned int duration);
int _stdc timeup(long timer);
int _fast sbsearch(char *key,char *base[],unsigned int num);
char * _fast memstr(char *string,char *search,unsigned lenstring,unsigned strlen_search);
void _fast ddos_timer(unsigned int duration);
void _fast ddos_priority(int value);
void _fast ddos_switch(void);
void _fast ddos_suspend(void);
void _fast ddos_resume(void);
void _fast ddos_kill(void);
void _fast ddos_clear_vkb(void);
void _fast ddos_send(char ch);
void _fast ddos_addkey(char ch);
void _fast ddos_key_disable(void);
void _fast ddos_key_enable(void);
int  _fast ddos_herestat(void);
int  _fast ddos_flipstat(void);
void _fast ddos_funcs_enable(void);
void _fast ddos_funcs_disable(void);
void _fast brktrap(void);
void _stdc  brkuntrap(void); /* cdecl because of atexit() */
void _fast qksort(int a[],size_t n);
int  _fast direxist(char *directory);

int _fast get_fdt(int handle, union stamp_combo *filestamp);
int _fast set_fdt(int handle, union stamp_combo *filestamp);

int _fast get_disk(void);
int _fast set_disk(int drive);
int pascal far ddos_sleep(void);
int pascal far desq_sleep(void);
int pascal far pcmos_sleep(void);
int _fast zeller(int m,int d,int y);
void _fast iqsort(char *base, unsigned int nel, unsigned int inwidth, int (_stdc *comp)(void *,void *));
void _fast colour_to_string(int col,char *s);
int _fast make_dir(char *dir);
FILE * _fast shfopen(char *name,char *fpmode,int fdmode);;
unsigned int cdecl Get_CPU_Type(void);
int _fast do_tune(FILE *tunefile,int (_stdc *chkfunc)(void),int dv);
int _fast play_tune(char *filespec,char *name,int (_stdc *chkfunc)(void),int dv);
void _fast tdelay(int msecs);
void _fast dv_noise(int freq,int duration);
void _fast noise(int freq,int duration);
void _fast ASCII_Date_To_Binary(char *msgdate,union stamp_combo *d_written);
union stamp_combo * _fast Get_Dos_Date(union stamp_combo *st);
struct tm * _fast DosDate_to_TmDate(union stamp_combo *dosdate,
                                     struct tm *tmdate);
union stamp_combo * _fast TmDate_to_DosDate(struct tm *tmdate,
                                             union stamp_combo *dosdate);

char * _fast Strip_Trailing(char *str,char strip);
char * _fast Add_Trailing(char *str,char add);
void _fast Parse_NetNode(char *netnode,word *zone,word *net,word *node,word *point);
void _fast ParseNN(char *netnode,word *zone,word *net,word *node,word *point,word all);
void _fast c_encode(char *str,char *iarray,int len,int key);
void _fast c_decode(char *iarray,char *str,int key);
char * _fast sc_time(union stamp_combo *sc,char *string);
unsigned long _fast ieee_to_long(unsigned long f);
unsigned long _fast long_to_ieee(unsigned long l);
int _fast ieee_to_msbin(void *source,void *dest);
int _fast msbin_to_ieee(void *source,void *dest);
int _stdc AreaNameComp(byte *a1,byte *a2);
void * _fast smalloc(unsigned size);
char * _fast sstrdup(char *s);
char * _fast strocpy(char *d, char *s);
int pascal kgetch(void);
int pascal kpeek(void);
int pascal khit(void);
int _fast cshopen(const char *path, int access);
dword _fast crc32fn(word ch, dword crc);
dword * _fast mkcrc32tab(void);
word _fast crc16fn(word ch, word crc);
word * _fast mkcrc16tab(void);
char * _fast strrstr(char *str, char *delim);
FILE * _fast sfopen(char *name, char *fpmode, int fdmode, int access);
int _fast FileDate(char *name, union stamp_combo *sc);
void * _fast qsortl(void *list, void *(_stdc *getnext)(void *),
                    void (_stdc *setnext)(void *, void *),
                    int (_stdc *compare)(void *, void *));
int _fast GEdate(union stamp_combo *s1,union stamp_combo *s2);
void _fast install_24(void);
void _stdc uninstall_24(void);
sword _fast uniqrename(char *from, char *toorig);
word pascal getcpu(void);
word pascal getfpu(void);
int _fast SetFileDate(char *name, union stamp_combo *sc);
int setfsize(int fd, long size);

