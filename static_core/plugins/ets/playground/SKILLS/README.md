# ArkTS Playground Skills

This directory contains custom skills for the ArkTS Playground project.

## What Are Skills?

Skills are reusable, project-specific capabilities that AI agents can invoke via slash commands (e.g., `/fullstack`, `/add-endpoint`). Each skill is documented with its usage, examples, and behavior.

## Structure

```
SKILLS/
├── README.md                    # This file
│
├── add-endpoint/
│   └── add-endpoint.md          # Add FastAPI endpoints
│
├── copyright/
│   └── copyright.md             # Update copyright years
│
├── docker/
│   └── docker.md                # Manage Docker containers
│
├── explore/
│   └── explore.md               # Explore codebase
│
├── fullstack/
│   └── fullstack.md             # Start development servers
│
├── plan-feature/
│   └── plan-feature.md          # Plan new features
│
├── shellcheck/
│   └── shellcheck.md            # Validate bash scripts
│
└── playground_test/
    └── playground_test.md       # Run tests
```

**One Folder Per Skill**: Each skill has its own folder with a matching `.md` filename. This structure:
- Keeps related files together (skill docs, examples, tests)
- Makes it easy to identify skill files
- Allows for future expansion (images, configs, etc.)
- Follows the convention: `SKILLS/<skill-name>/<skill-name>.md`

## Available Skills

### Development Skills

| Skill | Folder | File | Description | Usage |
|-------|--------|------|-------------|-------|
| **fullstack** | `fullstack/` | `fullstack.md` | Start both backend and frontend development servers | `/fullstack` |
| **explore** | `explore/` | `explore.md` | Explore codebase architecture, components, and data flow | `/explore [query]` |
| **plan-feature** | `plan-feature/` | `plan-feature.md` | Plan implementation of new features with architecture exploration | `/plan-feature [feature]` |

### Backend Skills

| Skill | Folder | File | Description | Usage |
|-------|--------|------|-------------|-------|
| **add-endpoint** | `add-endpoint/` | `add-endpoint.md` | Add new FastAPI endpoint with Pydantic models and tests | `/add-endpoint [endpoint]` |

### Code Quality Skills

| Skill | Folder | File | Description | Usage |
|-------|--------|------|-------------|-------|
| **copyright** | `copyright/` | `copyright.md` | Update copyright years in changed files | `/copyright` |
| **shellcheck** | `shellcheck/` | `shellcheck.md` | Verify bash scripts with ShellCheck static analysis | `/shellcheck [path]` |
| **playground-test** | `playground_test/` | `playground_test.md` | Run frontend (Jest) or backend (pytest) tests | `/playground-test [frontend\|backend]` |

### Deployment Skills

| Skill | Folder | File | Description | Usage |
|-------|--------|------|-------------|-------|
| **docker** | `docker/` | `docker.md` | Manage Docker containers and images | `/docker [command] [service]` |

## Skill Structure

Each skill folder contains a `<skill-name>.md` file with this format:

```markdown
---
name: skill-name
description: Brief one-line description
---

[Detailed documentation including Usage, Examples, What It Does, Best Practices, Tips, Related Skills]
```

## Adding New Skills

To add a new skill:

1. **Create a new folder** in `SKILLS/`:
   ```bash
   mkdir SKILLS/my-skill
   ```

2. **Create `<skill-name>.md`** with YAML front matter:
   ```bash
   touch SKILLS/my-skill/my-skill.md
   ```

3. **Add content** following the template:
   ```markdown
   ---
   name: my-skill
   description: What this skill does in one line
   ---

   [Documentation sections]
   ```

4. **Include required sections**:
   - Usage (command syntax)
   - Examples (optional but recommended)
   - What It Does (detailed behavior)
   - Best Practices
   - Tips (optional)
   - Related Skills (cross-references)

5. **Update this README** to list the new skill

## Skill Template

```markdown
---
name: my-skill
description: What this skill does in one line
---

Brief description of the skill's purpose.

## Usage

```bash
/my-skill [arguments]
```

## Examples

### Example 1
```bash
/my-skill example input
```

## What It Does

1. Step one
2. Step two
3. Step three

## Best Practices

- Practice one
- Practice two

## Tips

- Tip one
- Tip two

## Related Skills

- `/other-skill` - Description of related skill
```

## Folder Per Skill Benefits

The "1 folder 1 skill" structure with matching filenames provides:

- **Scalability**: Add related files (examples, tests, configs) without clutter
- **Isolation**: Each skill is self-contained
- **Discoverability**: Easy to see all available skills
- **Maintainability**: Update skills without affecting others
- **Consistency**: Filename matches folder name for easy identification
- **Future-Proof**: Ready for skill enhancements (images, templates, etc.)

## Naming Convention

**Rule**: The skill filename must match the folder name.

```
SKILLS/
├── my-skill/
│   └── my-skill.md        ✅ Correct - filename matches folder
│
└── another-skill/
    └── different.md       ❌ Incorrect - filename doesn't match folder
```

This convention makes it easy to:
- Identify skill files at a glance
- Navigate the skills directory
- Avoid naming conflicts
- Maintain consistency across the project

## Related Documentation

- **AGENTS.md** - Project overview and architecture

## Skill Integration

AI agents automatically discover skills from the `SKILLS/` directory. The folder structure follows the "folder-per-skill" pattern with "match-folder" naming.

Each skill folder's `<skill-name>.md` file is read and parsed for:
- YAML front matter (`name`, `description`)
- Command syntax (from Usage section)
- Documentation content
- Related skills references
