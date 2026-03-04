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
desc: Ambient variable declarations
tags: [compile-only, not-a-test]
---*/

export declare let x: int;
  
export declare let obj: Object;
  
export declare let a1: int, b1: float, c1: string, d1: Byte[], e1: string[];
  
export declare let a2: short, b2: double, c2: Object[];
  
export declare enum Color1 { Red, Green, Blue }
export declare let varColor1: Color1;

export declare const enum Color2 { Red = 1 , Green = 2, Blue =  3 }
export declare let varColor2: Color2;

export declare const enum Color3 { Red = "1" , Green = "2", Blue =  "3" }
export declare let varColor3: Color3;

export declare enum Color4 { Red, Green, Blue }
export declare let varColor4: Color4;

export declare let QuestionOfAll1: int

export declare let QuestionOfAll2: int
  
export declare let BigLongLiteral: long
  
export declare let FloatVar1: float
  
export declare let FloatVar2: number
  
export declare let FloatVar3: number
  
export declare let Hello: string
  
export declare let MultilineSentence: string
