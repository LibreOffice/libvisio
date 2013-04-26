#!/usr/bin/env perl
#
# Version: MPL 1.1 / GPLv3.0+ / LGPLv3.0+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Thorsten Behrens <tbehrens@novell.com>
# All Rights Reserved.
#
# Contributor(s):
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.


$ARGV0 = shift @ARGV;
$ARGV1 = shift @ARGV;
$ARGV2 = shift @ARGV;

open ( TOKENS, $ARGV0 ) || die "can't open token file: $!";
my %tokens;

while ( defined ($line = <TOKENS>) )
{
    if( !($line =~ /^#/) )
    {
        chomp($line);
        @token = split(/\s+/,$line);
        if ( not defined ($token[1]) )
        {
            $token[1] = "XML_".$token[0];
            $token[1] =~ tr/\-\.\:/___/;
            $token[1] =~ s/\+/PLUS/g;
            $token[1] =~ s/\-/MINUS/g;
        }

        $tokens{$token[0]} = uc($token[1]);
    }
}
close ( TOKENS );

open ( HXX, ">$ARGV1" ) || die "can't open tokens.hxx file: $!";
open ( GPERF, ">$ARGV2" ) || die "can't open tokens.gperf file: $!";

print ( GPERF "%language=C++\n" );
print ( GPERF "%global-table\n" );
print ( GPERF "%null-strings\n" );
print ( GPERF "%struct-type\n" );
print ( GPERF "struct xmltoken\n" );
print ( GPERF "{\n" );
print ( GPERF "  const char *name;\n  int tokenId;\n" );
print ( GPERF "};\n" );
print ( GPERF "%%\n" );

print ( HXX "#ifndef __VSDXMLTOKENS_HXX__\n" );
print ( HXX "#define __VSDXMLTOKENS_HXX__\n" );
print ( HXX "\n" );

$i = 0;
foreach( sort(keys(%tokens)) )
{
    $i = $i + 1;
    print( HXX "const int $tokens{$_} = $i;\n" );
    print( GPERF "$_,$tokens{$_}\n" );
}
print ( GPERF "%%\n" );
print ( HXX "\n" );
print ( HXX "const int XML_TOKEN_COUNT = $i;\n" );
print ( HXX "\n" );
print ( HXX "const int XML_TOKEN_INVALID = -1;\n" );
print ( HXX "\n" );
print ( HXX "#endif\n" );
close ( HXX );
close ( GPERF );
