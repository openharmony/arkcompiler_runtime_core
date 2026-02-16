# Skills Folder

Structured learning and reference materials for ETS stdlib development.

## 📁 Skill Structure

Each skill follows the skill-creator format:

```
SKILLS/
└── skill-name/
    ├── SKILL.md       # Main skill file (required)
    │                  # - YAML frontmatter with name and description
    │                  # - Lean instruction body (<5k words)
    │
    ├── scripts/       # Executable automation scripts (optional)
    │   └── validate.sh
    │
    └── assets/        # Output templates and resources (optional)
        └── template.txt
```

## 🎯 Available Skills

### Development Workflow Skills

1. **[build-stdlib](./build-stdlib/)**
   Build the ArkTS stdlib project using CMake and Ninja - handles configuration, compilation, build directory management, and clean rebuilds

2. **[arktest-generator](./arktest-generator/)**
   Generate ArkTest unit tests for stdlib APIs - handles test suite creation, assertions, async tests, and error handling tests

3. **[urunner-stdlib](./urunner-stdlib/)**
   Run ets_func_tests using the Universal Runner - handles workflow selection, test suite configuration, and running specific or full test suites

### Domain Knowledge Skills

4. **[native-integration](./native-integration/)**
   C++ programming, FFI, ANI (Ark Native Interface), ICU4C integration for internationalization, and native method implementation

5. **[internationalization](./internationalization/)**
   Unicode (UTF-8, UTF-16), ICU4C library, locale handling, collation, number/date formatting, and i18n APIs

### Utility Skills

6. **[copyright](./copyright/)**
   Update copyright years in changed source files according to Huawei code style - handles new files (single year) and modified files (year range)

---

## 📚 How to Use This Folder

### For New Contributors

1. **Learn the development workflow:**
   - `build-stdlib` - How to compile the stdlib
   - `arktest-generator` - How to write tests
   - `urunner-stdlib` - How to run tests

3. **Understand domain knowledge:**
   - `native-integration` - For native/C++ method implementation
   - `internationalization` - For i18n features and Unicode handling

### Typical Development Workflow

```
1. Build the project (build-stdlib)
   ↓
2. Generate/write tests (arktest-generator)
   ↓
3. Run tests (urunner-stdlib)
   ↓
4. Update copyright (copyright)
```

### For Daily Development

- **Quick lookup:** reference/ files in each skill
- **Build/run:** build-stdlib and urunner-stdlib skills
- **Testing:** arktest-generator skill
- **Best practices:** SKILL.md files in domain skills

---

## 📖 Skill Design Principles

All skills follow the skill-creator methodology:

### Concise is Key
- SKILL.md files are lean (<5k words)
- Detailed content split into reference/ files
- Progressive disclosure - load only what's needed

### Three-Level Loading System

1. **Metadata (YAML frontmatter)** - Always in context (~100 words)
2. **SKILL.md body** - When skill triggers (<5k words)
3. **Bundled resources** - As needed by AI agent

### File Organization

- **SKILL.md** - Core workflow and quick reference
- **references/** - Detailed documentation loaded as needed
- **scripts/** - Executable automation scripts
- **assets/** - Templates and output resources

### What Skills Provide

1. Specialized workflows - Multi-step procedures
2. Tool integrations - File format or API instructions
3. Domain expertise - Technical knowledge
4. Bundled resources - Scripts, references, assets

---

## 📊 Skill Status

| Skill | Status | Files | Category |
|-------|--------|-------|----------|
| build-stdlib | ✅ Complete | 1 | Workflow |
| arktest-generator | ✅ Complete | 1 | Workflow |
| urunner-stdlib | ✅ Complete | 1 | Workflow |
| native-integration | ✅ Complete | 1 + refs + scripts | Domain |
| internationalization | ✅ Complete | 1 + refs | Domain |
| copyright | ✅ Complete | 1 | Utility |

**Legend:**
- ✅ Complete - All files created following skill-creator format

---

## 🔄 Updating Skills

When updating skill documentation:

1. **Keep SKILL.md lean** - Under 500 lines
2. **Move details to references/** - Progressive disclosure
3. **Update frontmatter** - Keep description comprehensive
4. **Test scripts** - Ensure executability
5. **Version control** - Commit related files together

---

## 📝 Skill Creation Process

Skills are created using the skill-creator methodology:

1. Understand the skill with concrete examples
2. Plan reusable contents (scripts, references, assets)
3. Initialize the skill
4. Edit the skill (implement resources, write SKILL.md)
5. Package the skill
6. Iterate based on real usage

---

## 🔗 Navigation

### Back to Knowledge Base
- [Main AGENTS.md](../AGENTS.md)

---

**Last Updated:** 2026-02-13
