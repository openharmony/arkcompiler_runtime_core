/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

class Car {
    constructor(manufacturer: String, model: String, ecoFriendly: boolean, highPerformance: boolean) {
        this.manufacturer = manufacturer;
        this.__model = model;
        this.__ecoFriendly = ecoFriendly;
        this.highPerformance = highPerformance;
    }

    set model(model: String) {
        this.__model = model;
    }

    get ecoFriendly(): boolean {
        return this.__ecoFriendly;
    }

    set ecoFriendly(ecoFriendly: boolean) {
        this.__ecoFriendly = ecoFriendly;
    }

    manufacturer: String;
    private __model: String;
    private __ecoFriendly: boolean;
    highPerformance: boolean;
}

function newCarObject() {
    return new Car("Porsche", "Porsche 911", false, false);
}

interface Iface {
    prop: boolean
}

class C1 implements Iface {
    public prop: boolean = false
}

class C2 implements Iface {
    public classprop: boolean = false

    public get prop(): boolean {
        return this.classprop
    }

    public set prop(p: boolean) {
        this.classprop = p
    }
}

function newC1() {
    return new C1();
}

function newC2() {
    return new C2();
}
