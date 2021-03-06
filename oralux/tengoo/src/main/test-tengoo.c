/* 
----------------------------------------------------------------------------
test-tengoo.c
$Id: test-tengoo.c,v 1.5 2005/09/30 23:27:49 gcasse Exp $
$Author: gcasse $
Description: test Tengoo.
$Date: 2005/09/30 23:27:49 $ |
$Revision: 1.5 $ |
Copyright (C) 2005 Gilles Casse (gcasse@oralux.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
----------------------------------------------------------------------------
*/

/* This code comes from Yasr */

#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
/* #include <errno.h> */
#include <pty.h>  /* for openpty and forkpty */
#include <utmp.h> /* for login_tty */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* < Tengoo*/
#include "tengoo.h"
#include "debug.h"
static void* myTengoo=NULL;
/* > */

extern char **environ;

#define CHARSIZE sizeof(chartyp)

static struct termios t;

typedef unsigned char uchar;

static int cpid;
static int size;
static int master, slave;

/* < Tengoo */

/* unsigned char buf[256]; */
#define MAX_BUF_SIZE 10240 /* bigger to receive an enriched output */
unsigned char buf[MAX_BUF_SIZE];

/* > */

#define OPT_STR_SIZE 80
char usershell[OPT_STR_SIZE];
static int shell = 0;
static char **subprog = NULL;	/* if non-NULL, then exec it instead of shell */

static void yasr_ttyname_r(int fd, char *p, int size)
{
  char *t;

  ENTER("yasr_ttyname_r");

  if ((t = ttyname(fd)) == NULL) strcpy(p, "stdin");
  else (void) strncpy(p, t, size);
}

/* get the appropriate name for a tty from the filename */

static void rnget(char *s, char *d)
{
  (void) strcpy(d, s + 5);
}

static void utmpconv(char *s, char *d, int pid)
{
#ifdef UTMP_HACK
#ifdef sun
  struct utmpx *up;
  char *rs = (char *) buf, *rd = (char *) buf + 64;
  char *rstail = NULL, *rdtail = NULL;

  ENTER("utmpconv");

  rnget(s, rs);
  rnget(d, rd);

  if (rs)
  {
    if (strstr(rs, "/dev/") != NULL)
    {
      rstail = strdup(rs + sizeof("/dev/") - 1);
    }
    else
    {
      rstail = strdup(rs);
    }
  }

  if (rd)
  {
    if (strstr(rd, "/dev/") != NULL)
    {
      rdtail = strdup(rd + sizeof("/dev/") - 1);
    }
    else
    {
      rdtail = strdup(rd);
    }
  }

  setutxent();
  while ((up = getutxent()) != NULL)
  {
    if (!strcmp(up->ut_line, rstail))
    {
      (void) strcpy(up->ut_line, rdtail);
      (void) time(&up->ut_tv.tv_sec);
      up->ut_pid = pid;

      (void) pututxline(up);
      updwtmpx("wtmpx", up);
      break;
    }
  }

  endutxent();
#else
  FILE *fp;
  fpos_t fpos;
  struct utmp u;
  char *rs = (char *) buf, *rd = (char *) buf + 64;

  rnget(s, rs);
  rnget(d, rd);
  fp = fopen("/var/run/utmp", "r+");
  if (!fp) return;
  for (;;)
  {
    (void) fgetpos(fp, &fpos);
    (void) fread(&u, sizeof(struct utmp), 1, fp);
    if (feof(fp))
    {
      break;
    }
    if (!strcmp(u.ut_line, rs))
    {
      (void) strcpy(u.ut_line, rd);
      (void) fsetpos(fp, &fpos);
      (void) fwrite(&u, sizeof(struct utmp), 1, fp);
      break;
    }
  }
  (void) fclose(fp);
#endif /*sun */
#endif
}

 /*ARGSUSED*/ static void finish(int sig)
{
  ENTER("finish");

  (void) tcsetattr(0, TCSAFLUSH, &t);
  yasr_ttyname_r(slave, (char *) buf + 128, 32);
  yasr_ttyname_r(0, (char *) buf + 192, 32);
  utmpconv((char *) buf + 128, (char *) buf + 192, getpid());
  if (cpid)
  {
    (void) kill(cpid, 9);
  }

  /* < Tengoo */
  deleteTengoo( myTengoo);
  /* > */

  DEBUG_END;

  exit(0);
}


static void getinput()
{
  int key;

  ENTER("getinput");

  size = read(0, buf, 255);

  DISPLAY_RAW_BUFFER(buf, size);

  if (!size)
  {
    finish(0);
  }
  key = (int) buf[0];
  if (size > 1)
  {
    key = (key << 8) + buf[1];
  }
  if (size > 2)
  {
    key = (key << 8) + buf[2];
  }
  if (size > 3)
  {
    key = (key << 8) + buf[3];
  }
  if (key >> 8 == 0x1b4f)
  {
    key += 0x000c00;
  }

  /* Convert high-bit meta keys to escape form */
#ifndef __linux__
  if (key >= 0x80 && key <= 0xff) key += 0x1a80;
#endif

  /* < Tengoo */
  GByteArray* aByteArray=transcodeInputTengoo( myTengoo, buf, size);

  if (aByteArray)
    {
      size=MIN(aByteArray->len, MAX_BUF_SIZE);
      strncpy(buf,aByteArray->data, size);

      SHOW_TIME("g_byte_array_free");
      g_byte_array_free( aByteArray, 1);
      (void) write(master, buf, size); /* to be commented */
    }
  /* > */

/*   (void) write(master, buf, size); */
}


/*ARGSUSED*/ static void child_finish(int sig)
{
  int pid=0;

  ENTER("child_finish");

  (void) signal(SIGCHLD, &child_finish);
  pid=waitpid(-1, NULL, WNOHANG);


  if (pid == cpid)
  {
    finish(0);
  }
}

static void getoutput()
{
  ENTER("getoutput");

  size = read(master, buf, 255);

  if (size < 0)
  {
    perror("read");
    exit(1);
  }
  buf[size] = 0;

#ifdef TERMTEST
  (void) printf("size=%d buf=%s\n", size, buf);
#endif

  if (!size)
  {
    finish(0);
  }

  /* < Tengoo */
  GByteArray* aByteArray=transcodeOutputTengoo( myTengoo, buf, size);
/*       (void) write(1, buf, size);  */

  if (aByteArray)
    {
      (void) write(1, aByteArray->data, aByteArray->len); /* in comment */
/*       size=MIN(aByteArray->len, MAX_BUF_SIZE); */
/*       strncpy(buf,aByteArray->data, size); */

      DISPLAY_RAW_BUFFER(aByteArray->data, aByteArray->len);

      SHOW_TIME("g_byte_array_free");
      g_byte_array_free( aByteArray, 1);
    }
  /* > */

  /*  (void) write(1, buf, size);*/

}

static void parent()
{
  fd_set rf;
  struct termios rt;
  int maxfd;

  ENTER("parent");

  (void) tcgetattr(0, &t);
  (void) memcpy(&rt, &t, sizeof(struct termios));
  cfmakeraw(&rt);
  rt.c_cc[VMIN] = 1;
  rt.c_cc[VTIME] = 0;
  (void) tcsetattr(0, TCSAFLUSH, &rt);
  (void) signal(SIGCHLD, &child_finish);
  yasr_ttyname_r(0, (char *) (buf + 128), 32);
  yasr_ttyname_r(slave, (char *) (buf + 192), 32);
  utmpconv((char *) (buf + 128), (char *) (buf + 192), cpid);
  maxfd = master + 1;

  FD_ZERO(&rf);
  for (;;)
  {
    FD_SET(master, &rf);
    FD_SET(0, &rf);
    (void) select(maxfd, &rf, NULL, NULL, NULL);

    if (FD_ISSET(0, &rf))
    {
      getinput();
    }
    if (FD_ISSET(master, &rf))
    {
      getoutput();
    }
  }
}

static void child()
{
  char arg[20];
  char *cp;
  char envstr[40];

  ENTER("child");

  (void) login_tty(slave);
  if (!geteuid())
  { /* if we're setuid root */
    yasr_ttyname_r(0, (char *) buf, 32);
    (void) chown((char *) buf, getuid(), -1);
    (void) setuid(getuid());
  }
  cp = usershell + strlen(usershell) - 1;
  while (*cp != '/')
  {
    cp--;
  }
  cp++;
  arg[0] = shell ? '-' : '\0';
  *(arg + 1) = '\0';
  (void) strcat(arg, cp);
  (void) sprintf(envstr, "SHELL=%s", usershell);
  (void) putenv(envstr);
  if (subprog)
  {
    char *devname = ttyname(0);

    (void) setsid();
    (void) close(0);
    if (open(devname, O_RDWR) < 0)
    {
      perror(devname);
    }

    (void) execve(subprog[0], subprog, environ);
  }
  else
  {
    (void) execl(usershell, arg, 0);
  }
  perror("execl");
  exit(1);
}

int main(int argc, char *argv[])
{
  struct winsize winsz = { 0, 0 };

  /* < Tengoo */
  DEBUG_BEGIN;

  ENTER("main");
  /* > */

  /*  setvbuf(stdout, NULL, _IONBF, 0); / * TBD remove */

  /* initialize gettext */
#ifdef ENABLE_NLS
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
#endif


{
  static int aLock=0;

  while(aLock)
    {
      sleep(1);
      aLock=1;
    }
}


  if (argv[0][0] == '-') shell = 1;
  if (isatty(0))
  {
    (void) ioctl(0, TIOCGWINSZ, &winsz);
  }
  if (!winsz.ws_row)
  {
    winsz.ws_row = 25;
    winsz.ws_col = 80;
  }

  /* init */
  if (argc > 1)
    {
      subprog = argv + 1;
    }
  strcpy(usershell, "/bin/bash");

  /* < Tengoo */
  myTengoo = createTengoo("links2vox", 255);
  /* > */

  SHOW("openpty");
  (void) openpty(&master, &slave, NULL, NULL, &winsz);

  SHOW("fork");
  cpid = fork();
  if (cpid > 0) parent();
  else if (cpid == 0) child();
  perror("fork");
  return -1;
}

/* 
Local variables:
folded-file: t
folding-internal-margins: nil
*/
