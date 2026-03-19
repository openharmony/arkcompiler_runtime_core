# AGENTS.md

This file provides guidance to AI when working with code in this repository.

## Project Metadata

- **name**: Language Plugins
- **purpose**: Plugin architecture for language implementations. Each language is implemented as a plugin
- **primary language**: C++

## About Plugins

The **plugins/** directory contains language plugin implementations for ArkVM.

## Plugin Architecture

ArkVM uses a plugin architecture where each language is implemented as a plugin:

```
plugins/
├── ets/              # ETS (ArkTS-Sta) language plugin
└── ecmascript/       # Optional JS/interop plugin in checkouts that include it
```

Enabled plugins depend on both checkout contents and build flags such as `-DPANDA_WITH_<LANG>=ON/OFF`.
Do not assume fixed language counts are current facts for a specific checkout.

## ETS Plugin

The main language plugin is **ETS** for ArkTS-Sta:

See @./ets/AGENTS.md for complete ETS plugin documentation.

If the checkout also includes `plugins/ecmascript/`, treat it as optional support used by JS and interop workflows
rather than as guaranteed static-core functionality in every build.

## Build Commands

See @../AGENTS.md

## Code Style

See @../AGENTS.md

## Testing

See @../AGENTS.md
