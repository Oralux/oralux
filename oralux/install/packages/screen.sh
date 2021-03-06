#! /bin/sh
# ----------------------------------------------------------------------------
# screen.sh
# $Id: screen.sh,v 1.3 2005/08/06 22:06:32 gcasse Exp $
# $Author: gcasse $
# Description: Installing Screen
# $Date: 2005/08/06 22:06:32 $ |
# $Revision: 1.3 $ |
# Copyright (C) 2004, 2005 Gilles Casse (gcasse@oralux.org)
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
export SCREEN_RELEASE=4.0.2

####
# Installing the package in the current tree
InstallPackage()
{
    apt-get --purge remove screen
    cd /tmp
    rm -rf screen*
    wget ftp://ftp.cs.univ-paris8.fr/mirrors/ftp.gnu.org/screen/screen-$SCREEN_RELEASE.tar.gz
    tar -zxvf screen-$SCREEN_RELEASE.tar.gz

    # accessible window list
    patch -p0 < $INSTALL_PACKAGES/screen/screen.patch

    cd screen-$SCREEN_RELEASE
    ./configure --prefix=/usr
    mv Makefile Makefile.tmp
    cat Makefile.tmp | sed -s "s+OPTIONS=+OPTIONS= -DCARET_MODE+g" > Makefile
    rm Makefile.tmp
    make
    make install
    echo "autodetach on" >> /etc/screenrc

    cd /tmp
    rm -rf screen*
}

####
# Adding the package to the new Oralux tree
Copy2Oralux()
{
    export INSTALL_PACKAGES=/usr/share/oralux/install/packages

    chroot $BUILD  bash -c "\
    apt-get --purge remove screen;\
    cd /tmp;\
    rm -rf screen*;\
    wget ftp://ftp.cs.univ-paris8.fr/mirrors/ftp.gnu.org/screen/screen-$SCREEN_RELEASE.tar.gz;\
    tar -zxvf screen-$SCREEN_RELEASE.tar.gz;\
    patch -p0 < $INSTALL_PACKAGES/screen/screen.patch;\
    cd screen-$SCREEN_RELEASE;\
    ./configure --prefix=/usr;\
    mv Makefile Makefile.tmp;\
    cat Makefile.tmp | sed -s \"s+OPTIONS=+OPTIONS= -DCARET_MODE+g\" > Makefile;\
    rm Makefile.tmp;\
    make;\
    make install;\
    echo \"autodetach on\" >> /etc/screenrc"
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
