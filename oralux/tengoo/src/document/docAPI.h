#ifndef DOCAPI_H
#define DOCAPI_H
#include <glib.h>
#include "lineportion.h"
#include "frame.h"

enum elementType 
  {
    rootType=1,
    textType=2,
    linkType=4,
    frameType=8,
    anyNotRootType =14, 
  };

void* createDocAPI( int theVoiceVolume);
void deleteDocAPI( void* theDocAPI);

/* < addFrameDocAPI */
/* add a Frame style to the document  */
void addFrameStyleDocAPI( void* theDocAPI, GList* theFrame);
/* > */
/* < appendListEntryDocAPI */
/* append the list of terminfoEntries.
Each text entry will be associated to a text element (linked to the containing frame). 
If an entry belongs to two or more frames, it will be split so that each resulting entry is included in one frame.

The supplied list will be deleted by docAPI.
*/
void appendListEntryDocAPI( void* theDocAPI, GList* theList);
/* > */
/* < setElementTypeDocAPI */
/*
  Must be called after putListEntryDocAPI to enrich the type of an entry 
*/
void setElementTypeDocAPI( void* theDocAPI, GList* theEntry, enum elementType theType);
/* > */
/* < setFocusStateDocAPI */
/*
  Indicates for example that a link is hovered.
*/
enum elementFocusState
  {
    notHoveredElement=0,
    hoveredElement=1,
    activeElement=2,
    focusedElement=4,
  };

void setFocusStateDocAPI( GList* theEntry, enum elementFocusState theState);
/* > */
/* < getListEntryDocAPI */
/*
returned the list in taking in account the styles. 
The list will be deleted by docAPI.
*/
GList* getStyledListEntryDocAPI( void* theDocAPI);
/* > */
/* < loadStyle */
void loadStyleDocAPI( void* theDocAPI, char* theFilename);
/* > */
/* < clearContentDocAPI */
/*
clear any textual info (text and link nodes, entry list,...).
The layout is not concerned (frame nodes,...).   
*/
void clearContentDocAPI( void* theDocAPI);
/* > */
/* < getFramesDocAPI */
/* return a list of frame names (char*). 
   These frames are siblings of the current level. 
*/
GList* getFramesDocAPI();
char* getHoveredFrameDocAPI();
void setFocusOnFrameDocAPI( char* theFrameName, enum elementFocusState theState);

/* > */

/* 
Local variables:
folded-file: t
folding-internal-margins: nil
*/

#endif
