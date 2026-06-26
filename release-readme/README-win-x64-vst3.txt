PianoRes installation:

Copy PianoRes-win-x64-vst3.zip/PianoRes.vst3 folder into your
computer's VST3 folder, which is usually "C:/Program Files/Common/VST3".

Optionally copy PianoRes-win-x64-vst3.zip/ImpulseFiles to any accessible
location, for loading into PianoRes.  PianoRes already includes the
short impulse file for Accurate Salamander Grand 6.2beta2 and uses it by
default, without copying these files.

v0.2.1:
- changed impulse files to 48KHz

Known issues:
- Must be used at 48kHz (or whatever sample rate the IR file uses.)
  Otherwise audio effect is a garbled because the IR file is
  not yet converted to the active sample rate.
