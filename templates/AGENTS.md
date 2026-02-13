# Templates Module AGENTS.md

> Verified on commit: f965a250e9a4ad664a282f7e220a42a744964a5a
> Last updated: 2026-02-13
> Assumed workspace: arkcompiler/runtime_core/templates
> Scope: templates/

## Purpose

Provides shared Ruby scripts, ERB templates, and default YAML data sources for repo-wide **code generation** and **YAML merging**. This directory **does not define any GN build targets**; it is only referenced by path from other modules' `ark_gen_file`, `concat_yamls`, etc.

## Where to Edit

### Module Entry Points

- `templates/common.rb` - Shared Ruby utilities
- `templates/concat_yamls.sh` - YAML merge script
- `templates/merge.rb` - YAML merge Ruby implementation
- `templates/plugin_options.rb` - Plugin options parsing
- `templates/plugin_options.yaml` - Default plugin options
- `templates/options/options.h.erb` - Command-line/option table header template
- `templates/events/events.h.erb` - Event system header template
- `templates/logger_components/logger_components.inc.erb` - Logger component generation

### Module-Specific "Do Not Edit" Boundaries

⚠️ **Critical**: This directory is **shared data source and script library for repo-wide code generation**.
- ⚠️ Do not break interface or format of existing generated output (e.g. `base_options.h`, `plugin_options.yaml` structure)
- ⚠️ Do not add BUILD.gn or define targets in this directory; it is data/script-only

## Validation Checklist

### Module Dependencies

**Key upstream dependencies**:
- Ruby - Scripts require Ruby at build time
- `$ark_root/isa/gen.rb` - Some targets are driven by ISA generation driver

**Key downstream dependents**:
- **Root BUILD.gn** - `concat_plugins_yamls` uses `default_file = "$ark_root/templates/plugin_options.yaml"`
- **ark_config.gni** - `concat_yamls` uses `script = "$ark_root/templates/concat_yamls.sh"`
- **libpandabase** - `base_options_h` uses templates/options/options.h.erb
- **libpandafile** - `source_lang_enum_h` uses templates/plugin_options.rb

## References

- Verified: No BUILD.gn - only referenced as scripts and data
- Verified: Shared across modules - libpandabase, libpandafile, assembler, etc. use templates
- Verified: concat_yamls uses templates/plugin_options.yaml as default file