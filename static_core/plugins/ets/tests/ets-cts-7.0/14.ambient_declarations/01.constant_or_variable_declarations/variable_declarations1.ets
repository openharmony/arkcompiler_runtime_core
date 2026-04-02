/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: Ambient variable declarations implementations
tags: [compile-only, not-a-test]
---*/

export let x: int = 1;

export let obj: Object = '';

export let a1: int = 1;
export let b1: float = 1.0f;
export let c1: string = '';
export let d1: Byte[] = [0b1];
export let e1: string[] = [''];

export let a2: short = 1;
export let b2: double = 1.0;
export let c2: Object[] = [1.0];

export enum Color1 { Red, Green, Blue }
export let varColor1: Color1 = Color1.Red;

export const enum Color2 { Red = 1 , Green = 2, Blue =  3 }
export let varColor2: Color2 = Color2.Red;

export const enum Color3 { Red = "1" , Green = "2", Blue =  "3" }
export let varColor3: Color3 = Color3.Red;

export enum Color4 { Red, Green, Blue }
export let varColor4: Color4 = Color4.Red;

export let QuestionOfAll1: int = 42

export let QuestionOfAll2: int = 0xBAD3

export let BigLongLiteral: long = 9223372036854775807

export let FloatVar1: float = 3.14f

export let FloatVar2: number = 3.141_592

export let FloatVar3: number = .5

export let Hello: string = "Hello"

export let MultilineSentence: string = `This is an example of multi-line string,
      which should be enclosed in backticks (\`)`
