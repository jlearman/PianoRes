#!/usr/bin/env bash

# After building artifacts using Visual Studio, build a zip file
# in Artifacts folder, for associating with a release tag.

# After building Zynthian, copy PianoRes.lv2 to Artifacts folder,
# and run with -z option.

ZYN=false
if [[ $1 == "-z" ]] ; then
    ZYN=true
    shift
fi

if [ $# -lt 1 ] ; then
    echo "expecting version number, with v prefix"
    exit 1
fi


VERSION=$1

if $ZYN ; then
    FORMAT=lv2
    ARCH=zyn-$FORMAT
    BUILDDIR=Artifacts
else
    FORMAT=vst3
    ARCH=win-x64-$FORMAT
    BUILDDIR=Builds/VisualStudio2026/x64/Release/VST3/
fi

ZIP=PianoRes-$VERSION-$ARCH.zip

set -ex
mkdir -p Artifacts
rm -f Artifacts/$ZIP
zip -r Artifacts/$ZIP ImpulseFiles

cd $BUILDDIR
zip -r $OLDPWD/Artifacts/$ZIP PianoRes.$FORMAT
cd -

cd release-readme
zip -r $OLDPWD/Artifacts/$ZIP README-$ARCH.txt
