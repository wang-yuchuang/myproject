## ADDED Requirements

### Requirement: Build ext4 filesystem with 4096-byte blocks

The system SHALL create an ext4 filesystem image from the compiled app directory (`out/`) using `mke2fs` with 4096-byte block size, compatible with dm-verity requirements.

#### Scenario: ext4 created with correct block size
- **WHEN** build.sh runs with `--partition-size 256M` and has compiled output in `out/`
- **THEN** an ext4 image is created at `output/app.ext4` with block size 4096 and size appropriate for the partition minus overhead

#### Scenario: ext4 created with minimum size
- **WHEN** app content is small (e.g., 10MB)
- **THEN** ext4 image size is at least 64MB as minimum floor

### Requirement: Generate dm-verity hashtree

The system SHALL generate a dm-verity hash tree for the ext4 image using SHA-256, with configurable block size (default 4096).

#### Scenario: hashtree generated with correct root hash
- **WHEN** ext4 image is provided
- **THEN** a hashtree binary file is generated at `output/app_hashtree.bin` and a `verity_info.txt` with root hash and salt is written

#### Scenario: hashtree matches veritysetup verification
- **WHEN** the hashtree is verified with `veritysetup verify`
- **THEN** verification passes with the same root hash and salt

### Requirement: Generate D9APPMD1 metadata binary

The system SHALL construct a metadata binary following the D9APPMD1 protocol, containing root hash, salt, app version, rollback index, key ID, and signature algorithm info.

#### Scenario: metadata contains all required fields
- **WHEN** metadata is generated for version "1.0.0" with metadata version 1
- **THEN** the metadata binary includes magic `D9APPMD1`, version, total_len, image_size, hash_tree_offset/size, root_hash, salt, app_version, rollback_index, key_id, signature_alg, signature_len

### Requirement: Sign metadata via remote signing server

The system SHALL support online signing by POSTing the metadata binary to a remote signing server API and embedding the returned signature.

#### Scenario: metadata signed successfully
- **WHEN** `--sign-url https://server:8900/api/v1/sign` and `--sign-token token-xxx` are provided
- **THEN** the metadata binary is POSTed to the signing server and the returned signature is saved to `output/metadata.sig`

#### Scenario: skip signing in dev mode
- **WHEN** no `--sign-url` is provided or `SKIP_SIGN=1` is set
- **THEN** the metadata is generated without a signature (for development/debugging only)

### Requirement: Assemble final app.img

The system SHALL assemble the final app partition image as `[ext4 + hashtree + padding + signature + metadata]`, with metadata at the partition end minus 4096 bytes.

#### Scenario: app.img assembled at correct layout
- **WHEN** all components are ready
- **THEN** the output `app.img` has the exact size of `--partition-size`, with metadata at offset `partition_size - 4096`, signature immediately before metadata, and ext4+hashtree at the beginning

#### Scenario: app.img mountable with dm-verity
- **WHEN** `app.img` is written to a partition and `S91app_mount` processes it
- **THEN** the metadata signature is verified against trust anchor, and the partition is mounted with dm-verity at `/app`
