# Interactive Braille Firmware

Arduino firmware for the ELEN30013 interactive Braille learning device.

## Braille bit convention

Each character is stored in the lower six bits of an unsigned byte. Bit 0
represents dot 1, bit 1 represents dot 2, and so on through bit 5 for dot 6.

```text
Physical cell       Stored bits

1 4                 bit 0  bit 3
2 5                 bit 1  bit 4
3 6                 bit 2  bit 5
```

For example, the letter C uses dots 1 and 4, so its stored value is
`0b001001`.

## Code Structure

All firmware is in `firmware/interactive_braille/`:

- `interactive_braille.ino` — Handles sensors, buttons, servos, and audio.
- `BraillePatterns.h/.cpp` — Stores the Braille patterns for A–Z.
- `BrailleTrainer.h/.cpp` — Manages modes, targets, and answer checking.
- `HardwareConfig.h` — Defines hardware pins and settings.
