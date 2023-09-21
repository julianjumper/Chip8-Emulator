# `Chip8-Emulator` - first attemps with emulation
> "CHIP-8 is an interpreted programming language, developed by Joseph Weisbecker made on his 1802 Microprocessor."

This project is a CHIP-8 virtual machine that runs CHIP-8 programs. CHIP-8 is an assembler-like language using 2-byte-long opcodes. <br>
This emulator can be thought of a processor with its own registers, memory, clock etc, designed for running CHIP-8 programs.
Initially, CHIP-8 was made in the 1970s to program video games more easily and was later used to run video games on graphing calculators, especially those of Texas Instruments. 

---

## Installation
Just install the package, unzip the archive and open a program (any `.ch8`-file) by drag-and-dropping it onto the `Chip-8.exe`.
![screenrecording pong](https://github.com/jmjumper/Chip8-Emulator/blob/master/screen/chip8.gif)

--- 

## Games inside ROM-folder
### Pong
- Move your player with the `arrow-keys` or `1 (up)` and `Q (down)`
### Tron (two-players)
- restart: `C`
- start: `X`
- player 1: moves with `arrow-keys`
- player 2: moves with `i, j, k , l`
### Space-Invaders
- left/right: `Q`/`E`
- shoot: `W`
### Tetris
- rotate: `Q`
- left/right: `W`/`E`
- faster: `A`
### More
more can be found at https://github.com/kripod/chip8-roms

---

## Sources that I mainly used
- <a href="http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#2.0">Cowgod's Chip-8 Technical Reference v1.0</a> by <u>Thomas P. Greene</u> <br>
- https://en.wikipedia.org/wiki/CHIP-8 <br>
- https://github.com/kripod/chip8-roms - Games in ROMs folder<br>
- https://github.com/Timendus/chip8-test-suite - Test ROMs in ./roms/test folder
