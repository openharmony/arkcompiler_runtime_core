/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: enum for export
tags: [not-a-test, compile-only]
---*/

export enum ImpColorsInt {
  Red = 1,
  Green,
  Blue = 4
}

export enum ImpColorsStr {
  Red = "One",
  Green = "Two",
  Blue = "Three"
}

export enum ExpColorsLong: long {
  Red = 0x7FFF_FFFF_1,
  Green,
  Blue = 0x7FFF_FFFF_4
}

export enum ExpColorsInt: int {
  Red = 1,
  Green,
  Blue = 4
}

export enum ExpColorsShort: short {
  Red = 1,
  Green,
  Blue = 4
}

export enum ExpColorsByte: byte {
  Red = 1,
  Green,
  Blue = 4
}

export enum ExpColorsDouble: double {
  Red = 1.0,
  Green = 2.5,
  Blue = 5.0
}

export enum ExpColorsFloat: float {
  Red = 1.0f,
  Green = 2.5f,
  Blue = 5.0f
}
