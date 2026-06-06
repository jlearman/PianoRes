#!/usr/bin/env bash

# After building artifacts using Visual Studio, build a zip file
# in Artifacts folder, for associating with a release tag.

ZIP=PianoRes-win-x64-vst3.zip
set -ex
mkdir -p Artifacts
rm -f Artifacts/$ZIP
zip -r Artifacts/$ZIP ImpulseFiles

cd Builds/VisualStudio2026/x64/Release/VST3/
zip -r $OLDPWD/Artifacts/$ZIP PianoRes.vst3
cd -

cd release-readme
zip -r $OLDPWD/Artifacts/$ZIP README-win-x64-fst3.txt
