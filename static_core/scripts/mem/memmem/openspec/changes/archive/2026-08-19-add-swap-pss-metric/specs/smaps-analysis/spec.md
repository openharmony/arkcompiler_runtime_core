## MODIFIED Requirements

### Requirement: Smaps parser produces total memory profile
The system SHALL parse raw `/proc/<pid>/smaps` text and produce total Size, Rss, Pss, Referenced, Shared, Private, Swap, SwapPss, and Anonymous memory values in KiB.

#### Scenario: Smaps file contains multiple mappings
- **WHEN** the parser receives smaps text with multiple memory mappings
- **THEN** the returned total profile contains the sum of Size, Rss, Pss, Referenced, Shared, Private, Swap, SwapPss, and Anonymous values across all mappings

#### Scenario: Smaps mapping omits a supported metric
- **WHEN** a mapping does not contain one of Size, Rss, Pss, Referenced, Shared, Private, Swap, SwapPss, or Anonymous
- **THEN** the parser treats the missing metric as zero for that mapping

#### Scenario: Smaps file contains unsupported metrics
- **WHEN** the parser receives smaps text with fields other than Size, Rss, Pss, Referenced, Shared, Private, Swap, SwapPss, and Anonymous
- **THEN** the parser ignores unsupported fields for the total profile

### Requirement: Smaps parser exposes typed summary objects
The system SHALL expose immutable dataclasses for `MemProfile` and `SmapsSummary` where `SmapsSummary.total` contains aggregate metrics and `SmapsSummary.breakdown` maps tags to aggregate metrics.

#### Scenario: Caller parses smaps text
- **WHEN** a caller invokes `parse_smaps_text` with raw smaps text
- **THEN** the system returns a `SmapsSummary` with a total `MemProfile` and a tag breakdown dictionary

#### Scenario: Caller consumes profile fields
- **WHEN** a caller reads a returned `MemProfile`
- **THEN** the profile exposes `size_kb`, `rss_kb`, `pss_kb`, `referenced_kb`, `shared_kb`, `private_kb`, `swap_kb`, `swap_pss_kb`, and `anonymous_kb` integer fields