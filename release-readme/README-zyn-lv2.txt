PianoRes installation:

Copy PianoRes-zyn-lv2.zip/PianoRes.lv2 folder into zynthian lv2 plugin
folder, which is usually /zynthian/zynthian-plugins/lv2 .
Using Webconf, go to SOFTWARE -> Engines and click "Scan for Presets."
Then restart Zynthian.

Alternatively, go to SOFTWARE -> Engines and click "Install Plugin."

Loading of impulse files is not yet supported, due to Zynthian limitations.
Maybe someday.  PianoRes already includes the
short impulse file for Accurate Salamander Grand 6.2beta2 and uses it by
default, without copying the impulse files that happen to be
included in the zip file.

v0.2.1:
- changed impulse files to 48KHz

Known issues:
- Should be used at 48kHz (or whatever sample rate the IR file uses.)
  Otherwise audio effect is a garbled because the IR file is
  not yet converted to the active sample rate.  Issue #4
