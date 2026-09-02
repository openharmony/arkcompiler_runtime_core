## ADDED Requirements

### Requirement: Smaps parser produces total memory profile
The system SHALL parse raw `/proc/<pid>/smaps` text and produce total Size, Rss, Pss, Referenced, Shared, Private, Swap, and Anonymous memory values in KiB.

#### Scenario: Smaps file contains multiple mappings
- **WHEN** the parser receives smaps text with multiple memory mappings
- **THEN** the returned total profile contains the sum of Size, Rss, Pss, Referenced, Shared, Private, Swap, and Anonymous values across all mappings

#### Scenario: Smaps mapping omits a supported metric
- **WHEN** a mapping does not contain one of Size, Rss, Pss, Referenced, Shared, Private, Swap, or Anonymous
- **THEN** the parser treats the missing metric as zero for that mapping

#### Scenario: Smaps file contains unsupported metrics
- **WHEN** the parser receives smaps text with fields other than Size, Rss, Pss, Referenced, Shared, Private, Swap, and Anonymous
- **THEN** the parser ignores unsupported fields for the total profile

### Requirement: Smaps parser derives shared and private totals
The system SHALL calculate Shared as `Shared_Clean + Shared_Dirty` and Private as `Private_Clean + Private_Dirty` for each mapping.

#### Scenario: Mapping contains clean and dirty shared values
- **WHEN** smaps text contains `Shared_Clean` and `Shared_Dirty` values
- **THEN** the parser adds both values into the Shared metric

#### Scenario: Mapping contains clean and dirty private values
- **WHEN** smaps text contains `Private_Clean` and `Private_Dirty` values
- **THEN** the parser adds both values into the Private metric

### Requirement: Smaps parser produces per-tag breakdown
The system SHALL group parsed smaps mappings by mapping tag and produce a memory profile for each tag.

#### Scenario: Multiple mappings share a tag
- **WHEN** smaps text contains multiple mappings with the same tag
- **THEN** the breakdown entry for that tag contains the sum of supported metrics across those mappings

#### Scenario: Mappings have different tags
- **WHEN** smaps text contains mappings with different tags
- **THEN** the breakdown contains a separate memory profile for each tag

#### Scenario: Mapping has no explicit path tag
- **WHEN** a smaps mapping header has no explicit mapped path
- **THEN** the parser assigns it a stable tag suitable for reporting

### Requirement: Smaps parser exposes typed summary objects
The system SHALL expose immutable dataclasses for `MemProfile` and `SmapsSummary` where `SmapsSummary.total` contains aggregate metrics and `SmapsSummary.breakdown` maps tags to aggregate metrics.

#### Scenario: Caller parses smaps text
- **WHEN** a caller invokes `parse_smaps_text` with raw smaps text
- **THEN** the system returns a `SmapsSummary` with a total `MemProfile` and a tag breakdown dictionary

#### Scenario: Caller consumes profile fields
- **WHEN** a caller reads a returned `MemProfile`
- **THEN** the profile exposes `size_kb`, `rss_kb`, `pss_kb`, `referenced_kb`, `shared_kb`, `private_kb`, `swap_kb`, and `anonymous_kb` integer fields
