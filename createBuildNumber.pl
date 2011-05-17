#!/usr/bin/perl
# ***** BEGIN LICENSE BLOCK *****
# Version: LGPL 2.1
#
# The Original Code is Mozilla Calendar Code.
#
# Copyright (C) 2002 Christopher S. Charabaruk (ccharabaruk@meldstar.com).
# Copyright (C) 2004 Fridrich Strba (fridrich.strba@bluewin.ch).
# All Rights Reserved.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# For further information visit http://libwpd.sourceforge.net
#
#
# "This product is not manufactured, approved, or supported by 
# Corel Corporation or Corel Corporation Limited."
#
# ***** END LICENSE BLOCK *****

# create a build id to be used by build/win32/compile-resource
@timevars = localtime(time);
$buildid  = sprintf("%1.1d%.2d%.2d", ($timevars[5] - 100) , ($timevars[4] + 1) , $timevars[3]);

#print our build id
print $buildid . "\n";

foreach $file (@ARGV)
{
	# print filename
	print "Working on " . $file . "\n";
	
	open(OUT,">" . $file) or die "cannot open " . $file . "-temp for write\n";
	print OUT $buildid . "\n";
	close (OUT);
}

print "All done!\n";
