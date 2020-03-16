/***************************************************************************
 *                                                                         *
 *  MSGAPI Source Code, Version 2.00                                       *
 *  Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.          *
 *                                                                         *
 *  MsgAPI main module                                                     *
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

/* $Id: msgapi.c_v 1.0 1991/11/16 16:16:40 sjd Rel sjd $ */

#define MSGAPI_PROC

#include <string.h>
#include <stdlib.h>
#include "alc.h"
#include "prog.h"
#include "max.h"
#include "msgapi.h"
#include "apidebug.h"



static byte *intl="INTL";
static byte *fmpt="FMPT";
static byte *topt="TOPT";
static byte *area_colon="AREA:";

static char *copyright="MSGAPI - Copyright 1991 by Scott J. Dudley.  All rights reserved.";

word _stdc msgapierr=0;  /* Global error value for message API routines */
struct _minf _stdc mi;

sword EXPENTRY MsgOpenApi(struct _minf *minf)
{
  NW(copyright);

  mi=*minf;
  mi.haveshare=shareloaded();
  return 0;
}

sword EXPENTRY MsgCloseApi(void)
{
  return 0;
}

MSG * EXPENTRY MsgOpenArea(byte *name,word mode,word type)
{
  if (type & MSGTYPE_SQUISH)
    return (SquishOpenArea(name,mode,type));
  else return (SdmOpenArea(name,mode,type));
}

sword EXPENTRY MsgValidate(word type,byte *name)
{
  if (type & MSGTYPE_SQUISH)
    return (SquishValidate(name));
  else /*if (type==MSGTYPE_SDM)*/
    return (SdmValidate(name));
/*  else return FALSE;*/
}


/* Check to see if a message handle is valid.  This function should work    *
 * for ALL handlers tied into MsgAPI.  This also checks to make sure that   *
 * the area which the message is from is also valid.  (ie. The message      *
 * handle isn't valid, unless the area handle of that message is also       *
 * valid.)                                                                  */

sword MSGAPI InvalidMsgh(MSGH *msgh)
{
  if (msgh==NULL || msgh->id != MSGH_ID || InvalidMh(msgh->sq))
  {
    msgapierr=MERR_BADH;
    return TRUE;
  }

  return FALSE;
}

/* Check to ensure that a message area handle is valid.                     */

sword MSGAPI InvalidMh(MSG *mh)
{
  if (mh==NULL || mh->id != MSGAPI_ID)
  {
    msgapierr=MERR_BADH;
    return TRUE;
  }

  return FALSE;
}


byte * StripNasties(byte *str)
{
  byte *s;

  for (s=str; *s; s++)
    if (*s < ' ')
      *s=' ';

  return str;
}


#ifdef NEVER /* notused */

byte * MSGAPI cut_ansi(byte *s)
{
  byte *orig;
  
  for (orig=s;*s;s++)
    if (*s=='\x1b')
      *s=' ';
    
  return orig;
}

#endif


/* Copy the text itself to a buffer, or count its length if out==NULL */

static word near _CopyToBuf(byte *p, byte *out, byte **end)
{
  word len=0;

  if (out)
    *out++='\x01';

  len++;


  for (; *p=='\x0d' || *p=='\x0a' || *p==(byte)'\x8d'; p++)
    ;

  while (*p=='\x01' || strncmp(p, area_colon, 5)==0)
  {
    /* Skip over the first ^a */

    if (*p=='\x01')
      p++;

    while (*p && *p != '\x0d' && *p != '\x0a' && *p != (byte)'\x8d')
    {
      if (out)
        *out++=*p;
      
      p++;
      len++;
    }

    if (out)
      *out++='\x01';

    len++;

    while (*p=='\x0d' || *p=='\x0a' || *p==(byte)'\x8d')
      p++;
  }

  /* Cap the string */

  if (out)
    *out='\0';

  len++;
  

  /* Make sure to leave no trailing x01's. */

  if (out && out[-1]=='\x01')
    out[-1]='\0';
  

  /* Now store the new end location of the kludge lines */

  if (end)
    *end=p;
  
  return len;
}



byte * EXPENTRY CopyToControlBuf(byte *txt, byte **newtext, unsigned *length)
{
  byte *cbuf, *end;

  word clen;


  /* Figure out how long the control info is */

  clen=_CopyToBuf(txt, NULL, NULL);

  /* Allocate memory for it */
  
  #define SAFE_CLEN 20

  if ((cbuf=palloc(clen+SAFE_CLEN))==NULL)
    return NULL;

  memset(cbuf, '\0', clen+SAFE_CLEN);

  /* Now copy the text itself */

  clen=_CopyToBuf(txt, cbuf, &end);

  if (length)
    *length -= (size_t)(end-txt);

  if (newtext)
    *newtext=end;

  return cbuf;
}


byte * EXPENTRY GetCtrlToken(byte *where, byte *what)
{
  byte *end, *found, *out;

  if (where && (found=strstr(where, what)) != NULL && found[-1]=='\x01')
  {
    end=strchr(found,'\x01');

    if (!end)
      end=found+strlen(found);

    if ((out=palloc((size_t)(end-found)+1))==NULL)
      return NULL;

    memmove(out,found,(size_t)(end-found));
    out[(size_t)(end-found)]='\0';
    return out;
  }

  return NULL;
}



void EXPENTRY ConvertControlInfo(byte *ctrl,NETADDR *orig,NETADDR *dest)
{
  byte *p, *s;
  
  if ((p=s=GetCtrlToken(ctrl, intl)) != NULL)
  {
    NETADDR norig, ndest;

    /* Copy the defaults from the original address */

    norig=*orig;
    ndest=*dest;

    /* Parse the destination part of the kludge */

    s += 5;
    Parse_NetNode(s, &ndest.zone, &ndest.net, &ndest.node, &ndest.point);
    
    while (*s != ' ' && *s)
      s++;

    if (*s)
      s++;

    Parse_NetNode(s, &norig.zone, &norig.net, &norig.node, &norig.point);

    pfree(p);

    /* Only use this as the "real" zonegate address if the net/node         *
     * addresses in the INTL line match those in the message                *
     * body.  Otherwise, it's probably a gaterouted message!                */

    if (ndest.net==dest->net && ndest.node==dest->node &&
        norig.net==orig->net && norig.node==orig->node)
    {
      *dest=ndest;
      *orig=norig;

      /* Only remove the INTL line if it's not gaterouted, which is         *
       * why we do it here.                                                 */
      
      RemoveFromCtrl(ctrl,intl);
    }
  }


  /* Handle the FMPT kludge */
  
  if ((s=GetCtrlToken(ctrl,fmpt)) != NULL)
  {
    orig->point=atoi(s+5);
    pfree(s);

    RemoveFromCtrl(ctrl,fmpt);
  }

  
  /* Handle TOPT too */
  
  if ((s=GetCtrlToken(ctrl,topt)) != NULL)
  {
    dest->point=atoi(s+5);
    pfree(s);

    RemoveFromCtrl(ctrl,topt);
  }
}


  
byte * EXPENTRY CvtCtrlToKludge(byte *ctrl)
{
  byte *from;
  byte *buf;
  byte *to;
  size_t clen;
  
  clen=strlen(ctrl) + NumKludges(ctrl) + 20;
  
  if ((buf=palloc(clen))==NULL)
    return NULL;
  
  to=buf;

  /* Convert ^aKLUDGE^aKLUDGE... into ^aKLUDGE\r^aKLUDGE\r... */
  
  for (from=ctrl; *from=='\x01' && from[1];)
  {
    /* Only copy out the ^a if it's NOT the area: line */

    if (!eqstrn(from+1, area_colon, 5))
      *to++=*from;

    from++;
    
    while (*from && *from != '\x01')
      *to++=*from++;

    *to++='\r';
  }
  
  *to='\0';

  return buf;
}



    
void EXPENTRY RemoveFromCtrl(byte *ctrl,byte *what)
{
  byte *search;
  byte *p, *s;
  
  if ((search=palloc(strlen(what)+2))==NULL)
    return;
  
  strcpy(search, "\x01");
  strcat(search, what);
  
  /* Now search for this token in the control buffer, and remove it. */
    
  while ((p=strstr(ctrl, search)) != NULL)
  {
    for (s=p+1; *s && *s != '\x01'; s++)
      ;
      
    strocpy(p, s);
  }
  
  pfree(search);
}


word EXPENTRY NumKludges(char *txt)
{
  word nk=0;
  char *p;
  
  for (p=txt; (p=strchr(p, '\x01')) != NULL; p++)
    nk++;
    
  return nk;
}

