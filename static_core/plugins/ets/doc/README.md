# ETS Documentation Layout

This directory contains the ETS specification and guide sources.

## Directory roles

- `spec/`: current development specification content.
- `concurrency/`: shared concurrency chapters used by `spec/`.
- `runtime/`: shared runtime chapters used by `spec/`.
- `spec_700/`: current 7.0 delivery specification content.
- `spec_700_concurrency/`: concurrency chapters used by `spec_700/`.
- `spec_700_runtime/`: runtime chapters used by `spec_700/`.
- `zh/spec_700/`: Chinese content directory for the 7.0 delivery specification.
- `zh/spec_700_concurrency/`: Chinese concurrency chapters for `zh/spec_700/`.
- `zh/spec_700_runtime/`: Chinese runtime chapters for `zh/spec_700/`.
- `cookbook/`, `stdlib/`, `tutorial/`, `system_arkts/`: guide-style documents built independently.

## Branch plan

- `spec`: development branch for the current target specification.
- `spec_700`: delivery branch for the current 7.0 release.

Keep `spec/` as the development directory and `spec_700/` as the 7.0 delivery directory. Use `zh/spec_700/` to store Chinese 7.0 content, not as a filesystem alias.

## Naming rule

- Use `spec` as the development naming for the current target.
- Use `spec_700` as the delivery naming for the current 7.0 release.
- Use `zh/spec_700` for real Chinese content.
- Keep Chinese content physically separated from `spec_700` so translation and delivery updates can be managed independently.
- Mirror English file names in the Chinese directories one by one.
- Track the English baseline in `zh/translation_manifest.json`.

## Build entry

Use the existing build targets from this directory:

- `./build-docs.sh spec`
- `./build-docs.sh spec_700`

When `./build-docs.sh spec_700` runs, it builds both:

- the English 7.0 delivery specification from `spec_700/`
- the Chinese 7.0 delivery specification from `zh/spec_700/`

Before the Chinese build starts, the layout validator checks that:

- every English `.rst` file under `spec_700/`, `spec_700_concurrency/`, and `spec_700_runtime/`
  has a Chinese counterpart under `zh/`
- every mirrored file is listed in `zh/translation_manifest.json`

## Translation drift check

Use the following command to detect Chinese `.rst` files whose English source
changed after the recorded `source_commit`:

- `python3 ./check_translation.py`
- `python3 ./check_translation.py --update-manifest-status`

When `--update-manifest-status` is used, the script writes the comparison result
back to `zh/translation_manifest.json` by switching `translation_status`
between `translated` and `outdated`. It does not rewrite `source_commit`.

Exit codes:

- `0`: all checked Chinese files are up to date
- `1`: one or more Chinese files are outdated
- `2`: script or metadata error
