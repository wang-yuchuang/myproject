## ADDED Requirements

### Requirement: Build RAUC-compatible OTA bundle

The system SHALL package the `app.img` into a RAUC bundle file (`.raucb`) for OTA deployment, supporting A/B slot updates.

#### Scenario: RAUC bundle created with correct compatible string
- **WHEN** `scripts/ota-bundle.sh --app-image pkg/app.img --version 1.0.0 --compatible "d9360-bmu05"` is run
- **THEN** a RAUC bundle `.raucb` file is generated with the specified compatible string and version

#### Scenario: RAUC bundle signable with local key
- **WHEN** `--rauc-cert cert.pem --rauc-key key.pem` is provided
- **THEN** the bundle is signed with the provided RAUC certificate and key

### Requirement: Support remote signature replacement for RAUC bundle

The system SHALL support two-phase signing: generate bundle with temporary key, then replace CMS signature using remote signing server.

#### Scenario: remote signing for RAUC bundle
- **WHEN** `--sign-url URL --sign-token TOKEN --sign-key-id ID` is provided
- **THEN** the bundle is first signed with a temporary key, then the CMS signature is replaced using `rauc replace-signature` with the remote server

### Requirement: Generate app-post-install compatible bundle

The OTA bundle SHALL trigger the `app-post-install` hook on the device after successful installation, which updates the DATA_SLOT U-Boot environment variable.

#### Scenario: post-install hook triggered
- **WHEN** the RAUC bundle is installed on the device and marked successful
- **THEN** `app-post-install` script runs and sets `DATA_SLOT` to the newly installed slot
