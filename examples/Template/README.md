# Template

A starting point for a GuitarML Funbox v3 pedal built on the Funbox SDK.

Uses the self-contained `funbox::Hardware` class from [`include/funbox.h`](../../include/funbox.h) —
no libDaisy patching required.

## Controls

| Control | Description | Comment |
| --- | --- | --- |
| Knob 1 | `param1` |  |
| Knob 2 | `param2` |  |
| Knob 3 | `param3` |  |
| Knob 4 | `param4` |  |
| Knob 5 | `param5` |  |
| Knob 6 | `param6` |  |
| 3-Way Switch 1 |  |  |
| 3-Way Switch 2 |  |  |
| 3-Way Switch 3 |  |  |
| Dip Switch 1 | Mono / Stereo out |  |
| Dip Switch 2 |  |  |
| FS 1 | Bypass toggle |  |
| FS 2 |  |  |
| LED 1 | Bypass indicator |  |
| LED 2 |  |  |
| Expression | v3 only |  |

## Build

From this folder, with the submodules initialized:

```sh
# One-time: build the libraries
make -C ../../libs/libDaisy
make -C ../../libs/DaisySP

# Build and flash the pedal
make
make program-dfu
```
