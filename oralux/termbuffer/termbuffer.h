#ifndef TERMBUFFER_H
#define TERMBUFFER_H

#include <wchar.h>
#include "escape2terminfo.h"

/* Current supported terminals */
enum termbufferName
  {
    LINUX,
    XTERM
  };

/* < cursor */
struct t_cursor
{
  int myLine;
  int myCol;
  struct t_style myStyle;
  mbstate_t myEncodingState; /* Useful for stateful encoding */
};

#define copyCursor( theDestination, theSource) memcpy( theDestination, theSource, sizeof(struct t_cursor))
/* > */
/* < linePortion */
/* 
A line portion describes an horizontal area in the screen.
When a menu is browsed, the selected item and the previously ones are displayed with distinct styles (for example distinct backgrounds).
These two line portions must be particularly managed to distinguish the selected one.
*/
struct t_linePortion
{
  int myLine; /* Line number */
  int myFirstCol; /* First column of the portion */
  int myLastCol; /* Last column of the portion */
  int myContentIsModified; /* equals 1 if at least one char has been modified */
  int myStyleIsModified; /* equals 1 if one of the style has been modified */
};

enum {MAX_LINE_PORTION=2}; /* 2 line portions are expected to distinguish the selected item */
/* > */
/* < t_termbuffer */
typedef char chartype; /* TBD: add future support for multi-bytes characters */

struct t_termbuffer
{
  chartype **myDataBuffer; /* data of the screen, one data buffer per row */
  struct t_style** myStyleBuffer; /* style of the screen, one style buffer per row */
  struct t_style myDefaultStyle; /* used if car are deleted */
  struct t_cursor myCursor;
  struct t_cursor mySavedCursor;
  int myNumberOfLine;
  int myNumberOfCol;
  struct t_linePortion myLinePortion[ MAX_LINE_PORTION]; /* the data of a line from the received escape sequence */
  int myLinePortionIndex;
  int myErasedCharAreReturned; /* if 1, theOutput will include any erased char. Useful for retreiving the single erased char when the backspace is pressed */
  int myCarry; /* greater than 0 if the current line is greater than a screen line. */
  int *myTab; /* one bool per column. true means: this is a tab position */
};
/* > */
/* < createTermBuffer */
/* To be called before any other function, return a pointer on the 'object' (this) */
struct t_termbuffer* createTermbuffer( enum termbufferName theName, int theNumberOfLine, int theNumberOfCol);

/* > */
/* < interpretEscapeSequence */
/* Interprets the escape sequence, update the internal data and style buffers.
   theOutput gives the chars to be rendered 
*/
void interpretEscapeSequence( struct t_termbuffer* this, FILE* theStream, chartype** theOutput);

/* > */
/* < returnTheErasedChar */
/* theChoice must be equal to 1 if the erased chars must be also returned in theOutput of interpretEscapeSequence */
void returnTheErasedChar( struct t_termbuffer* this, int theChoice);

/* > */
/* < deleteTermbuffer */
/* To be called after any other function */
void deleteTermbuffer( struct t_termbuffer* this);

/* > */

/* 
Local variables:
folded-file: t
folding-internal-margins: nil
*/

#endif