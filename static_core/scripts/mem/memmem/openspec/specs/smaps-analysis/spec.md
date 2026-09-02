# smaps-analysis Specification

## Purpose
Defines parsing and reporting semantics for raw `/proc/<pid>/smaps` memory snapshots.

## Requirements

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
- **THEN** the profile exposes `size_kb`, `rss_kb`, `pss_kb`, `referenced_kb`, `shared_kb`, `private_kb`, `swap_kb`, `swap_pss_kb`, and `anonymous_kb` integer fields

### Requirement: Smaps parser filters mappings by tag pattern
The system SHALL accept an optional compiled regular expression as a tag filter for `parse_smaps_text`. When no filter is provided, the parser SHALL aggregate every mapping as today. When a filter is provided, the parser SHALL include a mapping's values in both the per-tag breakdown and the total profile only if `pattern.match(normalized_tag)` returns a match, where the pattern is a compiled `re.Pattern`, the tag is the normalized mapping tag (whitespace-stripped; `[anonymous]` for mappings without an explicit path), and matching uses `re.match` semantics anchored at the start of the tag. The parser SHALL return `None` when no mapping was aggregated (the text contains no mappings, or the filter excludes every mapping); mappings that are aggregated with zero values SHALL NOT be conflated with no aggregated mappings and SHALL yield a summary. The parser SHALL remain free of logging and global state.

#### Scenario: Parser aggregates all tags without a filter
- **WHEN** a caller invokes `parse_smaps_text` without a tag filter
- **THEN** the parser aggregates every mapping into the breakdown and total profile exactly as before

#### Scenario: Filter restricts breakdown and totals to matching tags
- **WHEN** a caller passes a compiled pattern to `parse_smaps_text` and some mapping tags match via `re.match`
- **THEN** the breakdown contains only matching tags and the total profile contains the sum of the matching tags only

#### Scenario: Filter matches no tags
- **WHEN** a caller passes a compiled pattern to `parse_smaps_text` and no mapping tag matches via `re.match`
- **THEN** the parser returns `None`

#### Scenario: Parser returns None for text without mappings
- **WHEN** a caller invokes `parse_smaps_text` with text containing no mapping headers
- **THEN** the parser returns `None` rather than a zero-valued summary

#### Scenario: Filter matching is anchored at the start of the tag
- **WHEN** a caller passes the compiled pattern `.*\.so` against a mapping whose tag is `/system/lib64/lib.so`
- **THEN** the mapping is included, and a pattern such as `\.so$` that is not anchored at the start matches no tags

#### Scenario: Filter applies to normalized tags
- **WHEN** a caller passes a compiled pattern matching the normalized anonymous tag `[anonymous]`
- **THEN** mappings without an explicit path are included when their normalized tag matches and excluded otherwise
