# PianoRes

PianoRes is an audio plugin that adds damper resonance to sampled pianos.
It is designed to enhance the realism of sampled pianos by simulating the
sympathetic vibrations that occur when the sustain (damper) pedal is pressed.
This plugin can be used to increase realism in digital audio workstations (DAWs)
or live hosts.

Built using the JUCE framework, PianoRes is compatible with various plugin
formats, including LV2, VST, VST3, and AU, on various platforms.

It is currently available for Windows 64 bit (VST3) and Zynthian
(Debian Bookworm on ARM, as an LV2 plugin.)

## Installation

Get the plugin, example IR files, and README by clicking the Release
link at the right of the github Code page.  These are in a zip file.
Follow the instructions in the README file.

## Usage

Plug PianoRes to process the audio output your piano sample player
(sfizz, Sforzando, etc.) Unfortunately, neither sfizz nor Sforzando
passes MIDI through, so you will need to do some MIDI or audio
routing on your host.  There's a trick to get Sforzando to pass MIDI,
see below.

One option is to use two chains or tracks.  On one track, plugin the player.
On the other track, plugin PianoRes.  Set both to receive MIDI from the same
source, and patch the audio output of the player to the input of PianoRes.

Another option is to use one chain or track.  Plugin the player, then plugin
PianoRes after it.  Using host routing, patch the MIDI input into PianoRes.

(I will be submitting a feature request to sfizz to add MIDI passthrough,
which would make this much easier.)

PianoRes will work best with sampled pianos that do not already have damper
resonance samples.  Most of these have controls to adjust the amount of
damper resonance, which you can set to zero.  If your piano does have
damper resonance samples and you can't easily disable them, you can still
use PianoRes. [Note: editing the sfz file you can remove the resonance
sample groups.]

### Windows: Getting Plogue Sforzando to pass MIDI

This works only with the VST (not VST3) version of Sforzando.  It might
also work for LV2, but I haven't tried it.

Copy Resources/gui-settings.xml from this repo into the Sforzando GUI
directory (usually, "C:\Program Files\Plogue\Sforzando\GUI".)
In Sforzando GUI, choose the SETTINGS tab, and set MIDI to "Process."

Then you can simply plug PianoRes following Sforzando, and ignore
the alternatives below.

### Windows: Usage in Reaper, without Sforzando trick above

1. Add a track and plug in your sample player configured with your sampleset (e.g.,
    Sforzando with Accurate Salamander Grand 6.2beta2.)
2. Add another track and plug in PianoRes.  Load the impulse file. (Any piano resonance
   impulse file will do, but ideally one created with your piano sampleset.)
3. Set PianoRes dry control to 0%
4. Click the piano track's "ROUTE" button and click "Add new send", and choose
   the PianoRes Track
5. Control the amount of resonance using the PianoRes track's volume slider
6. To hear the effect only, click the piano track's ROUTE button and uncheck
   "Master Send"

### Zynthian

PianoRes has been tested on RPi5 only.  Good luck on RPi4.

#### Zynthian Audio buffering

Set audio buffer size to 256 samples, with 3 buffers.  Both 44100 Hz and 48000 Hz
have been tested and work.  YMMV depending on your audio interface.  If you get
constant overruns/crackling, increase buffering.

#### Zynthian Chain setup

Load the PianoRes plugin as an Audio+MIDI chain.
Load the piano (Ideally, Salamander Grand, but any piano should be OK) on another
chain, and set that chain's audio output to the PianoRes chain.  Disable the chain's
output going to the mixer.

#### Zynthian Limitations

On other platforms, it's possible to load the IR file, to use one based on your
sampled piano.  Zynthian doesn't provide access to the file chooser, so it's
not possible the normal way.  Copy the desired IR file to
`/zynthian/zynthian-my-data/files/IRs/PianoResIR.flac`
All instances of PianoRes will use this IR file.

## Controls

The plugin has a file chooser to select the impulse response (IR) file,
which is made from summing all the notes of a piano, ideally at low velocity.
An impulse file for AccurateSalamanderGrand 6.2beta2 is included by default.
The plugin also has gain controls for the dry signal (the original piano sound)
and the wet signal (the damper resonance effect).
The IR file should be a stereo audio file (e.g., WAV, AIF, OGG, or FLAC.)

The longer the IR file, the more realistic the damper resonance will sound,
but it will also use more CPU resources. A good starting point is to use an
IR file that is around 5-10 seconds long.

Several IR files are included with the plugin, and you can create your
own by recording all notes on a piano and summing them together in a DAW,
and normalizint it.

The plugin has a release time control, which adjusts how quickly the
damper resonance fades out as the sustain pedal is released.

[NYI] Half-pedaling

## BUILDING

I haven't tested this, but this should work on Windows.  Small changes would be
needed for Linux/Mac.

1. Requirements:
   1. JUCE (install at C:\JUCE on Windows, for easiest)
   2. Visual Studio 2026 (Community edition fine.  I haven't tried VSCode.)
   3. this repo
2. Run JUCE "ProJucer" and load the "PianoRes.jucer" project
3. Click the VS button to the right of "Selected exporter: Visual Studio 2026".  That opens VS with the project
4. Build->Build Solution
5. Hunt down the plugin you want (currently LV2 or VST3) and install it in your system.  On a Mac you might be able to build AU, though you might need to change Jucer options.

## License

The source code for PianoRes is licensed under the GIT "Unlicense", which
means it is free to use, modify, and distribute without any restrictions.
You can find the full text of the license in the LICENSE file included
with the source code.

Attibution is not required, but would be appreciated.

## Credits

Big thanks to https://github.com/etosphere, whose Coneko plugin helped me
quickly understand how to use a convolution engine in JUCE.

Thanks also to the developers of JUCE (https://juce.com/), which is an
amazing framework for building audio plugins.
