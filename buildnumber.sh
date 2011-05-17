#!/bin/sh

# Script to create the BUILDNUMBER used by compile-resource. This script
# needs the script createBuildNumber.pl to be in the same directory.

{ perl ./createBuildNumber.pl \
	src/lib/libvisio-build.stamp \
	src/lib/libvisio-stream-build.stamp \
	src/conv/raw/visio2raw-build.stamp \
	src/conv/svg/visio2svg-build.stamp
#Success
exit 0
}
#unsucessful
exit 1
