#! /bin/sh
# ----------------------------------------------------------------------------
# lliaphon.sh
# $Id: lliaphon.sh,v 1.1 2005/12/10 14:37:45 gcasse Exp $
# $Author: gcasse $
# Description: Installing LLiaPhon
# $Date: 2005/12/10 14:37:45 $ |
# $Revision: 1.1 $ |
# Copyright (C) 2004, 2005 Nath (nath.ml at free.fr)
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# ----------------------------------------------------------------------------
####
source ../oralux.conf
export REL="0.4"
export URL="http://download.gna.org/lliaphon/stable/lliaphon-${REL}.tar.gz"

####
# Installing the package in the current tree
InstallPackage()
{
# Installing LLiaPhon
cd /tmp
wget $URL
tar xvzf lliaphon-${REL}.tar.gz
cd /tmp/lliaphon-$REL
./configure --enable-lex80k
make
make install
#echo "export LLIAPHON=/usr/local/lliaphon" >>/etc/profile
rm -rf /tmp/lliaphon*
}

####
# Adding the package to the new Oralux tree
Copy2Oralux()
{
# Installing LLiaPhon
cd $BUILD/tmp
wget $URL
tar xvzf lliaphon-${REL}.tar.gz
chroot $BUILD bash -c "cd /tmp/lliaphon-$REL;./configure --enable-lex80k;make;make install"

#echo "export LLIAPHON=/usr/local/lliaphon" >>/etc/profile
rm -rf $BUILD/tmp/lliaphon*
}

case $1 in
    i|I)
        InstallPackage
        ;;
    b|B)
        Copy2Oralux
        ;;
    *)
        echo "I (install) or B(new tree) is expected"
        ;;
esac

exit 0