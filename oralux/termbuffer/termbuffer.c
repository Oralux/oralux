/* 
----------------------------------------------------------------------------
termbuffer.c
$Id: termbuffer.c,v 1.3 2005/01/05 23:10:55 gcasse Exp $
$Author: gcasse $
Description: manage the terminal layout in a buffer.
$Date: 2005/01/05 23:10:55 $ |
$Revision: 1.3 $ |
Copyright (C) 2003, 2004 Gilles Casse (gcasse@oralux.org)

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
/* < include */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "escape2terminfo.h"
#include "termbuffer.h"
#include "debug.h"

/* Misc */
#define MAX_VALUE(a,b) (a > b) ? a : b;
#define MIN_VALUE(a,b) (a < b) ? a : b;

/* > */
/* < flex definitions */
int yywrap ()
{  
   return 1; 
}
/* > */
/* < createBuffer */
void createBuffer( char** theDataBuffer, struct t_style** theStyleBuffer, int theNumberOfCell)
{
  ENTER("createBuffer");
  *theDataBuffer=(char*)malloc( theNumberOfCell * sizeof(char));
  *theStyleBuffer=(struct t_style*)malloc( theNumberOfCell * sizeof(struct t_style));
}
/* > */
/* < clearBuffer */
#define clearBuffer(theDataBuffer, theNumberOfCell) memset(theDataBuffer, 0x20, theNumberOfCell*sizeof(char))
/* > */
/* < setStyleBuffer */

void setStyleBuffer( struct t_style* theStyleBuffer, int theNumberOfCell, struct t_style* theDefaultStyle)
{
  int i=0;
  for (i=0; i<theNumberOfCell; i++)
    {
      copyStyle(theStyleBuffer+i, theDefaultStyle);
    }
}

/* > */
/* < initCursor */
/*
theFirstCell indicates the value of the first line and the first column.
This can be 0 or 1 according to the termbuffer.
*/
void initCursor(struct t_cursor* theCursor, int theFirstCell, struct t_style* theStyle)
{
  ENTER("initCursor");
  theCursor->myLine=theFirstCell;
  theCursor->myCol=theFirstCell;
  copyStyle (& (theCursor->myStyle), theStyle);
}
/* > */
/* < getCell, getCursor*/
#ifdef DEBUG
int getCell( int theLine, int theCol, int theNumberOfCol) 
{
  assert((theCol<theNumberOfCol) && (theLine<30));
  return theLine * theNumberOfCol + theCol;
  }
#else
#define getCell( theLine, theCol, theNumberOfCol) (theLine * theNumberOfCol + theCol)
#endif
void getCursor( int theCell, int theNumberOfCol, struct t_cursor* theCursor) 
{
  ENTER("getCursor");
  theCursor->myLine = theCell / theNumberOfCol;
  theCursor->myCol = theCell % theNumberOfCol;
}
/* > */
/* < compareAndSetStyle */
/* 
search if the style of the cell equals one of the two supplied styles (in the ListOfStyle).
If yes, this style is copied in theFoundStyle and 1 is returned. 
Otherwise 0 is returned
*/
int compareAndSetStyle( struct t_style* theListOfStyle, struct t_style* theStyleBuffer, int theLine, int theCol, int theNumberOfCol, struct t_style* theFoundStyle)
{
  int aCell=getCell( theLine, theCol, theNumberOfCol);
  int aStatus=0;
  if ((compareStyle (theListOfStyle, theStyleBuffer+aCell) == 0)
      || (compareStyle (theListOfStyle+1, theStyleBuffer+aCell) == 0))
    {
      copyStyle( theFoundStyle, theStyleBuffer+aCell);
      DISPLAY_STYLE(theFoundStyle);
      aStatus=1;
    }
  return aStatus;
}

/* > */
/* < getBackgroundStyle */

/* retrieve the background style of a line portion. 
If the background style has been found, theStyle is updated, and the number of distinct styles found in the line portion is returned (so 1, 2 or 3). 
Otherwise 0 is returned. 

Three cases are expected:
- 1 style in the line portion: each character has the same style which is also considered as the background style.
- 2 styles: then the surroundings of the line portion is checked to determine the background.
- 3 styles: if two of them are equal, this the background.
- other cases: not processed.
*/ 

int getBackgroundStyle( int theLine, int theCol, struct t_style* theStyleBuffer, int theLength, int theNumberOfLine, int theNumberOfCol, struct t_style* theStyle)
{
  enum {MAX_DISTINCT_STYLE=3};
  struct t_style aStyle[MAX_DISTINCT_STYLE];
  int aStyleWeight[MAX_DISTINCT_STYLE];
  int aIndex=0;
  int i=0;
  int aCell=getCell( theLine, theCol, theNumberOfCol);

  ENTER("getBackgroundStyle");

  copyStyle( aStyle+aIndex, theStyleBuffer+aCell+i);
  aStyleWeight[ aIndex]=1;

  for(i=1; i<theLength; i++)
    {
      if (compareStyle (aStyle+aIndex, theStyleBuffer+aCell+i) == 0)
	{
	  ++aStyleWeight[ aIndex];
	}
      else if (aStyleWeight[ aIndex - 1]==1)
	{ /* the previous style concerns just one char */ 
	  --aIndex; /* it is not taken in account (it can be a shortcut). */
	  if ((aIndex>0) 
	      && (compareStyle (aStyle+aIndex-1, theStyleBuffer+aCell+i) == 0))
	    { /* the current style is the same than the previous one */
	      --aIndex;
	      ++aStyleWeight[ aIndex];
	    }
	  else
	    { /* the current style is distinct */
	      copyStyle( aStyle+aIndex, theStyleBuffer+aCell+i);
	      aStyleWeight[ aIndex]=1;	      
	    }
	}
      else if (++aIndex < MAX_DISTINCT_STYLE)
	{
	  copyStyle( aStyle+aIndex, theStyleBuffer+aCell+i);
	  aStyleWeight[ aIndex]=1;
	}
      else
	{ /* too many styles: abandon... */
	  aIndex=-1;
	}
    }

#ifdef DEBUG
  for (i=0; i<=aIndex; i++)
    {
      SHOW3("* style #%d used by %d chars\n", i, aStyleWeight[i]);
      DISPLAY_STYLE(aStyle+i);
    }
#endif

  switch(aIndex)
    {
    case 0: /* just one style */
      copyStyle( theStyle, aStyle);
      break;
    case 1: /* two styles: look around the line portion */
      aIndex=-1; /* no result at the moment */

      /* Check the top cells */
      if (theLine > 0)
	{ 
	  if (theCol > 0)
	    {
	      SHOW("Test Top Left Cell");
	      aIndex=compareAndSetStyle( aStyle, theStyleBuffer, theLine-1, theCol-1, theNumberOfCol, theStyle);
	    }
	  if((theCol+theLength < theNumberOfCol) && (aIndex != 1))
	    {
	      SHOW("Test Top Right Cell");
	      aIndex=compareAndSetStyle( aStyle, theStyleBuffer, theLine-1, theCol+theLength, theNumberOfCol, theStyle);
	    }
	}
      /* otherwise check the bottom cells */
      if ((theLine+1 < theNumberOfLine) && (aIndex != 1))
	{
	  if (theCol > 0)
	    {
	      SHOW("Test Bottom Left Cell");
	      aIndex=compareAndSetStyle( aStyle, theStyleBuffer, theLine+1, theCol-1, theNumberOfCol, theStyle);
	    }
	  if((theCol+theLength < theNumberOfCol) && (aIndex != 1))
	    {
	      SHOW("Test Bottom Right Cell");
	      aIndex=compareAndSetStyle( aStyle, theStyleBuffer, theLine+1, theCol+theLength, theNumberOfCol, theStyle);
	    }
	}
      if (aIndex != 1)
	{
	  SHOW("Can't decide...");
	  aIndex=-1;
	}
      break;
    case 2: /* 3 styles: 2 of them are equal */
      if (compareStyle (aStyle, aStyle+1) == 0)
	{
	  copyStyle( theStyle, aStyle);
	}
      else if(compareStyle (aStyle, aStyle+2) == 0)
	{
	  copyStyle( theStyle, aStyle);
	}
      else if(compareStyle (aStyle+1, aStyle+2) == 0)
	{
	  copyStyle( theStyle, aStyle+1);
	}
      else
	{
	  aIndex=-1;
	}
      break;
    case -1:
    default:
      aIndex=-1;
      break;
    }
  return aIndex + 1;
}

/* > */
/* < eraseCharWithThisStyle */
void eraseCharWithThisStyle( char* theDataBuffer, struct t_style* theStyleBuffer, int theLength, struct t_style* theStyle)
{
  int i=0;

  ENTER("eraseCharWithThisStyle");
  
  for(i=0; i<theLength; i++)
    {
      if (compareStyle( theStyle, theStyleBuffer+i)==0)
	{
	  theDataBuffer[i]=0x20;
	}
    }
}
/* > */
/* < addString */
/* 
Concatenate the two supplied string, a new pointer is returned in *theDestination.
theLength is the maximum number of bytes copied from theSource. 
Warning: theDestination is either NULL or expected to come from malloc.
 */ 
int addString( char** theDestination, char* theSource, int theLength)
{
  char* aNewString=NULL;
  int aOldLength = (*theDestination) ? strlen(*theDestination): 0; 
  aNewString=realloc(*theDestination, aOldLength + theLength + 1);

  if (aNewString)
    {
      *theDestination=aNewString;
      strncpy(aNewString+aOldLength, theSource, theLength);
      aNewString[ aOldLength+theLength]=0;
    }
  return (aNewString != NULL);
}
/* > */
/* < flushText */
void flushText( char* theDataBuffer, struct t_style* theStyleBuffer, int theLength, char** theOutputBuffer)
{
  ENTER("flushText");
  addString(theOutputBuffer, theDataBuffer, theLength);
}
/* > */
/* < testIfPortionsAreMenuItems */
int testIfPortionsAreMenuItems( struct t_linePortion* theLinePortion1, struct t_linePortion* theLinePortion2)
{
  ENTER("testIfPortionsAreMenuItems");
  SHOW3("* theLinePortion1: myFirstCol=%d, myLastCol=%d\n",theLinePortion1->myFirstCol, theLinePortion1->myLastCol);
  SHOW3(" Content modified=%d, style modified=%d\n",theLinePortion1->myContentIsModified, theLinePortion1->myStyleIsModified);
  SHOW3("* theLinePortion2: myFirstCol=%d, myLastCol=%d\n",theLinePortion2->myFirstCol, theLinePortion2->myLastCol);
  SHOW3(" Content modified=%d, style modified=%d\n",theLinePortion2->myContentIsModified, theLinePortion2->myStyleIsModified);
  return !theLinePortion1->myContentIsModified 
    && !theLinePortion2->myContentIsModified
    && theLinePortion1->myStyleIsModified
    && theLinePortion2->myStyleIsModified
    && ((theLinePortion1->myLastCol >= theLinePortion2->myFirstCol)
	|| (theLinePortion2->myLastCol >= theLinePortion1->myFirstCol));
}
/* > */
/* < lookForHighlightedMenuItem */

/* Among theLinePortion - array of two menu items - retreives the index of the highlighted one.
It guessed the background style and comapres it to the menu item style.

The returned value is:
* 0: theLinePortion[0] is the highlighted style.
* 1: theLinePortion[1] is the highlighted style.
* -1: no highlighted item found.
*/
int lookForHighlightedMenuItem( struct t_termbuffer* this)
{
  /* Distinguish the top and bottom portions */
  struct t_linePortion* aTopPortion=this->myLinePortion;
  struct t_linePortion* aBottomPortion=this->myLinePortion;
  int aStatus=0;
  struct t_style aSurroundingBackgroundStyle;
  struct t_style aTopPortionBackgroundStyle;
  struct t_style aBottomPortionBackgroundStyle;
  int aLength=this->myLinePortion->myLastCol + 1 - this->myLinePortion->myFirstCol;

  ENTER("lookForHighlightedMenuItem");

  if (this->myLinePortion[0].myLine < this->myLinePortion[1].myLine)
    {
      aBottomPortion++; /* points to this->myLinePortion+1 */
    }
  else
    {
      aTopPortion++;  /* points to this->myLinePortion+1 */
    }

  /* Look for the background style of the top and bottom line portions */
  if(!getBackgroundStyle( aTopPortion->myLine, aTopPortion->myFirstCol, this->myStyleBuffer, aLength, this->myNumberOfLine, this->myNumberOfCol, &aTopPortionBackgroundStyle))
    {
      SHOW("Top portion: no background found");
      return -1;
    }

  if(!getBackgroundStyle( aBottomPortion->myLine, aBottomPortion->myFirstCol, this->myStyleBuffer, aLength, this->myNumberOfLine, this->myNumberOfCol, &aBottomPortionBackgroundStyle))
    {
      SHOW("Bottom portion: no background found");
      return -1;
    }

  /* Look for the background style of the surrounding line portions
     Two possible cases:
     - Contiguous line portions: we check if possible the line upper than aTopPortion or the line lower than aBottomPortion
     - Not contiguous: check the line lower than aTopPortion
  */
  /* Check if the lines are contiguous */
  if ( abs( aTopPortion->myLine - aBottomPortion->myLine)==1)
    { /* yes, contiguous */
      if (aTopPortion->myLine > 0)
	{ /* Check the upper line */
	  aStatus=getBackgroundStyle( aTopPortion->myLine - 1, aTopPortion->myFirstCol, this->myStyleBuffer, aLength, this->myNumberOfLine, this->myNumberOfCol, &aSurroundingBackgroundStyle);
	}
      if (!aStatus && (aBottomPortion->myLine + 1 < this->myNumberOfLine))
	{ /* Check the lower line */
	  aStatus=getBackgroundStyle( aBottomPortion->myLine - 1, aBottomPortion->myFirstCol, this->myStyleBuffer, aLength, this->myNumberOfLine, this->myNumberOfCol, &aSurroundingBackgroundStyle);
	}
    }
  else
    { /* Not contiguous: check the line lower than aTopPortion */
      aStatus=getBackgroundStyle( aTopPortion->myLine + 1, aTopPortion->myFirstCol, this->myStyleBuffer, aLength, this->myNumberOfLine, this->myNumberOfCol, &aSurroundingBackgroundStyle);
    }

  /* If the surrounding background has been found, compare it to the style of the bottom and top line portion */
  if (aStatus)
    {
      aStatus=-1;
      if (0==compareStyle( &aTopPortionBackgroundStyle, &aSurroundingBackgroundStyle))
	{
	  if (0!=compareStyle( &aBottomPortionBackgroundStyle, &aSurroundingBackgroundStyle))
	    { /* Return the index of the bottom portion */
	      aStatus=(aBottomPortion==this->myLinePortion) ? 0 : 1;
	      SHOW("Bottom portion is highlighted");
	    }
	}
      else if (0==compareStyle( &aBottomPortionBackgroundStyle, &aSurroundingBackgroundStyle))
	{ /* return the index of the top portion */
	  aStatus=(aTopPortion==this->myLinePortion) ? 0 : 1;
	  SHOW("Top portion is highlighted");
	}
    }
  else
    {
      aStatus=-1;
      SHOW("No highlighted portion found");
    }
  return aStatus;
}

/* > */
/* < initCurrentLinePortion */
void initCurrentLinePortion( struct t_termbuffer* this)
{
  struct t_linePortion* aPortion = this->myLinePortion + this->myLinePortionIndex;
  ENTER("initCurrentLinePortion");
  SHOW3("myLine=%d, myCol=%d\n", this->myCursor.myLine, this->myCursor.myCol);
  aPortion->myLine = this->myCursor.myLine;
  aPortion->myFirstCol = this->myCursor.myCol;
  aPortion->myLastCol = this->myCursor.myCol;
  aPortion->myContentIsModified = 0;
  aPortion->myStyleIsModified = 0;
}

/* > */
/* < flushPortion */

/* Process and display the stored line portions according to the style.
This function can:
- determine the possible selected item in a menu and displays it.
- take in account the style (single letter as shortcut, distinct voices)

* selected item
If there are 1 or 2 line portions without any content modification but with style change, it can be the new selected item and the previous one which are displayed. 
The current background style around the line portion is determined.

*/
void flushPortion( struct t_termbuffer* this, char** theOutputBuffer)
{
  ENTER("flushPortion");

  /* Firstly, look for menu items: 2 lines without any content modification but style change */ 
  if ((this->myLinePortionIndex==1)
      && testIfPortionsAreMenuItems(this->myLinePortion, this->myLinePortion+1))
    {
      int i;
      int aCell;
      switch( i=lookForHighlightedMenuItem( this))
	{
	case 0:
	case 1:
	  aCell=getCell( this->myLinePortion[i].myLine, this->myLinePortion[i].myFirstCol, this->myNumberOfCol);
	  flushText( this->myDataBuffer + aCell, NULL, this->myLinePortion[i].myLastCol - this->myLinePortion[i].myFirstCol, theOutputBuffer);
	  break;
	case -1:
	default:
	  for (i=0; i<2; i++)
	    {
	      aCell=getCell( this->myLinePortion[i].myLine, this->myLinePortion[i].myFirstCol, this->myNumberOfCol);	    
	      flushText( this->myDataBuffer + aCell, NULL, this->myLinePortion[i].myLastCol - this->myLinePortion[i].myFirstCol, theOutputBuffer);
	    }
	  break;
	}
    }
  else
    { /* process each line portion */
      struct t_linePortion* aPortion=NULL;
      int aFirstCell=0;
      int aLastCell=0;
      int i=0;

      for (i=0; i<=this->myLinePortionIndex; i++)
	{
	  struct t_style aStyle;
	  int aLength=0;
	  aPortion=this->myLinePortion+i;
	  aFirstCell=getCell(aPortion->myLine, aPortion->myFirstCol, this->myNumberOfCol);
	  aLastCell=getCell(aPortion->myLine, aPortion->myLastCol, this->myNumberOfCol);

	  aLength=aLastCell + 1 - aFirstCell;

	  if (aPortion->myContentIsModified)
	    {
	      flushText( (char*)(this->myDataBuffer + aFirstCell), NULL, aLength, (char**)theOutputBuffer);
	    }
	  else if (aPortion->myStyleIsModified)
	    {
	      int aStatus=getBackgroundStyle( aPortion->myLine, aPortion->myFirstCol, this->myStyleBuffer, aLength, this->myNumberOfLine, this->myNumberOfCol, &aStyle);

	      
	      if (aStatus)
		{ 
		  /* The characters associated with the background style are not displayed */
		  /* First duplicate the current string */
		  char* aFilteredString=NULL;
		  char c = this->myDataBuffer[ aFirstCell + aLength];
		  this->myDataBuffer[ aFirstCell + aLength] = 0;
		  aFilteredString = strdup(this->myDataBuffer + aFirstCell);
		  this->myDataBuffer[ aFirstCell + aLength] = c;

		  /* Erase the characters */
		  eraseCharWithThisStyle( aFilteredString, this->myStyleBuffer + aFirstCell, aLength, &aStyle);
		  
		  /* And display the remaining characters */
		  flushText( aFilteredString, NULL, aLength, theOutputBuffer);

		  free( aFilteredString);
		}
	      else
		{
		  flushText( this->myDataBuffer + aFirstCell, NULL, aLength, theOutputBuffer);
		}
	    }
	}
    }

  this->myLinePortionIndex=-1;
}
/* > */
/* < getLinePortion */

/* Get the index of the current line portion. 
If the line number has been changed, the next line portion is used.
If the max is reached, the previous line portions are flushed (theOutput).
*/
int getLinePortion( struct t_termbuffer* this, char** theOutput)
{
  int* aIndex = &(this->myLinePortionIndex);

  if (*aIndex==-1) 
    { /* First portion */
      *aIndex=0;
      initCurrentLinePortion( this);
      SHOW("FIRST PORTION");
    }
  else if (this->myLinePortion[ *aIndex].myLine != this->myCursor.myLine)
    { /* New portion */
      if (1 + *aIndex >= MAX_LINE_PORTION)
	{
	  flushPortion(this, theOutput);
	}

      ++ *aIndex;

      initCurrentLinePortion( this);
      SHOW2("NEW PORTION: %d\n", *aIndex);
    }
  return *aIndex;
}

/* > */
/* < setPortion */

/* 
Update the column number of the current line portion.
*/ 

void setLinePortionCol( struct t_termbuffer* this, int theCol)
{
  struct t_linePortion* aLinePortion = this->myLinePortion + this->myLinePortionIndex;
  ENTER("setLinePortionCol");

  if (aLinePortion->myLastCol < theCol)
    {
      aLinePortion->myLastCol=theCol;
    }
  else if (aLinePortion->myFirstCol > theCol)
    {
      aLinePortion->myFirstCol=theCol;
    }
}

/* > */
/* < storeChar, storeStyle */

/* Store the style (color, bold,...); return 1 if the style of the cell has been changed */
int storeStyle( struct t_style* theStyleBuffer, struct t_style* theStyle)
{
  int aNewStyle=0;
  ENTER("storeStyle");
  
  /* Test if the style has changed */
  if (0 != compareStyle( theStyleBuffer, theStyle))
    {
      aNewStyle=1;
      copyStyle( theStyleBuffer, theStyle);
      SHOW("New Style!");
    }

  return aNewStyle;
}

/* Store the char; return 1 if the cell content has been changed */
int storeChar(char* theDataBuffer, char theChar)
{
  int aNewChar=0;
  ENTER("storeChar");

  /* Check if the character has changed */
  if (*theDataBuffer != theChar)
    {
      aNewChar=1;
      *theDataBuffer = theChar;
      SHOW("New Char!");
    }

  return aNewChar;
}

/* > */
/* < eraseCharacter */
void eraseCharacter( struct t_termbuffer* this, int theNumberOfCharToErase, char** theOutput)
{
  int i=0;
  int aContentIsModified=0;
  int aStyleIsModified=0;
  int aCell=getCell( this->myCursor.myLine, this->myCursor.myCol, this->myNumberOfCol);
  char* aChar=this->myDataBuffer + aCell;
  struct t_style* aStyle=this->myStyleBuffer+aCell;
  int aLinePortionIndex=getLinePortion( this, theOutput);
  struct t_linePortion* aLinePortion = this->myLinePortion;
  int aNumberOfCharToErase=theNumberOfCharToErase;

  ENTER("eraseCharacter");

  if (aNumberOfCharToErase + this->myCursor.myCol > this->myNumberOfCol)
    {
      aNumberOfCharToErase = this->myNumberOfCol - this->myCursor.myCol;
    }

  for (i=0; i<aNumberOfCharToErase; i++)
    {
      if (this->myErasedCharAreReturned)
	{
	  addString( theOutput, aChar+i, 1);
	}
      if (storeChar( aChar+i, 0x20))
	{ 
	  aContentIsModified=1;
	}
      if (storeStyle( aStyle+i, &(this->myCursor.myStyle)))
	{
	  aStyleIsModified=1;
	}  
    }

  if (aContentIsModified)
    {
      aLinePortion[ aLinePortionIndex].myContentIsModified = 1;
    }
  if (aStyleIsModified)
    {
      aLinePortion[ aLinePortionIndex].myStyleIsModified = 1;
    }
}
/* > */
/* < eraseCell */
/*
Erase n cells from one position (theFirstCursor) to the final one (theLastCursor) using the default style.
*/
void eraseCell( struct t_termbuffer* this, struct t_cursor* theFirstCursor, struct t_cursor* theLastCursor, struct t_style* theDefaultStyle, char** theOutput)
{
  int aFirstCell = getCell( theFirstCursor->myLine, theFirstCursor->myCol, this->myNumberOfCol);
  int aLastCell = getCell( theLastCursor->myLine, theLastCursor->myCol, this->myNumberOfCol);
  struct t_style* aStyle = NULL;
  int aNumberOfCellToErase = 0;
  int i=0;

  ENTER("eraseCell");

  if (aFirstCell > aLastCell)
    {
      int aCell=aFirstCell;
      aFirstCell=aLastCell;
      aLastCell=aCell;
    }

  aNumberOfCellToErase = aLastCell - aFirstCell + 1;

  clearBuffer( this->myDataBuffer + aFirstCell, aNumberOfCellToErase);

  aStyle = this->myStyleBuffer + aFirstCell;
  for (i=0; i<aNumberOfCellToErase; i++)
    {
      copyStyle(aStyle+i, theDefaultStyle);      
    }

  /* TBD: the line portion could be updated since the content/style are probably modified. 
     pros : useful to say e.g. a single erased char; cons = slower */
}
/* > */
/* < eraseCell */
/*
Insert n char from the current col to the supplied one using the default style.
*/
void insertCol( struct t_termbuffer* this, int theLastCol, struct t_style* theDefaultStyle, char** theOutput)
{
  int aFirstCell = 0;
  int aFirstCol = this->myCursor.myCol;
  int aLastCol = theLastCol;
  int aFinalCol = this->myNumberOfCol - 1;
  struct t_style* aStyle = NULL;
  int aNumberOfCellToInsert = 0;
  int aNumberOfCellToMove = 0;
  int i=0;

  ENTER("insertCell");

  /* check */
  if (aFirstCol > aLastCol)
    {
      int aCol=aFirstCol;
      aFirstCol=aLastCol;
      aLastCol=aCol;
    }

  if (aLastCol > aFinalCol)
    {
      aLastCol = aFinalCol;
    }

  if (aFirstCol > aFinalCol)
    {
      aFirstCol = aFinalCol;
    }

  aNumberOfCellToInsert = aLastCol - aFirstCol + 1;
  aNumberOfCellToMove = aFinalCol - aLastCol;

  aFirstCell = getCell( this->myCursor.myLine, aFirstCol, this->myNumberOfCol);

  memmove( this->myDataBuffer + aFirstCell + aNumberOfCellToInsert, this->myDataBuffer + aFirstCell, aNumberOfCellToMove);
  memmove( this->myStyleBuffer + aFirstCell + aNumberOfCellToInsert, this->myStyleBuffer + aFirstCell, aNumberOfCellToMove*sizeof( struct t_style));
  clearBuffer( this->myDataBuffer + aFirstCell, aNumberOfCellToInsert);

  /* and fill the empty area using the default style */
  aStyle = this->myStyleBuffer + aFirstCell;
  for (i=0; i<aNumberOfCellToInsert; i++)
    {
      copyStyle(aStyle+i, theDefaultStyle);      
    }

  /* TBD: the line portion could be updated since the content/style are probably modified. 
     pros : useful to say e.g. a single erased char; cons = slower */
}
/* > */
/* < deleteCharacter */
void deleteCharacter( struct t_termbuffer* this, int theNumberOfCharToDelete, char** theOutput)
{
  int aNumberOfCharToDelete=theNumberOfCharToDelete;
  int aCurrentCell=getCell( this->myCursor.myLine, this->myCursor.myCol, this->myNumberOfCol);
  struct t_linePortion* aLinePortion = this->myLinePortion;
  int aLinePortionIndex=getLinePortion( this, theOutput);
  int aNumberOfCharToMove=0;
  int aLastCell= 0;
      
  ENTER("deleteCharacter");

  if (aNumberOfCharToDelete + this->myCursor.myCol > this->myNumberOfCol)
    {
      aNumberOfCharToDelete = this->myNumberOfCol - this->myCursor.myCol;
    }
  if (aNumberOfCharToDelete <= 0)
    {
      return;
    }

  aNumberOfCharToMove = this->myNumberOfCol - (this->myCursor.myCol + aNumberOfCharToDelete);
  if (aNumberOfCharToMove > 0)
    {
      memmove( this->myDataBuffer + aCurrentCell, this->myDataBuffer + aCurrentCell + aNumberOfCharToDelete, aNumberOfCharToMove);
    }

  /* Clear the remaining cells */
  aLastCell = getCell( this->myCursor.myLine, this->myNumberOfCol - aNumberOfCharToDelete, this->myNumberOfCol);
  clearBuffer( this->myDataBuffer + aLastCell, aNumberOfCharToDelete);
  setStyleBuffer( this->myStyleBuffer + aLastCell, aNumberOfCharToDelete, &(this->myDefaultStyle));

  /* Update the line portion */

  /* Modifications are expected */
  aLinePortion[ aLinePortionIndex].myContentIsModified = 1;
  aLinePortion[ aLinePortionIndex].myStyleIsModified = 1;
  
  if (aLinePortion[ aLinePortionIndex].myLastCol > this->myCursor.myCol)
    {
      aLinePortion[ aLinePortionIndex].myLastCol= MAX_VALUE(this->myCursor.myCol, aLinePortion[ aLinePortionIndex].myLastCol - aNumberOfCharToDelete);
    }
  if (aLinePortion[ aLinePortionIndex].myFirstCol > this->myCursor.myCol)
    {
      aLinePortion[ aLinePortionIndex].myFirstCol= MAX_VALUE(this->myCursor.myCol, aLinePortion[ aLinePortionIndex].myFirstCol - aNumberOfCharToDelete);
    }
}
/* > */
/* < deleteLine */
void deleteLine( struct t_termbuffer* this, int theNumberOfLineToDelete, char** theOutput)
{
  int aNumberOfLineToDelete=theNumberOfLineToDelete;
  int aFirstCell=getCell( this->myCursor.myLine, 0, this->myNumberOfCol);
  int aLastCell= 0;
  struct t_linePortion* aLinePortion = this->myLinePortion;
  int aLinePortionIndex=getLinePortion( this, theOutput);
  int aNumberOfLineToMove=0;
  int aNumberOfColToDelete = 0;

  ENTER("deleteLineacter");

  if (aNumberOfLineToDelete + this->myCursor.myLine > this->myNumberOfLine)
    {
      aNumberOfLineToDelete = this->myNumberOfLine - this->myCursor.myLine;
    }
  if (aNumberOfLineToDelete <= 0)
    {
      return;
    }

  aNumberOfColToDelete = aNumberOfLineToDelete * this->myNumberOfCol;
  aNumberOfLineToMove = this->myNumberOfLine - (this->myCursor.myLine + aNumberOfLineToDelete);
  if (aNumberOfLineToMove > 0)
    {
      memmove( this->myDataBuffer + aFirstCell, this->myDataBuffer + aFirstCell + aNumberOfColToDelete, aNumberOfLineToMove * this->myNumberOfCol);
    }

  /* Clear the remaining lines */
  aLastCell= getCell( this->myNumberOfLine - aNumberOfLineToDelete, 0, this->myNumberOfCol);
  clearBuffer( this->myDataBuffer + aLastCell, aNumberOfColToDelete);
  setStyleBuffer( this->myStyleBuffer + aLastCell, aNumberOfColToDelete, &(this->myDefaultStyle));

  /* Update the line portion */

  /* Modifications are expected */
  aLinePortion[ aLinePortionIndex].myContentIsModified = 1;
  aLinePortion[ aLinePortionIndex].myStyleIsModified = 1;
  }
/* > */
/* < setChar*/
void setChar( struct t_termbuffer* this, char theNewChar, char** theOutput)
{
  int aCell=0;
  int aLinePortionIndex=0;
  struct t_linePortion* aLinePortion = NULL;

  ENTER("setChar");

  aLinePortion = this->myLinePortion;
  aCell=getCell( this->myCursor.myLine, this->myCursor.myCol, this->myNumberOfCol);
  aLinePortionIndex=getLinePortion( this, theOutput);


  if (storeChar( this->myDataBuffer + aCell, theNewChar))
    { /* the content has been modified */
      aLinePortion[ aLinePortionIndex].myContentIsModified=1;
    }
  if (storeStyle( this->myStyleBuffer + aCell, &(this->myCursor.myStyle)))
    { /* the style has been modified */
      aLinePortion[ aLinePortionIndex].myStyleIsModified=1;
    }
  
  setLinePortionCol( this, this->myCursor.myCol);
  ++this->myCursor.myCol;
}    
/* > */
/* < createTermbuffer, deleteTermbuffer */
/* Create a termbuffer and returns */
struct t_termbuffer* createTermbuffer( enum termbufferName theName, int theNumberOfLine, int theNumberOfCol)
{
  int i=0;
  struct t_termbuffer* this = (struct t_termbuffer *) malloc(sizeof(struct t_termbuffer));
  int aNumberOfCell = theNumberOfLine * theNumberOfCol;
  createBuffer( &(this->myDataBuffer), &(this->myStyleBuffer), aNumberOfCell);
  clearBuffer(this->myDataBuffer, aNumberOfCell);

  /* default style: black and white */
  clearStyle( &(this->myDefaultStyle));
  this->myDefaultStyle.myBackgroundColor=TERM_COLOR_BLACK;
  this->myDefaultStyle.myForegroundColor=TERM_COLOR_WHITE;

  copyStyle (&TheDefaultStyle, &(this->myDefaultStyle));
  copyStyle (&TheCurrentStyle, &(this->myDefaultStyle));

  /* init style buffer */
  for (i=0;i<aNumberOfCell;i++)
    {
      copyStyle(this->myStyleBuffer+i, &(this->myDefaultStyle));
    }
  this->myNumberOfLine=theNumberOfLine;
  this->myNumberOfCol=theNumberOfCol;

  initCursor( &(this->myCursor), 0, &(this->myDefaultStyle));
  initCursor( &(this->mySavedCursor), 0, &(this->myDefaultStyle));

  this->myErasedCharAreReturned = 0;

  return this;
}

void deleteTermbuffer( struct t_termbuffer* this)
{
  free(this->myDataBuffer);
  free(this->myStyleBuffer);
  free(this);
}

/* > */
/* < checkString */
void checkString( char** theOutput)
{
  int aLength=0;
  int aEscapeSequence=0;
  int i=0;
  char* aString;
  
  if ((theOutput==NULL)||(*theOutput==NULL))
    {
      return;
    }

  aString=*theOutput;
  aLength=strlen( aString);

  /* TBD: to be removed.
     Any escape sequence is filtered.
     In the future, no escape sequence must be found.
  */
  for (i=0; i<aLength;i++)
    {
      if (!aEscapeSequence)
	{
	  if (aString[i]==0x1b)
	    {
	      aEscapeSequence=1;
	      aString[i]=0x20;
	    }
	}
      else 
	{
	  switch(aString[i])
	    {
	    case '\n':
	    case '\r':
	    case ' ':
	    case '\t':
	      aEscapeSequence=0;
	      break;
	    default:
	      aString[i]=0x20;
	      break;
	    }
	}
    }
}
/* > */
/* < interpretEscapeSequence */

void interpretEscapeSequence( struct t_termbuffer* this, FILE* theStream, char** theOutput)
{
  enum StringCapacity aCapacity;
  struct t_cursor* aCursor=&(this->myCursor);
  int aNumberOfCell=this->myNumberOfLine * this->myNumberOfCol;

  ENTER("interpretEscapeSequence");

  this->myLinePortionIndex=-1;

  yyin=theStream;

  while((aCapacity=yylex()))
    {
      DISPLAY_CAPACITY( aCapacity);

      switch(aCapacity)
	{
	case CLEAR:
	  this->myLinePortionIndex=-1;
	  aCursor->myCol=0;
	  aCursor->myLine=0;
	  clearBuffer(this->myDataBuffer, aNumberOfCell);
	  setStyleBuffer(this->myStyleBuffer, aNumberOfCell, &(this->myDefaultStyle));
	  break;
	case CUB1:
	  if (aCursor->myCol!=0)
	    {
	      aCursor->myCol--;
	    }
	  break;
	case CUD1:
	  aCursor->myLine++;
	  break;
	case CR:
	  aCursor->myCol=0;
	  break;
	case NEL:
	  aCursor->myLine++;
	  aCursor->myCol=0;
	  break;
	case CUF1:
	  aCursor->myCol++;
	  break;
	case CUP:
	  aCursor->myLine = (TheParameter[0] > 0) ? TheParameter[0] - 1 : TheParameter[0];
	  aCursor->myCol = (TheParameter[1] > 0) ? TheParameter[1] - 1 : TheParameter[1];
	  break;
	case CUU:
	  aCursor->myLine-=TheParameter[0];
	  break;
	case DCH: /* delete characters (shorter line) */
	    /* TheParameter[0] gives the number of characters to delete */
	    deleteCharacter( this, TheParameter[0], theOutput);
	  break;
	case DL: /* delete lines */
	    /* TheParameter[0] gives the number of lines to delete */
	    deleteLine( this, TheParameter[0], theOutput);
	  break;
	case ECH: /* Erase characters (same line length) */
	    /* TheParameter[0] gives the number of characters to erase */
	    eraseCharacter( this, TheParameter[0], theOutput);
	  break;
	case ED: /* Clear the display after the cursor */
	  {
	    struct t_cursor aFirstCursor;
	    struct t_cursor aLastCursor;

	    switch( TheParameter[0])
	      {
	      case 1: /* erase from start to cursor */
		aFirstCursor.myLine=0;
		aFirstCursor.myCol=0;
		aLastCursor.myLine = this->myCursor.myLine;
		aLastCursor.myCol = this->myCursor.myCol;
		break;
	      case 2: /* whole display */
		aFirstCursor.myLine=0;
		aFirstCursor.myCol=0;
		aLastCursor.myLine=this->myNumberOfLine - 1;
		aLastCursor.myCol=this->myNumberOfCol - 1;
		break;
	      case 0: /* from cursor to end of display */
	      default:
		aFirstCursor.myLine = this->myCursor.myLine;
		aFirstCursor.myCol = this->myCursor.myCol;
		aLastCursor.myLine=this->myNumberOfLine - 1;
		aLastCursor.myCol=this->myNumberOfCol - 1;
		break;
	      }
	    eraseCell( this, &aFirstCursor, &aLastCursor, &(this->myCursor.myStyle), theOutput);
	  }
	  break;
	case EL:
	  {
	    struct t_cursor aFirstCursor;
	    struct t_cursor aLastCursor;

	    aFirstCursor.myLine = this->myCursor.myLine;
	    aFirstCursor.myCol = this->myCursor.myCol;
	    aLastCursor.myLine = aFirstCursor.myLine;

	    switch( TheParameter[0])
	      {
	      case 1: /* erase from start of line to cursor */
		aLastCursor.myCol=0;
		break;
	      case 2: /* whole line */
		aFirstCursor.myCol=0;
		aLastCursor.myCol=this->myNumberOfCol - 1;
		break;
	      case 0: /* from cursor to end of line */
	      default:
		aLastCursor.myCol=this->myNumberOfCol - 1;
		break;
	      }
	    eraseCell( this, &aFirstCursor, &aLastCursor, &(this->myCursor.myStyle), theOutput);
	  }
	  break;
	case HOME:
	  aCursor->myCol=0;
	  aCursor->myLine=0;
	  break;
	case HPA:
	  aCursor->myCol=TheParameter[0] - 1;
	  break;
	case ICH: /* insert n characters */
	    insertCol( this, this->myCursor.myCol + TheParameter[0] - 1, &(this->myCursor.myStyle), theOutput);
	  break;
	case IL: /* several lines are added (the content is shifted to the bottom of the screen) */
	  break;
	case RC:
	  aCursor->myCol=this->mySavedCursor.myCol;
	  aCursor->myLine=this->mySavedCursor.myLine;
	  break;
	case SC:
	  this->mySavedCursor.myCol=aCursor->myCol;
	  this->mySavedCursor.myLine=aCursor->myLine;
	  break;
	case SGR:
	  copyStyle(& aCursor->myStyle, &TheCurrentStyle);
	  DISPLAY_STYLE( &TheCurrentStyle);
	  break;
	case VPA:
	  aCursor->myLine=(TheParameter[0] > 0) ? TheParameter[0] - 1 : TheParameter[0];
	  break;
	case TEXTFIELD:
	    setChar( this, (char)*yytext, theOutput);
	  break;
	default:
	case RMACS:
	case BEL:
	  break;
	}
    }

  flushPortion( this, theOutput);

  DISPLAY_BUFFER(this->myDataBuffer, this->myStyleBuffer, this->myNumberOfLine, this->myNumberOfCol);

  checkString( theOutput);
}
/* > */
/* < returnTheErasedChar */
void returnTheErasedChar( struct t_termbuffer* this, int theChoice)
{
  this->myErasedCharAreReturned=theChoice;
}
/* > */


