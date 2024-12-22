# Cave Story speedrun patcher

A program for applying or reverting the allowed speedrunning patches (see below),
for those who don't want to use a hex editor to do it themselves.

## Patches that this applies

Each one of these can be toggled individually.
(Original byte values are given for reference.)

* Keeps the game running even when not in focus:
```
0x13593: 75 -> 71
0x12BC0: 55 -> C3
0x12BF0: 55 -> C3
```
* Changes the 320x240 windowed mode to multiply it by a scale factor:
```
Example for 3x scale (960x720):
0xB4A6: 01 -> 03           (3x scale)
0x127C2: 40 01 -> C0 03    (little-endian, 0x3C0 = 960)
0x127CC: F0 00 -> D0 02    (little-endian, 0x2D0 = 720)
These last two edits fix the font size and are the same regardless of the scale factor:
0xCD6B: 1E -> 10
0xCD7C: C7 45 F8 14 ... -> 6B C8 0A 89 4D F8 6B D0 05 89 55 FC 90 90
```
* Automatically advances text after a 2-frame delay when holding Z or X:
```
0x16CA0: 55 8B EC 0F B6 05 ... -> B8 09 5B 4A 00 FE 00 82 38 02 7E 0A C6 05 DC 5A 4A 00 01 C6 00 00 C3
0x25584: 14 -> 10
0x2558A: C6 05 DC 5A 4A 00 01 -> E8 11 17 FF FF 90 90
```

## Usage/Building

Most people can just go to the Releases page and download the patcher from there.

If you want to build this yourself, simply compile all of the .cpp files together
with a C++17-compatible compiler.
On Windows, you may also need to link with the Comdlg32 library (`-lcomdlg32` for MinGW,
`Comdlg32.lib` for MSVC).
This project has no other external dependencies.