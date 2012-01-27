#!/bin/sh

# Script to create the BUILDNUMBER used by compile-resource. This script
# needs the script createBuildNumber.pl to be in the same directory.

{ perl ./createBuildNumber.pl \
	src/lib/libvisio-build.stamp \
	src/conv/raw/vsd2raw-build.stamp \
	src/conv/svg/vsd2xhtml-build.stamp
#Success
exit 0
}
#unsucessful
exit 1
