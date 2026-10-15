## ADDED Requirements

### Requirement: Unified build.sh entry point

The system SHALL provide a `build.sh` entry point that orchestrates both the compilation phase (existing Makefile-based flow) and the packaging phase (D9APPMD1 image building).

#### Scenario: full build with defaults
- **WHEN** `./build.sh` is run without arguments
- **THEN** the source is compiled with `release mu3` config, and `app.img` is generated at `pkg/app.img` with default partition size (auto-calculated)

#### Scenario: build with signing
- **WHEN** `./build.sh --sign-url http://server:8900/api/v1/sign --sign-token token-xxx --version 2.0.0` is run
- **THEN** the source is compiled, and the output `app.img` is signed via the remote signing server with the specified version

### Requirement: Docker build support

The build.sh SHALL support executing the entire build inside a Docker container, using the same Docker image as the D9 SDK.

#### Scenario: build in Docker
- **WHEN** `./build.sh --docker` is run
- **THEN** the build is executed inside the `d9-sdk-build:2.11` Docker container with the project root mounted

### Requirement: Preserve existing compilation flow

The build.sh SHALL invoke the existing `mkenv_arm_st` environment setup, `mk_define` build type selection, and `make install` for source compilation, without modifying the existing Makefile structure.

#### Scenario: compilation uses existing toolchain
- **WHEN** `./build.sh` runs
- **THEN** it sources `src/mkenv_arm_st`, runs `src/mk_define release mu3`, executes `make install`, and outputs compiled files to `out/`

### Requirement: Configurable partition size

The build.sh SHALL support an explicit `--partition-size` parameter, defaulting to auto-calculation (3x data size, minimum 64MB).

#### Scenario: partition size auto-calculated
- **WHEN** `--partition-size` is not specified
- **THEN** the partition size is calculated as `max(du(out) * 300%, 64M)` rounded up to MB
