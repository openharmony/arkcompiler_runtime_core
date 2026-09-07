/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

/*
 * Patch carrying the WHOLE module (the "full recompile" shape): every class of the
 * original file is present with a changed method body. After coldReload every
 * lookup finds its class here, so every report line must show the new behaviour --
 * including `lazy`, which resolves `Lazy` through two levels of indirection.
 */

class Greeter {
    public n: int = 0;

    public bump(): int {
        this.n = this.n + 1;
        return this.n;
    }

    public tag(): string {
        return 'V2';
    }
}

class Sub extends Greeter {
}

class Calc {
    public static base: int = 100;

    public static calc(): int {
        return 42;
    }
}

class LazyHolder {
    public get(): string {
        return new Lazy().tag();
    }
}

class Lazy {
    public tag(): string {
        return 'L2';
    }
}

class Driver {
    public run(): string {
        let g = new Greeter();
        g.bump();
        g.bump();
        let s = new Sub();
        return 'tag=' + g.tag() + ' sub=' + s.tag() + ' n=' + g.n + ' calc=' + Calc.calc() + ' base=' + Calc.base;
    }

    public lazy(): string {
        return new LazyHolder().get();
    }
}

function main(args: FixedArray<string>): int {
    return 0;
}
