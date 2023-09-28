# ArkTS Migration Linter

IntelliJ platform code inspection for TypeScript source code which is to be migrated into ArkTS code.

## Build

### Linux

In terminal run from the project's root:
```bash
./gradlew buildPlugin
```

which will generate ready-to-use plugin located at
`build/distributions/${PLUGIN_NAME}-{PLUGIN_VERSION}.zip`.

### Windows

In `cmd` run from the project's root:
```cmd
gradlew.bat buildPlugin
```

which will generate ready-to-use plugin located at
`build\distributions\%PLUGIN_NAME%-%PLUGIN_VERSION%.zip`.
