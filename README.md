# PianoRes

PianoRes is an audio plugin that adds damper resonance to sampled pianos, built using JUCE.
It is designed to enhance the realism of sampled pianos by simulating the
sympathetic vibrations that occur when the sustain (damper) pedal is pressed.
This plugin can be used in digital audio workstations (DAWs) or live hosts to
increase realism.

Built using the JUCE framework, PianoRes is compatible with various plugin
formats, including LV2, VST, VST3, and AU.

Currently under development.  Don't bother trying it yet!

## Usage

Plug PianoRes to process the audio output your piano sample player
(sfizz, Sforzando, etc.) Unfortunately, neither sfizz nor Sforzando
passes MIDI through, so you will need to do some MIDI or audio
routing on your host.

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

## Controls

The plugin has a file chooser to select the impulse response (IR) file,
which is made from summing all the notes of a piano, ideally at low velocity.
The plugin also has a mix control to adjust the balance between the dry signal
(the original piano sound) and the wet signal (the damper resonance effect).
The IR file should be a stereo audio file (e.g., WAV, AIF, OGG, or FLAC.)

The longer the IR file, the more realistic the damper resonance will sound,
but it will also use more CPU resources. A good starting point is to use an
IR file that is around 5-10 seconds long.

Several IR files will be included with the plugin, but you can also create your
own by recording all notes a piano and summing them together in a DAW, and
normalizint it.

The plugin also has a release time control, which adjusts how quickly the
damper resonance fades out as the sustain pedal is released.  Half-pedaling
will hopefully be supported in the future.

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