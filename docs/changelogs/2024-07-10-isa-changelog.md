# 2024-07-10-isa-changelog

This document describes change log with the following modifications:

* Version
* Bytecode instructions

## Version
We update version from 12.0.5.0 to  12.0.6.0

## Bytecode instructions
To support [lazy import](https://gitee.com/openharmony/docs/blob/3cc4941c35f525895c9710c761d92f1a4d799f11/zh-cn/application-dev/quick-start/arkts-lazy-import.md), the following bytecode instructions are added for loading module variables lazily:
    - callruntime.ldlazymodulevar
    - callruntime.wideldlazymodulevar
    - callruntime.ldlazysendablemodulevar
    - callruntime.wideldlazysendablemodulevar