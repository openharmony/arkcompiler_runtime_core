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
└── ets/              # ETS (ArkTS-Sta) language plugin
```

## ETS Plugin

The main language plugin is **ETS** for ArkTS-Sta:

See @./ets/AGENTS.md for complete ETS plugin documentation.

## Build Commands

See @../AGENTS.md

## Code Style

See @../AGENTS.md

## Testing

See @../AGENTS.md
