# Building the PSP EBOOT.PBP

This project is built for the PlayStation Portable using the PSP SDK and the included Makefile.

## Requirements

Before building, make sure you have:

- PSP SDK / `pspsdk` installed
- `psp-gcc`, `make`, and the PSP toolchain available in your `PATH`
- A terminal opened in the project directory that contains the `Makefile`

Typical setup on Linux/macOS:

```bash
export PSPDEV=/usr/local/pspdev
export PATH="$PSPDEV/bin:$PATH"
```

If your toolchain is installed in a different location, adjust the paths accordingly.

## Build

From the project directory, run:

```bash
make
```

This will compile the program and package it into a PSP executable named:

```bash
EBOOT.PBP
```

The generated file is usually placed in the project root, alongside the source files and the Makefile.

## Clean build artifacts

To remove generated objects and rebuild from scratch:

```bash
make clean
make
```

## Output

After a successful build, copy `EBOOT.PBP` to your PSP memory stick under:

```text
PSP/GAME/<your_game_name>/EBOOT.PBP
```

Then launch the game from the PSP home screen.

## Troubleshooting

- If `make` fails with a missing compiler or linker, verify that the PSP toolchain is installed and in `PATH`.
- If `EBOOT.PBP` is not created, check the build log for linker or packaging errors.
- Make sure your project contains a valid `Makefile` and PSP-compatible source structure.

This is the standard workflow for building a PSP homebrew project using the project Makefile.
