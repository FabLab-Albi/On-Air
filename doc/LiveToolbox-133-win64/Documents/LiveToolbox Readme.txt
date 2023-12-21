Title: X32 Live Toolbox
Author: Paul Vannatto  (donations accepted at paypal.me/PVannatto)
Licence: GPL 3.0
OS Platform: Windows, linux, RPi, OSX
Version: 1.33

** NEW **
* Added NewFade checkbox to Setup
* Added Matrix, Main LR and Main M/C to Groups (DCA, Mutegroup) tool (requires firmware 4.06)
* Please see the "Revisions.txt", "English Commands Supplement.pdf" or "X32 Live Toolbox User Manual.pdf" for details

Features:
Interface
* Show/hide header panel
* Option to minimize Live Toolbox when popup Tidbit button is used.
* Custom Button grid - assign both OSC and tidbit commands, unlimited quantity.
* Hide/unhide tabs (tools) for customization (turnkey installs)
* Password for Setup screen
* Testing screen can now send tidbit commands
* Added subtabs to CopyEQ and Groups screens to reduce minimum screen size

OSC engine
* Connection commands - Connect, Search, Disconnect
* Send and receive OSC commands to/from the X32.
* Toggle enumerated values (on/off, colors, etc) through a set range.
* Percentage increase/decrease float values.
* Db increase/decrease float values.
* Linear type values (l).
* Bitmap type values (%).
* Multi-value OSC commands can now be sent (eg. /copy, /load, /save, etc.)
* Streaming returns can now be received.
* Turn off sending to X32 (for testing tidbits, etc.)
* OscTxRx engine is a separate unit that can be used for other OSC development.
* Search IP. Provide port # (10023) and it will search and connect.
* Store commands and values internally (0..9) for later recall
* Save all global, network, remote, config, etc. console settings to snippet or tidbit
  (using a combination of store and recall tidbit commands)
* Reset gain, gate, comp, fader, bus/fx sends in proportion
  (or inverse) to gain or fader change
* Command scope (# substitution) can be single, range or non-consecutive - delimited by commas, no spaces
  (eg 1-4,6,9-12)
* User In/Out patching English equivalent commands.
* DP48 English equivalent commands.
* Mutegroup on/off English equivalent command

Specialized Tools
* Copy graphic EQ settings from one side and/or slot to another, each side independent of the other
* DCA/Mute Group analysis and modification
* Headamp copy tool with visual phantom (LEDs) and gain (text)
* Input-Output mute tool to change mutes and/or create mute tidbits
  (which can be converted to snippets)
* OSC Command list for both X32 and X-Air are now available with type, value, range and description.
  These have been incorporated into the Testing screen.
* X32 Command Editor (from Sysex OSC Generator app)

Tidbits and Cabinets
* Tidbits (similar to snippets) and cabinets of tidbits (similar to shows), with sections.
* Input-output fader values can now be analysed and changed in OSC or db levels
  and in absolute or relative values. The results can be sent to the X32 or transferred
  to the tidbit editor.
* Optional Tidbit launch button (popup) to launch tidbits by mouse click or spacebar.
* Optional advance to next tidbit.
* Tidbit popup now shows previous and next tidbits. Ability to move up/down tidbit list when popup is visible.
* Many specialized tidbit commands, including:
  Delay, Buffer, Pause, TapTempo, Go, Load, Save, Store, Recall, Launch, Fade, XFade, Wave, Update, Reset, Connect, Search, Disconnect, etc.
* English equivalent commands - see "English Commands Supplement.pdf"


The X32 Live Toolbox is a work in progress.

The initial launching provides the ability to send any command to an X32, either requesting current status or to change any parameters using OSC commands. This can be accomplished on the Testing screen.

Copy EQ allows copying the 32 values (31 bands plus gain) of the various graphics EQ's (dual graphic, stereo graphic, dual true and stereo true EQ's) from one side and/or slot to another.

DCA/Mute Group analysis is a matrix of 16 channels by 8 DCA plus 6 Mute groups. The 4 banks (similar to the left 4 banks on the X32) provide the ability to see all 32 channels, 8 Aux, 8 FxRtn, and 16 mixbuses. Changes of state (On/Off) can be accomplished by simply checking/unchecking the respective checkboxes. Once everything is setup as desired, the changes can then be sent to the X32.

Headamp copy tool copies headamps from one block of 8 to another. This can be repeated as necessary.

Input-Output mute and fader level tools provide the ability to load appropriate values in blocks of 8, change the values, then send the changes to the X32. These values can also be used to create mute tidbits, which can be converted to snippets if desired.

Tidbits function in the same manner as snippets, but use the OSC single-parameter format (command, type, parameter). In contrast, snippets use the multiple-parameter format that is also used by scenes. Cabinets are simply a list of tidbits, similar to shows (of scenes). Tidbits can be converted to snippets (with some effort - this will be enhanced in the future).

Snapshots of various current states (of the X32) can now be captured and added to tidbits for future execution. Enumerated type values (on/off, pre/post, colors, etc.) can now be toggled through a set range.
Float values (fader, gain, threshold, etc.) can be increased/decreased by percentage, db or changed by linear db.

