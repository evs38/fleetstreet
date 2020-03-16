/***************************************************************************
 *                                                                         *
 *  MSGAPI Source Code, Version 2.00                                       *
 *  Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.          *
 *                                                                         *
 *  Parse network address                                                  *
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

/* $Id: parsenn.c_v 1.0 1991/11/16 16:16:40 sjd Rel sjd $ */

#define NOVARS
#define NOVER
#define MAX_NOPROTO

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "prog.h"
#include "max.h"

static char *colon=":";
static char *slash="/";

void _fast Parse_NetNode(char *netnode,word *zone,word *net,word *node,word *point)
{
  ParseNN(netnode,zone,net,node,point,FALSE);
}

void _fast ParseNN(char *netnode,word *zone,word *net,word *node,word *point,word all)
{
  char *p;

  p=netnode;
  
  if (all && point)
    *point=POINT_ALL;

  if (all && toupper(*netnode)=='W')  /* World */
  {
    if (zone)
      *zone=ZONE_ALL;

    if (net)
      *net=NET_ALL;

    if (node)
      *node=NODE_ALL;

    return;
  }

  /* If we have a zone (and the caller wants the zone to be passed back).. */

  if (strchr(netnode,':'))
  {
    if (zone)
    {
      if (all && toupper(*p)=='A')  /* All */
        *zone=ZONE_ALL;
      else *zone=atoi(p);
    }

    p=firstchar(p,colon,2);
  }

  /* If we have a net number... */

  if (p && *p)
  {
    if (strchr(netnode,'/'))
    {
      if (net)
      {
        if (all && toupper(*p)=='A')  /* All */
          *net=NET_ALL;
        else *net=atoi(p);
      }

      p=firstchar(p,slash,2);
    }
    else if (all && toupper(*p)=='A')
    {
      /* If it's in the form "1:All" or "All" */

      if (strchr(netnode,':')==NULL && zone)
        *zone=ZONE_ALL;

      *net=NET_ALL;
      *node=NODE_ALL;
      p += 3;
    }
  }

  /* If we got a node number... */

  if (p && *p && node && *netnode != '.')
  {
    if (all && toupper(*p)=='A')  /* All */
    {
      *node=NODE_ALL;

      /* 1:249/All implies 1:249/All.All too... */

      if (point && all)
        *point=POINT_ALL;
    }
    else *node=atoi(p);
  }

  if (p)
    while (*p && isdigit(*p))
      p++;

  /* And finally check for a point number... */

  if (p && *p=='.')
  {
    p++;

    if (point)
    {
      if (!p && *netnode=='.')
        p=netnode+1;

      if (p && *p)
      {
        *point=atoi(p);

        if (all && toupper(*p)=='A')  /* All */
          *point=POINT_ALL;
      }
      else *point=0;
    }
  }
}

