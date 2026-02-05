# ArkGuard — Agent Guide

This document is the **full working guide** for **`arkcompiler/runtime_core/ark_guard`**.

---

## What is ArkGuard?

**ArkGuard** is a **static bytecode obfuscation** tool for Panda/Ark `.abc` files. It reads an input ABC, applies name obfuscation (classes, methods, fields, file names, paths), optional remove-log and name-cache apply/print, then writes the obfuscated ABC. It is used in the Ark/ArkTS toolchain for release builds to reduce symbol exposure and optionally strip log calls.

Main capabilities:

- **Single entry**: **`ArkGuard::Run(argc, argv)`** — parse args → parse config → init **FileView** (ABC) → **Obfuscator::Execute(fileView)** → **fileView.Save(obfPath)**. Public API is in **include/ark_guard/ark_guard.h** (namespace **ark::guard**).
- **Configuration**: JSON config supplies **abcPath**, **obfAbcPath**, **defaultNameCachePath**, and **obfuscationRules** (disable obfuscation, print/apply name cache, remove-log, file name obfuscation, keep path/class options, printseeds). Parsed by **ConfigurationParser** into **Configuration**.
- **FileView (abckit_wrapper)**: Wraps **abckit::File**; provides **Init(path)**, **GetModule** / **GetObject** / **Get\<T\>**, **ModulesAccept(visitor)**, **ClearObjectsData()**, **Save(path)**. Built from **libabckit** via the **abckit_wrapper** layer (Module, Object, Class, Method, Field, annotations, visitors).
- **Obfuscation pipeline**: **MemberLinker** (link) → **NameMarker** (keep file/path/class) → optional **SeedsDumper** → **NameCacheParser** / **NameCacheKeeper** (apply name cache) → **MemberPreProcessor** / **ElementPreProcessor** (prepare name mappings) → optional **RemoveLogObfuscator** → **MemberObfuscator** / **ElementObfuscator** (apply renames) → **NameCacheDumper** (print name cache).

Artifacts: **ark_guard_shared** (shared library), **ark_guard_static** (static library), and **ark_guard** executable (when **abckit_enable**). The executable is driven by a config file path (and optional debug/log flags).

---

## High-level pipeline and data flow

1. **CLI** (**ark_guard/main.cpp**): **ArkGuard::Run(argc, argv)** → **ArgsParser::Parse** (config path, debug, help, debug file) → **InitLogger** → **ConfigurationParser::Parse(configuration)** → **FileView::Init(abcPath)** → **Obfuscator(config).Execute(fileView)** → **fileView.Save(obfAbcPath)**.

2. **Obfuscator::Execute** (see **library/obfuscator/obfuscator.cpp**):
   - **MemberLinker(fileView).Link()** — link member references across the ABC.
   - **NameMarker(config).Execute(fileView)** — mark file names, paths, and class specifications to keep (no obfuscation).
   - If **HasKeepConfiguration()**: **SeedsDumper(printSeedsOption).Dump(fileView)**.
   - **NameCacheParser(applyNameCache).Parse()** → **NameCacheKeeper(fileView).Process(parser.GetNameCache())** — apply previous name cache so re-runs are stable.
   - **MemberPreProcessor** / **ElementPreProcessor** — build **NameMappingManager** (old name → new name) for members and elements; **Process(fileView)**.
   - If **IsRemoveLogObfEnabled()**: **RemoveLogObfuscator().Execute(fileView)**.
   - **MemberObfuscator(memberNameMapping).Execute(fileView)** — rename members in bytecode/metadata.
   - **ElementObfuscator(elementNameMapping).Execute(fileView)** — rename elements (e.g. class/element names).
   - **NameCacheDumper(fileView, printNameCache, defaultNameCachePath).Dump()** — write name cache for next run.

3. **Output**: Obfuscated ABC at **obfAbcPath**; optional name cache and printseeds output per config.

Key data structures:

- **Configuration**: **abcPath**, **obfAbcPath**, **defaultNameCachePath**; **ObfuscationRules** (disableObfuscation, printNameCache, applyNameCache, removeLog, printSeedsOption, fileNameOption, keepOptions with path and **ClassSpecification** list).
- **FileView**: **abckit::File**; **moduleTable_**; **ObjectPool**; visitor-based iteration over modules/objects/classes/members.
- **abckit_wrapper**: **Module**, **Object**, **Class**, **Method**, **Field**, **Annotation**; visitors (**ModuleVisitor**, **ClassVisitor**, **MemberVisitor**, etc.) for traversing the ABC view.

---

## Repository layout (where things live)

### Entry and driver

- **include/ark_guard/ark_guard.h** — **ArkGuard**, **Run(argc, argv)**. Namespace **ark::guard**.
- **library/ark_guard.cpp** — **ArkGuard::Run** implementation: ArgsParser, InitLogger, ConfigurationParser, Configuration, FileView::Init, Obfuscator::Execute, FileView::Save.
- **ark_guard/main.cpp** — **main** → **ArkGuard::Run(argc, argv)**.

### Arguments and configuration

- **library/args_parser.h** / **args_parser.cpp** — **ArgsParser**: **Parse**, **GetConfigFilePath**, **IsDebug**, **IsHelp**, **GetDebugFile**.
- **library/configuration/configuration.h** — **Configuration**, **ObfuscationRules**, **KeepOptions**, **ClassSpecification**, **PrintSeedsOption**, **FileNameObfuscationOption**, **KeepPathOption**; getters for paths, disable/remove-log/file-name/keep options.
- **library/configuration/configuration_parser.h** / **configuration_parser.cpp** — **ConfigurationParser(configPath)**; **Parse(configuration)** from JSON; **keep_option_parser**, **word_reader** for keep rules.
- **library/configuration/class_specification.h** — **ClassSpecification** (class/member keep rules).
- **library/configuration/keep_option_parser.h** / **keep_option_parser.cpp** — parsing keep options.
- **library/configuration/word_reader.h** / **word_reader.cpp** — word/token reading for config.

### Name cache and name mapping

- **library/name_cache/name_cache_parser.h** / **name_cache_parser.cpp** — parse name cache file (apply run).
- **library/name_cache/name_cache_keeper.h** / **name_cache_keeper.cpp** — apply name cache to **FileView**.
- **library/name_cache/name_cache_dumper.h** / **name_cache_dumper.cpp** — dump name cache after obfuscation.
- **library/name_mapping/name_mapping_manager.h** / **name_mapping_manager.cpp** — manage old→new name mappings.
- **library/name_mapping/order_name_generator.h** / **order_name_generator.cpp** — generate obfuscated names (e.g. order-based).

### Obfuscator pipeline

- **library/obfuscator/obfuscator.h** / **obfuscator.cpp** — **Obfuscator(config)**; **Execute(fileView)** orchestrates linker, name marker, seeds dump, name cache apply, preprocessors, remove-log, member/element obfuscators, name cache dump.
- **library/obfuscator/member_linker.h** / **member_linker.cpp** — **MemberLinker**; **Link()**.
- **library/obfuscator/name_marker.h** / **name_marker.cpp** — **NameMarker**; mark kept file names/paths/classes.
- **library/obfuscator/member_preprocessor.h** / **member_preprocessor.cpp** — **MemberPreProcessor**; build member name mapping, **Process(fileView)**.
- **library/obfuscator/element_preprocessor.h** / **element_preprocessor.cpp** — **ElementPreProcessor**; build element name mapping.
- **library/obfuscator/member_obfuscator.h** / **member_obfuscator.cpp** — **MemberObfuscator**; apply member renames to **FileView**.
- **library/obfuscator/element_obfuscator.h** / **element_obfuscator.cpp** — **ElementObfuscator**; apply element renames.
- **library/obfuscator/remove_log_obfuscator.h** / **remove_log_obfuscator.cpp** — **RemoveLogObfuscator**; strip log calls when enabled.
- **library/obfuscator/seeds_dumper.h** / **seeds_dumper.cpp** — **SeedsDumper**; dump seeds (kept classes/members) for debugging.
- **library/obfuscator/obfuscator_data_manager.h** / **obfuscator_data_manager.cpp** — shared obfuscator data.
- **library/obfuscator/member_descriptor_util.h** / **member_descriptor_util.cpp** — member descriptor helpers.

### abckit_wrapper (ABC view layer)

- **abckit_wrapper/include/abckit_wrapper/file_view.h** — **FileView**: **Init**, **GetModule**, **GetObject**, **Get\<T\>**, **ModulesAccept**, **ClearObjectsData**, **Save**.
- **abckit_wrapper/include/abckit_wrapper/module.h**, **object.h**, **class.h**, **method.h**, **field.h**, **annotation.h**, **namespace.h**, **modifiers.h** — ABC entity types and **ObjectPool**.
- **abckit_wrapper/include/abckit_wrapper/visitor.h** and **visitor/** — **ModuleVisitor**, **ClassVisitor**, **MemberVisitor**, **AnnotationTargetVisitor**, etc.
- **abckit_wrapper/library/** — implementations (file_view.cpp, module.cpp, class.cpp, method.cpp, field.cpp, annotation*.cpp, *visitor.cpp, class_hierarchy_builder, hierarchy_dumper, etc.).

### Util and error

- **library/error.h** / **error.cpp** — **ErrorCode**, error handling.
- **library/logger.h** — logging (uses **ark::Logger**).
- **library/util/assert_util.h**, **file_util**, **json_util**, **string_util** — utilities.

### Build and tests

- **BUILD.gn** — **ark_guard_public_config**; **ark_guard_sources**; **ark_guard_shared**, **ark_guard_static**; **ark_guard** executable (when **abckit_enable**). Deps: **abckit_wrapper** (shared/static), **libarktsbase**.
- **tests/** — unit tests (C++ and .ets); **ark_guard_host_unittest** target.

---

## Build and test

### Build

- **Libraries**: **ark_guard_shared** (shared), **ark_guard_static** (static). Sources listed in **ark_guard_sources** (args_parser, ark_guard, configuration/*, name_cache/*, name_mapping/*, obfuscator/*, util/*). Config: **ark_guard_public_config**. Deps: **abckit_wrapper** (shared/static), **libarktsbase**.
- **Executable**: **ark_guard** — built only when **abckit_enable**; links **ark_guard_static**; **install_enable = false**.

From ark root (with abckit enabled):

```bash
./ark.py x64.release ark_guard_packages_linux --gn-args="abckit_enable=true"
```

### Test

- Run host unit tests:

```bash
./ark.py x64.release ark_guard_host_unittest --gn-args="abckit_enable=true"
```

Tests live under **tests/** (C++ and .ets); test data under **tests/** and **abckit_wrapper/tests/**.

---

## How to add a new feature (checklist)

### A) New config option

1. Add the field to **Configuration** or **ObfuscationRules** (or a new option struct) in **configuration/configuration.h**.
2. In **ConfigurationParser**, add parsing in **Parse** or a dedicated **ParseXxx** from the JSON (see **ParsePrintSeedsOption**, **ParseFileNameObfuscation**, **ParseKeepOption**).
3. Use the new option in **Obfuscator::Execute** or in the relevant sub-component (e.g. NameMarker, NameCacheDumper). Add or extend tests.

### B) New obfuscation step or keep rule

1. If a new “keep” or “mark” rule: extend **NameMarker** or **KeepOptions** / **ClassSpecification** and **keep_option_parser** so the config can express it; ensure **NameMarker::Execute** marks the right entities so later obfuscators skip them.
2. If a new obfuscation pass: implement a class (e.g. **XxxObfuscator**) that takes **FileView** (and any **NameMappingManager** or config), implement **Execute(fileView)** by traversing via **FileView::ModulesAccept** and/or **Get**/visitors, apply edits to the ABC view. Call it from **Obfuscator::Execute** in the correct order (e.g. after preprocessors, before or after member/element obfuscators as needed).
3. Add tests under **tests/** or **abckit_wrapper/tests/** with a small ABC and config that cover the new behavior.

### C) abckit_wrapper / FileView extension

1. If the ABC view needs a new entity type or API: extend **abckit_wrapper** (e.g. new type in **include/abckit_wrapper**, implementation in **library/**), and ensure **FileView** exposes it (**Get\<T\>**, or a new visitor) if needed.
2. If libabckit’s **abckit::File** API changes, update **FileView** and the wrapper types so **Init**, **Save**, and visitor-based iteration stay consistent. Keep **ObjectPool** and module/object lifecycle correct.

### D) Name cache format or name mapping

1. To change name cache format: update **NameCacheParser**, **NameCacheKeeper**, and **NameCacheDumper** so parse/apply/dump stay in sync. Preserve backward compatibility or document the break.
2. To add a new name generator: extend **NameMappingManager** and/or add a new generator (e.g. in **name_mapping/**) and wire it in the preprocessors that build the mapping.

---

## Critical pitfalls / "do not do this"

- **Do not** assume **FileView::Init** succeeded without checking the return value; **ArkGuard::Run** uses **ARK_GUARD_ASSERT** on failure.
- **Do not** run obfuscation steps in the wrong order: e.g. name cache apply must happen before preprocessors so applied names are used; member/element obfuscators must run after preprocessors; name cache dump after obfuscation.
- **Do not** modify **FileView** (or underlying abckit state) in a way that breaks **Save** or leaves bytecode/metadata inconsistent; the wrapper and libabckit must stay in sync.
- **Do not** add global mutable state for the obfuscation pipeline; **Configuration** and **FileView** (and **NameMappingManager** per run) carry the needed state.
- When adding keep rules, ensure both **NameMarker** and the obfuscators that read “marked” state respect the same semantics (file name, path, class specification) so kept names are never obfuscated.

---

## Where to look (quick reference)

| Goal | Location |
|------|----------|
| Entry and pipeline | **include/ark_guard/ark_guard.h**, **library/ark_guard.cpp**, **ark_guard/main.cpp** |
| Args and config | **library/args_parser.h**, **library/configuration/configuration.h**, **library/configuration/configuration_parser.cpp** |
| Keep / class spec | **library/configuration/class_specification.h**, **library/configuration/keep_option_parser.cpp** |
| Name cache | **library/name_cache/name_cache_parser.cpp**, **name_cache_keeper.cpp**, **name_cache_dumper.cpp** |
| Name mapping | **library/name_mapping/name_mapping_manager.cpp**, **order_name_generator.cpp** |
| Obfuscator steps | **library/obfuscator/obfuscator.cpp**, **member_linker**, **name_marker**, **member_preprocessor**, **element_preprocessor**, **member_obfuscator**, **element_obfuscator**, **remove_log_obfuscator**, **seeds_dumper** |
| ABC view | **abckit_wrapper/include/abckit_wrapper/file_view.h**, **module.h**, **object.h**, **class.h**, **method.h**, **field.h**, **visitor/** |
| Build and tests | **BUILD.gn**, **tests/** , **abckit_wrapper/tests/** |
