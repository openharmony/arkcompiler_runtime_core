/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
desc: Exported class, interface and a class which implements an interface
tags:
- compile-only
- not-a-test
---*/

export abstract class AC {
    i: number = 0;
    j: string = "";
}

export class C {
    i: number = 0;
    j: string = "";
}

export interface I {
    i: number;
    j: string;
}

export class CI implements I {
    set i(i: number) { this.i = i }
    get i(): number { return this.i }
    set j(j: string) { this.j = j }
    get j(): string { return this.j }
}

export class CA extends AC { }

// Same with optional params
export abstract class ACOPT {
    i?: number = 0;
    j?: string = "";
}

export class COPT {
    i?: number = 0;
    j?: string = "";
}

export interface IOPT {
    i?: number;
    j?: string;
}

//export class CIOPT implements IOPT {
//    set i(i: number) { this.i = i }
//    get i(): number { return this.i }
//    set j(j: string) { this.j = j }
//    get j(): string { return this.j }
//}

export class CAOPT extends ACOPT { }
