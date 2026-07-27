# MakePri

[![Discord Invite][2]][1]

`MakePri.exe` for Project Reality.

## Overview

This project aims to be a drop-in replacement for `MakePri.exe`.

It reuses Microsoft's open-source
[MRT Core](https://github.com/Microsoft/WindowsAppSDK/tree/main/dev/MRTCore) and reimplements
the missing `MrmIndexer` and `MakePri` CLI components.

While the tool is currently Windows-only, we aim to build a UNIX port, allowing it to be used in the
cross-compilation of UWP applications.

## Building

This project uses CMake and assumes an LLVM toolchain.

## Testing

This project has several unit tests and extensive integration test coverage.

### Integration Testing

Integration tests are run by default with `ctest`. They can also be invoked by running the
`makepri_integration_tests` target.

#### With Stored Data

By default integration tests check the observed output with sample data stored in the repository.

The data covers:
- `stdin`, `stdout`, `stderr`.
- Exit code.
- Input files and output artifacts.

The stored data is extracted from official `MakePri.exe` runs - see below.

#### With Official Binary

To compare the built `MakePri.exe` with an official copy of the tool, either:
- At configure time, pass the full path to the official executable to
`MAKEPRI_TEST_OFFICIAL_EXECUTABLE`, or
- When invoking `makepri_integration_tests`, pass the official executable path to
`--makepri-offical`.

Both the built `MakePri.exe` and the official copy will have their outputs compared with the stored
data.

To update the stored data using official output (such as after adding a new test), set
`MAKEPRI_UPDATE_INTEGRATION_OUTPUTS` in CMake or pass `--makepri-update-outputs` to the test binary.

## Community

This repo is a part of [Project Reality][1].

Need help using this project? Join me on [Discord][1], and let's find a solution together.

[1]: https://reality.trungnt2910.com/discord
[2]: https://img.shields.io/discord/1185622479436251227?logo=discord&logoColor=white&label=Discord&labelColor=%235865F2
