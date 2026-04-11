# Chinese Documentation Layout

This directory stores Chinese ETS specification content.

- `spec_700/`: Chinese 7.0 delivery specification content.
- `spec_700_concurrency/`: Chinese concurrency chapters used by `spec_700/`.
- `spec_700_runtime/`: Chinese runtime chapters used by `spec_700/`.
- `translation_manifest.json`: baseline and tracking manifest for English-to-Chinese mapping.

Do not use this directory as a link or alias to English content. Store Chinese source files here directly.

Translation rules:

- Keep the Chinese file layout aligned with the English 7.0 directories.
- Record the English source path and baseline commit in each Chinese `.rst` file.
- Update `translation_manifest.json` when files are added or when the baseline changes.

Use `python3 ../check_translation.py` from this directory, or
`python3 ./check_translation.py` from `static_core/plugins/ets/doc/`,
to detect translated `.rst` files whose English source changed after the
recorded baseline commit.

Add `--update-manifest-status` to write the comparison result back to
`translation_manifest.json` by updating `translation_status`.
