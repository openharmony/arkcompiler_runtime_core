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
desc: Ambient Constant declarations implementations
tags: [compile-only, not-a-test]
---*/

export const x: int = 1;

export const obj: Object = '';

export const a1: int = 1;
export const b1: float = 1.0f;
export const c1: string = '';
export const d1: Byte[] = [0b1];
export const e1: string[] = [''];

export const a2: short = 1;
export const b2: double = 1.0;
export const c2: Object[] = [1.0];

export enum Color1 { Red, Green, Blue }

export enum Color2 { Red = 1 , Green = 2, Blue =  3 }

export enum Color3 { Red = "1" , Green = "2", Blue =  "3" }

export const enum Color4 { Red, Green, Blue }

export const QuestionOfAll1: int = 42

export const QuestionOfAll2: int = 0xBAD3

export const BigLongLiteral: long = 9223372036854775807

export const FloatConst1: float = 3.14f

export const FloatConst2: number = 3.141_592

export const FloatConst3: number = .5

export const Hello: string = "Hello"

export const MultilineSentence: string = `This is an example of multi-line string,
      which should be enclosed in backticks (\`)`
