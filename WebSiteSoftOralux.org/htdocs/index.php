<?PHP
// ----------------------------------------------------------------------------
// index.php
// $Id: index.php,v 1.1 2005/05/14 01:32:06 gcasse Exp $
// $Author: gcasse $
// Description: homepage
// $Date: 2005/05/14 01:32:06 $ |
// $Revision: 1.1 $ |
// Copyright (C) 2003 Gilles Casse (gcasse@oralux.org)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ----------------------------------------------------------------------------

require_once("include/webpage.php");
#require_once("prepend.php");

$aStyle[0]="home.css";
$aWebPage=new webPage("mcvox", "$Date: 2005/05/14 01:32:06 $", $aStyle);

// Printing the web page
$aWebPage->printTag("htm/index.htm");
echo "$BASE";

?>
