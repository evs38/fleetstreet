#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prog.h"
#include "alc.h"
#include "msgapi.h"


#define BUFSIZE 4096

int _stdc main(int argc,char *argv[])
{
  XMSG msg;
  MSG *in_area, *out_area;
  MSGH *in_msg, *out_msg;
  char *ctrl, *buffer;
  dword offset, msgn;
  long got;
  int t1, t2;
  int ctrllen;
  struct _minf mi;

  if (argc < 6)
  {
    printf("format:  SQCONVERT <from_name> <from_type>  <to_name> <to_type> <default_zone>\n");
    printf("example: SQCONVERT \\msg\\foo     *.msg     \\msg\\foosq  squish       1\n");
    exit(1);
  }

  printf("Converting area %s...\n",argv[1]);

  if (eqstri(argv[2],"*.msg"))
    t1=MSGTYPE_SDM;
  else if (eqstri(argv[2],"squish"))
    t1=MSGTYPE_SQUISH;
  else t1=atoi(argv[2]);

  mi.def_zone=atoi(argv[5]);

  MsgOpenApi(&mi);

  if ((in_area=MsgOpenArea(argv[1], MSGAREA_NORMAL, t1))==NULL)
  {
    printf("Error opening area `%s' (type %d) for read!\n",
           argv[1], t1);
    exit(1);
  }

  MsgLock(in_area);

  if (eqstri(argv[4],"*.msg"))
    t2=MSGTYPE_SDM;
  else if (eqstri(argv[4],"squish"))
    t2=MSGTYPE_SQUISH;
  else t2=atoi(argv[4]);

  if ((out_area=MsgOpenArea(argv[3], MSGAREA_CRIFNEC, t2))==NULL)
  {
    printf("Error opening area `%s' (type %d) for write!\n",
           argv[3], t2);
    exit(1);
  }

  MsgLock(out_area);

  if ((buffer=malloc(BUFSIZE))==NULL)
  {
    printf("Error!  Ran out of memory...\n");
    exit(1);
  }

  for (msgn=1L; msgn <= MsgHighMsg(in_area); msgn++)
  {
    if ((msgn % 5)==0)
      printf("Msg: %ld\r",msgn);

    if ((in_msg=MsgOpenMsg(in_area,MOPEN_READ,msgn))==NULL)
      continue;

    if ((out_msg=MsgOpenMsg(out_area, MOPEN_CREATE, 0L))==NULL)
    {
      printf("Error writing to output area; msg#%ld (%d).\n", msgn, msgapierr);
      MsgCloseMsg(in_msg);
      continue;
    }

    ctrllen=(int)MsgGetCtrlLen(in_msg);

    if ((ctrl=malloc(ctrllen))==NULL)
      ctrllen=0;

    MsgReadMsg(in_msg, &msg, 0L, 0L, NULL, ctrllen, ctrl);

    msg.attr |= MSGSCANNED;

    msg.replyto=0L;
    memset(msg.replies, '\0', sizeof(msg.replies));

    MsgWriteMsg(out_msg, FALSE, &msg, NULL, 0L,
                MsgGetTextLen(in_msg), ctrllen, ctrl);

    for (offset=0L; offset < MsgGetTextLen(in_msg);)
    {
      got=MsgReadMsg(in_msg, NULL, offset, BUFSIZE, buffer, 0L, NULL);

      if (got <= 0)
        break;

      MsgWriteMsg(out_msg, TRUE, NULL, buffer, got,
                  MsgGetTextLen(in_msg), 0L, NULL);

      offset += got;
    }

    if (ctrl)
      free(ctrl);

    MsgCloseMsg(out_msg);
    MsgCloseMsg(in_msg);
  }

  free(buffer);
  MsgCloseArea(out_area);
  MsgCloseArea(in_area);
  MsgCloseApi();

  printf("\nDone!\n");
  return 0;
}

