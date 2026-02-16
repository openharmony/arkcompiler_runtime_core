---
name: copyright
description: Update copyright years in changed source files according to Huawei code style
---

Updates copyright year headers in **changed files only** to comply with Huawei Device Co., Ltd. code style requirements.

## Usage

```bash
/copyright
```

Automatically detects changed files via git and updates copyright years appropriately.

## What It Does

Analyzes git-changed files and updates copyright years based on file status:

**New Files** (added in git): Single year (current year)
```
Copyright (c) 2026 Huawei Device Co., Ltd.
```

**Modified Files** (existing files): Extend year range to include current year
```
Before: Copyright (c) 2024-2025 Huawei Device Co., Ltd.
After:  Copyright (c) 2024-2026 Huawei Device Co., Ltd.
```

### Supported File Types

| File Type | Comment Prefix | Example |
|-----------|---------------|---------|
| Shell scripts (`*.sh`) | `#` | `# Copyright (c) 2026 Huawei Device Co., Ltd.` |
| Python (`*.py`) | `#` | `# Copyright (c) 2026 Huawei Device Co., Ltd.` |
| TypeScript/JavaScript (`*.ts`, `*.tsx`, `*.js`, `*.jsx`) | ` *` | ` * Copyright (c) 2024-2026 Huawei Device Co., Ltd.` |
| ArkTS (`*.ets`) | ` *` | ` * Copyright (c) 2024-2026 Huawei Device Co., Ltd.` |
| SCSS (`*.scss`, `*.sass`) | ` *` | ` * Copyright (c) 2026 Huawei Device Co., Ltd.` |
| Dockerfile | `#` | `# Copyright (c) 2024-2026 Huawei Device Co., Ltd.` |
| YAML/TOML | `#` | `# Copyright (c) 2026 Huawei Device Co., Ltd.` |
| Markdown | `#` | `# Copyright (c) 2026 Huawei Device Co., Ltd.` |

## How It Works

1. **Detect Changed Files**: Uses `git diff --name-only` to find modified/new files
2. **Determine File Status**:
   - New files (added): Set copyright to current year only
   - Existing files (modified): Extend year range to include current year
3. **Detect Comment Style**: Determines the correct comment prefix based on file extension
4. **Update**: Replaces or adds copyright header while preserving format
5. **Verify**: Shows summary of changed files with before/after comparison

## Huawei Copyright Header Format

The full copyright header for Huawei Device Co., Ltd.:

```
Copyright (c) [YEAR] Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

**Note**: This skill only manages the year line. The full license header should be included in all source files.

## Common Workflows

### Pre-Commit Copyright Check
Update copyright on staged files before committing:
```bash
# Stage your changes first
git add .

# Run copyright on changed files
/copyright

# Review and commit
git diff  # Review copyright changes
git commit
```

### Working on Feature Branch
Keep copyright current as you develop:
```bash
# After making changes
/copyright

# See what was updated
git status
```

### Update Last N Commits
Apply copyright updates to recent commits:
```bash
# Show files changed in lastest commit
git diff --name-only HEAD~1 HEAD

# Apply copyright to those files
/copyright HEAD~1
```

## Examples

### New File Created Today
```bash
# Created: frontend/src/components/NewFeature.tsx
# Before: No copyright header
# After:
/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
```

### Existing File Modified
```bash
# Modified: backend/src/api/endpoints.py
# Before: # Copyright (c) 2024-2025 Huawei Device Co., Ltd.
# After:  # Copyright (c) 2024-2026 Huawei Device Co., Ltd.
```

### File with Single Year Updated
```bash
# Modified: frontend/utils/helpers.ts
# Before: * Copyright (c) 2024 Huawei Device Co., Ltd.
# After:  * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
```

## Best Practices

- **Pre-Commit Hook**: Run `/copyright` before every commit to keep years current
- **New Files**: Always add copyright with single current year when creating files
- **Modified Files**: Preserve the original year and extend range to current year
- **Review Changes**: Always check `git diff` after copyright updates
- **Branch Safety**: Only updates changed files, safe to run on any branch

## Year Logic

| Scenario | Current Copyright | Updated Copyright |
|----------|-------------------|-------------------|
| New file (no copyright) | - | `2026` |
| Existing file | `2024` | `2024-2026` |
| Existing file | `2024-2025` | `2024-2026` |
| Already current | `2024-2026` | `2024-2026` (no change) |

## Tips

- The skill uses `git diff` to detect changed files automatically
- Comment style is preserved based on file extension
- Single year for truly new files, year range for modifications
- If a file was created in the current year, it stays as single year
- Run before committing to ensure copyright is always current
- Safe to run multiple times - only updates when necessary

## Related Skills

- `/add-endpoint` - Add new endpoints (files created need copyright)
- `/docker` - Docker files may need copyright updates
- `/shellcheck` - Validate shell scripts after copyright updates

## Troubleshooting

**No files updated:**
- No changed files detected in git (check `git status`)
- All changed files already have current copyright years
- No supported file types were changed

**Some files not updated:**
- File may not have git changes (only updates tracked changes)
- File doesn't contain Huawei copyright format yet
- File extension not supported (see supported file types above)

**Incorrect comment style:**
- The skill auto-detects based on file extension
- Manually fix if auto-detection fails for custom file types

**New file got year range instead of single year:**
- File likely has existing copyright from a template
- Remove old copyright before adding new one if it's truly new

**File status confusion:**
- Git staging area matters: staged files are detected
- Use `git diff --name-only` to see what files will be processed
- Commit your changes first if you want to test the skill

## Implementation Notes

This skill would need to:
1. Run `git diff --name-only` to detect changed files
2. For each file, check if it's new (added) or modified
3. Parse existing copyright year if present
4. Apply year logic:
   - New file without copyright → Add single current year
   - New file with old copyright → Update to range including current year
   - Modified file → Extend year range to include current year
5. Preserve comment style based on file extension
