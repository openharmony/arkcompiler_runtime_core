# AGENTS.md

This file provides guidance to AI when working with code in this repository.

## Project Metadata

- **name**: Test Suite
- **purpose**: Comprehensive testing infrastructure including unit tests, benchmarks, regression tests, and verification tests for the Ark runtime
- **primary language**: C++, Panda Assembly, ETS (.ets)

## About Tests

The **tests/** directory contains all test suites for the Ark runtime:
- **benchmarks/** - Performance benchmarks
- **checked/** - Compiler/GC optimization verification tests
- **cts-assembly/** - Compatibility test suite assembly tests
- **regression/** - Regression tests
- **integration/** - Integration tests

## Directory Structure

Main File Directories

```
tests/
├── CMakeLists.txt                    # Test build configuration
├── benchmarks/                        # Performance benchmarks
├── checked/                           # Compiler verification tests
├── cts-assembly/                     # Compatibility test suite
├── regression/                        # Regression tests
└── ...
```

## Build Commands

See @../AGENTS.md

## Running Tests

### Run All Tests

See @../AGENTS.md

## Code Style

See @../AGENTS.md

## Dependencies

- **Python 3**: For test scripts
- **es2panda**: Compile .ets to .abc
- **ark**: Run .abc files
