/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

class A{
    constructor() {
        this.member = 1;
    }
    public member : int;
}

class F {
    public member0 : boolean;
    public member1 : byte;
    public member2 : char;
    public member3 : short;
    public member4 : int;
    public member5 : long;
    public member6 : float;
    public member7 : double;
    public member8 : A;

    constructor() {
        this.member0 = true;
        this.member1 = 2;
        this.member2 = 3;
        this.member3 = 4;
        this.member4 = 5;
        this.member5 = 6;
        this.member6 = 7.0;
        this.member7 = 8.0;
        this.member8 = new A;
    }
}

class F_sub extends F {}

class F_static {
    public static member0 : boolean = true;
    public static member1 : byte = 2;
    public static member2 : char = 3;
    public static member3 : short = 4;
    public static member4 : int = 5;
    public static member5 : long = 6;
    public static member6 : float = 7;
    public static member7 : double = 8;
    public static member8 : A = new A;
}

class F_static_sub extends F_static {}
