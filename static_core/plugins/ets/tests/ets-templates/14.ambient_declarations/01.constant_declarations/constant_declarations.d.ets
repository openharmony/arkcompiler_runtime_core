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
desc: Ambient Constant declarations
tags: [compile-only, not-a-test]
---*/

export declare const x: int;
  
export declare const obj: Object;
  
export declare const a1: int, b1: float, c1: string, d1: Byte[], e1: string[];
  
export declare const a2: short, b2: double, c2: Object[];
  
export declare enum Color1 { Red, Green, Blue }
  
export declare enum Color2 { Red = 1 , Green = 2, Blue =  3 }
  
export declare enum Color3 { Red = "1" , Green = "2", Blue =  "3" }
    
export declare const enum Color4 { Red, Green, Blue }
  
export declare const QuestionOfAll1: int

export declare const QuestionOfAll2: int
  
export declare const BigLongLiteral: long
  
export declare const FloatConst1: float
  
export declare const FloatConst2: number
  
export declare const FloatConst3: number   
  
export declare const Hello: string
  
export declare const MultilineSentence: string
