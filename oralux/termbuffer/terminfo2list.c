/* 
----------------------------------------------------------------------------
terminfo2list.c
$Id: terminfo2list.c,v 1.1 2005/07/10 20:33:57 gcasse Exp $
$Author: gcasse $
Description: convert the terminfo entries to a list of commands.
$Date: 2005/07/10 20:33:57 $ |
$Revision: 1.1 $ |
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

#include "terminfo2list.h"

/* < handlers type*/
/* < createEntry, deleteEntry */
/* < create_SGR_Entry */
/* < create_CUP_Entry */
/* < addChar */
/* < array of entries */
/* < convertTerminfo2List */

GList* convertTerminfo2List( FILE* theStream)
{
  enum StringCapacity aCapacity;
  GList* aList=NULL;

  ENTER("convertTerminfo2List");

  yyin=theStream;

  while((aCapacity=yylex()))
    {
      entryCommands* aCommand=NULL;
      terminfoEntry* anEntry=NULL;

      DISPLAY_CAPACITY( aCapacity);

      aCommand=TheEntryCommands+aCapacity;

      /* check consistency */
      if ((aCapacity > LASTENUM)
	  || (aCommand->myCreateEntryHandler == NULL)
	  || (aCommand->myDeleteEntryHandler == NULL)
	  || (aCommand->myCapacity != aCapacity)
	  )
	{
	  exit(0);
	}

      anEntry = aCommand->myCreateEntryHandler( aCapacity, aCommand->myData1, aCommand->myData2);



      aCommand->myDeleteEntryHandler( anEntry);
    }



    }
  return aList;
}

/* > */