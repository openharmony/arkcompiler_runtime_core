## ADDED Requirements

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