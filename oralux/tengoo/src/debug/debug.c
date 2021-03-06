#ifdef DEBUG
/* 
----------------------------------------------------------------------------
debug.c
$Id: debug.c,v 1.4 2006/01/14 23:47:57 gcasse Exp $
$Author: gcasse $
Description: for applicative trace.
$Date: 2006/01/14 23:47:57 $ |
$Revision: 1.4 $ |
Copyright (C) 2003, 2004, 2005 Gilles Casse (gcasse@oralux.org)

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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gnode.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "escape2terminfo.h"
#include "debug.h"

/* < array myStringCapacity */

char* myStringCapacity[]={
  "",
  "ACSC",
  "CBT",
  "BEL",
  "CR",
  "CPI",
  "LPI",
  "CHR",
  "CVR",
  "CSR",
  "RMP",
  "TBC",
  "MGC",
  "CLEAR",
  "EL1",
  "EL",
  "ED",
  "HPA",
  "CMDCH",
  "CWIN",
  "CUP",
  "CUD1",
  "HOME",
  "CIVIS",
  "CUB1",
  "MRCUP",
  "CNORM",
  "CUF1",
  "LL",
  "CUU1",
  "CVVIS",
  "DEFC",
  "DCH1",
  "DL1",
  "DIAL",
  "DSL",
  "DCLK",
  "HD",
  "ENACS",
  "SMACS",
  "SMAM",
  "BLINK",
  "BOLD",
  "SMCUP",
  "SMDC",
  "DIM",
  "SWIDM",
  "SDRFQ",
  "SMIR",
  "SITM",
  "SLM",
  "SMICM",
  "SNLQ",
  "SNRMQ",
  "PROT",
  "REV",
  "INVIS",
  "SSHM",
  "SMSO",
  "SSUBM",
  "SSUPM",
  "SMUL",
  "SUM",
  "SMXON",
  "ECH",
  "RMACS",
  "RMAM",
  "SGR0",
  "RMCUP",
  "RMDC",
  "RWIDM",
  "RMIR",
  "RITM",
  "RLM",
  "RMICM",
  "RSHM",
  "RMSO",
  "RSUBM",
  "RSUPM",
  "RMUL",
  "RUM",
  "RMXON",
  "PAUSE",
  "HOOK",
  "FLASH",
  "FF",
  "FSL",
  "WINGO",
  "HUP",
  "IS1",
  "IS2",
  "IS3",
  "IF",
  "IPROG",
  "INITC",
  "INITP",
  "ICH1",
  "IL1",
  "IP",
  "KA1",
  "KA3",
  "KB2",
  "KBS",
  "KBEG",
  "KCBT",
  "KC1",
  "KC3",
  "KCAN",
  "KTBC",
  "KCLR",
  "KCLO",
  "KCMD",
  "KCPY",
  "KCRT",
  "KCTAB",
  "KDCH1",
  "KDL1",
  "KCUD1",
  "KRMIR",
  "KEND",
  "KENT",
  "KEL",
  "KED",
  "KEXT",
  "KF0",
  "KF1",
  "KF10",
  "KF11",
  "KF12",
  "KF13",
  "KF14",
  "KF15",
  "KF16",
  "KF17",
  "KF18",
  "KF19",
  "KF2",
  "KF20",
  "KF21",
  "KF22",
  "KF23",
  "KF24",
  "KF25",
  "KF26",
  "KF27",
  "KF28",
  "KF29",
  "KF3",
  "KF30",
  "KF31",
  "KF32",
  "KF33",
  "KF34",
  "KF35",
  "KF36",
  "KF37",
  "KF38",
  "KF39",
  "KF4",
  "KF40",
  "KF41",
  "KF42",
  "KF43",
  "KF44",
  "KF45",
  "KF46",
  "KF47",
  "KF48",
  "KF49",
  "KF5",
  "KF50",
  "KF51",
  "KF52",
  "KF53",
  "KF54",
  "KF55",
  "KF56",
  "KF57",
  "KF58",
  "KF59",
  "KF6",
  "KF60",
  "KF61",
  "KF62",
  "KF63",
  "KF7",
  "KF8",
  "KF9",
  "KFND",
  "KHLP",
  "KHOME",
  "KICH1",
  "KIL1",
  "KCUB1",
  "KLL",
  "KMRK",
  "KMSG",
  "KMOV",
  "KNXT",
  "KNP",
  "KOPN",
  "KOPT",
  "KPP",
  "KPRV",
  "KPRT",
  "KRDO",
  "KREF",
  "KRFR",
  "KRPL",
  "KRST",
  "KRES",
  "KCUF1",
  "KSAV",
  "SHIFTKBEG",
  "SHIFTKCAN",
  "SHIFTKCMD",
  "SHIFTKCPY",
  "SHIFTKCRT",
  "SHIFTKDC",
  "SHIFTKDL",
  "KSLT",
  "SHIFTKEND",
  "SHIFTKEOL",
  "SHIFTKEXT",
  "KIND",
  "SHIFTKFND",
  "SHIFTKHLP",
  "SHIFTKHOM",
  "SHIFTKIC",
  "SHIFTKLFT",
  "SHIFTKMSG",
  "SHIFTKMOV",
  "SHIFTKNXT",
  "SHIFTKOPT",
  "SHIFTKPRV",
  "SHIFTKPRT",
  "KRI",
  "SHIFTKRDO",
  "SHIFTKRPL",
  "SHIFTKRIT",
  "SHIFTKRES",
  "SHIFTKSAV",
  "SHIFTKSPD",
  "KHTS",
  "SHIFTKUND",
  "KSPD",
  "KUND",
  "KCUU1",
  "RMKX",
  "SMKX",
  "LF0",
  "LF1",
  "LF10",
  "LF2",
  "LF3",
  "LF4",
  "LF5",
  "LF6",
  "LF7",
  "LF8",
  "LF9",
  "FLN",
  "RMLN",
  "SMLN",
  "RMM",
  "SMM",
  "MHPA",
  "MCUD1",
  "MCUB1",
  "MCUF1",
  "MVPA",
  "MCUU1",
  "NEL",
  "PORDER",
  "OC",
  "OP",
  "PAD",
  "DCH",
  "DL",
  "CUD",
  "MCUD",
  "ICH",
  "INDN",
  "IL",
  "CUB",
  "MCUB",
  "CUF",
  "MCUF",
  "RIN",
  "CUU",
  "MCUU",
  "PFKEY",
  "PFLOC",
  "PFX",
  "PLN",
  "MC0",
  "MC5P",
  "MC4",
  "MC5",
  "PULSE",
  "QDIAL",
  "RMCLK",
  "REP",
  "RFI",
  "RS1",
  "RS2",
  "RS3",
  "RF",
  "RC",
  "VPA",
  "SC",
  "IND",
  "RI",
  "SCS",
  "SGR",
  "SETB",
  "SMGB",
  "SMGBP",
  "SCLK",
  "SCP",
  "SETF",
  "SMGL",
  "SMGLP",
  "SMGR",
  "SMGRP",
  "HTS",
  "SMGT",
  "SMGTP",
  "WIND",
  "SBIM",
  "SCSD",
  "RBIM",
  "RCSD",
  "SUBCS",
  "SUPCS",
  "HT",
  "DOCR",
  "TSL",
  "TONE",
  "UC",
  "HU",
  "U0",
  "U1",
  "U2",
  "U3",
  "U4",
  "U5",
  "U6",
  "U7",
  "U8",
  "U9",
  "WAIT",
  "XOFFC",
  "XONC",
  "ZEROM",
  "SCESA",
  "BICR",
  "BINEL",
  "BIREP",
  "CSNM",
  "CSIN",
  "COLORNM",
  "DEFBI",
  "DEVT",
  "DISPC",
  "ENDBI",
  "SMPCH",
  "SMSC",
  "RMPCH",
  "RMSC",
  "GETM",
  "KMOUS",
  "MINFO",
  "PCTRM",
  "PFXL",
  "REQMP",
  "SCESC",
  "S0DS",
  "S1DS",
  "S2DS",
  "S3DS",
  "SETAB",
  "SETAF",
  "SETCOLOR",
  "SMGLR",
  "SLINES",
  "SMGTB",
  "EHHLM",
  "ELHLM",
  "ELOHLM",
  "ERHLM",
  "ETHLM",
  "EVHLM",
  "SGR1",
  "SLENGTH",
  "TPHL",
  "TEXT...",
};

/* > */
/* < displayBuffer */

static char* TheHtmlStart="\
<title>termbuffer</title>\n\
<html>\n\
<head>\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\" />\n\
<style>\n\
body {\n\
 margin:0;\n\
 padding:0;\n\
 background-color:black;\n\
 color:white;\n\
 font-size:100%;\n\
 font-family:monospace;\n\
 font-weight:normal;\n\
}\n\
\n\
p {\n\
 margin:0;\n\
 padding:0;\n\
 border-bottom:1px red solid;\n\
}\n\
</style>\n\
\n\
</head>\n\
<body>\n";

static char* TheHtmlEnd="</body></html>";

static char* TheHtmlColorArray[]=
  {
    "black",
    "red",
    "green",
    "yellow",
    "blue",
    "fuchsia",
    "aqua",
    "white"
  };

void displayBuffer( char *theDataBuffer, struct t_style* theStyleBuffer, int theMaxLine, int theMaxCol)
{
  int i=0;
  int j=0;
  int aBgColor=TERM_COLOR_BLACK;
  int aFgColor=TERM_COLOR_WHITE;
  FILE* fd=fopen("termbuffer.htm","w");

  ENTER("displayBuffer");

  fprintf(fd,"%s",TheHtmlStart);
  for (i=0;i<theMaxLine;i++)
    {
      char c[sizeof("&nbsp;")+1];
      fprintf(fd, "<p>");
      fprintf(fd, "<span style=\"background-color:%s; color:%s\">", TheHtmlColorArray[ aBgColor], TheHtmlColorArray[ aFgColor]);
      for (j=i*theMaxCol; j < (i+1)*theMaxCol; j++)
	{
	  if ((theStyleBuffer[j].myForegroundColor != aFgColor)
	      ||(theStyleBuffer[j].myBackgroundColor != aBgColor))
	    {
	      aBgColor=theStyleBuffer[j].myBackgroundColor;
	      aFgColor=theStyleBuffer[j].myForegroundColor;
	      fprintf(fd, "</span>");
	      fprintf(fd, "<span style=\"background-color:%s; color:%s\">", TheHtmlColorArray[ aBgColor], TheHtmlColorArray[ aFgColor]);
	    }

	  *c=theDataBuffer[j];
	  *(c+1)=0;
	  switch(*c)
	    {
	    case ' ':strcpy(c,"&nbsp;");break;
	    case '<':strcpy(c,"&lt;");break;
	    case '>':strcpy(c,"&gt;");break;
	    default:break;
	    }
	  fprintf(fd, "%s", c);
	}
      fprintf(fd, "</span></p>\n");
    }
  fprintf(fd,"%s",TheHtmlEnd);

  fclose(fd);
}

/* > */
/* < displayCapacity */
void displayCapacity( enum StringCapacity theCapacity, char* theFile)
{ /* debug: display pattern */
  char* aLine=strdup( yytext);
  char* aString=aLine;
  ENTER("displayCapacity");

  while((aString=strchr (aString, '')))
    {
      *aString='E';
    }
  fprintf(TheOutputStream,"%s: " "|%s = %s|\n", theFile, myStringCapacity[ theCapacity], aLine);
  free(aLine);
}

/* > */
/* < displayColor */
void displayColor( char* theLabel, int theColor, char* theFile)
{
  static char* aColorArray[]=
    {
      "BLACK",
      "RED",
      "GREEN",
      "YELLOW",
      "BLUE",
      "MAGENTA",
      "CYAN",
      "WHITE"
    };
  ENTER("displayColor");

  if (theColor<sizeof(aColorArray)/sizeof(aColorArray[0]))
    {
      fprintf(TheOutputStream,"%s: " "|%s: %s|\n",theFile,theLabel,aColorArray[theColor]);
    }
}
/* > */
/* < displayModes */
void displayStyle(struct t_style* theStyle, char* theFile)
{
  ENTER("displayStyle");
  fprintf(TheOutputStream,"%s: " "|", theFile);
  displayColor( "Background",theStyle->myBackgroundColor, theFile);
  displayColor( "Foreground",theStyle->myForegroundColor, theFile);

  if (theStyle->isStandout)
    {
      fprintf(TheOutputStream,"STANDOUT ");
    }
  if (theStyle->isUnderline)
    {
      fprintf(TheOutputStream,"UNDERLINE ");
    }
  if (theStyle->isReverse)
    {
      fprintf(TheOutputStream,"REVERSE ");
    }
  if (theStyle->isBlink)
    {
      fprintf(TheOutputStream,"BLINK ");
    }
  if (theStyle->isDim)
    {
      fprintf(TheOutputStream,"DIM ");
    }
  if (theStyle->isBold)
    {
      fprintf(TheOutputStream,"BOLD ");
    }
  if (theStyle->isBlank)
    {
      fprintf(TheOutputStream,"BLANK ");
    }
  if (theStyle->isProtect)
    {
      fprintf(TheOutputStream,"PROTECT ");
    }
  if (theStyle->isAlternate)
    {
      fprintf(TheOutputStream,"ALTERNATE_CHARSET ");
    }
  fprintf(TheOutputStream," |\n");
}
/* > */
/* < debugBegin, debugEnd */
FILE* TheOutputStream;

void debugBegin()
{
  if (strcmp(OUTPUT_FILENAME,"stdout")==0)
    {
      TheOutputStream=stdout;
    }
  else
    {
      TheOutputStream=fopen(OUTPUT_FILENAME, "w+");
    }
  setvbuf(TheOutputStream, NULL, _IONBF, 0);
}

void debugEnd()
{
  fclose(TheOutputStream);
}

/* > */
/* < displayRawBuffer */
void displayRawBuffer( unsigned char* theBuffer, int theSize)
{
  unsigned char* c=theBuffer;
  static unsigned char aBuffer[6*255];
  unsigned char* p=aBuffer;
  int fd;

  /*   ENTER("displayRawBuffer"); */

  /*
    p += sprintf((char*)p, "%c[H%c[2J", 0x1b, 0x1b);
  */
  p += sprintf((char*)p, "\r\n-->\r\n");
  while (c < theBuffer+theSize)
    {
      if (*c<0x20)
	{
	  p += sprintf((char*)p, " 0x%x ",*c);
	}
      else
	{
	  p += sprintf((char*)p, "%c", *c);
	}
      c++;
    }
  fd = open( "/dev/tty2", O_RDWR);
  write(fd, aBuffer, p - aBuffer);
  close(fd);
  SHOWn(aBuffer, p - aBuffer);
}

/* > */

/* < displayTree */
void displayTree(void *theNode)
{
  GNode* aNode = theNode;
  fprintf(TheOutputStream,"> displayTree: Node=%x\n", (int)aNode); 
  if (aNode)
    {
      fprintf(TheOutputStream,"next=%x, prev=%x, parent=%x, children=%x\n",
	      (int)aNode->next, (int)aNode->prev,
	      (int)aNode->parent, (int)aNode->children);
      if(aNode->next)
	{
	  displayTree( aNode->next);
	}
      if(aNode->children)
	{
	  displayTree( aNode->children);
	}
    }
}
/* > */

#else
void displayRawBuffer( unsigned char* theBuffer, int theSize){}
#endif

/* 
Local variables:
folded-file: t
folding-internal-margins: nil
*/
