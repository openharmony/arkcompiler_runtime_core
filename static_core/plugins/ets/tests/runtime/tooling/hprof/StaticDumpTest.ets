/**
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

class AllPrimitives {
    public z: boolean;
    public b: byte;
    public s: short;
    public i: int;
    public j: long;
    public f: float;
    public d: double;
}

class Node {
    public name: string;
    public next: Node | null;
    public static totalNodes: int = 0;

    constructor(n: string) {
        this.name = n;
        this.next = null;
        Node.totalNodes++;
    }
}

class Animal {
    public species: string;
    public age: int;
    constructor(s: string, a: int) {
        this.species = s;
        this.age = a;
    }
}

class Dog extends Animal {
    public breed: string;
    constructor(s: string, a: int, b: string) {
        super(s, a);
        this.breed = b;
    }
}

class Cat extends Animal {
    public color: string;
    constructor(s: string, a: int, c: string) {
        super(s, a);
        this.color = c;
    }
}

class Person {
    public age: int;
    public weight: double;
    public height: double;
    public flags: byte;
    public id: short;
    public bigId: long;
    public name: string;
    public pet: Dog | null;
    public active: boolean;

    constructor(a: int, w: double, h: double, f: byte, i: short, b: long,
                n: string, p: Dog | null, act: boolean) {
        this.age = a;
        this.weight = w;
        this.height = h;
        this.flags = f;
        this.id = i;
        this.bigId = b;
        this.name = n;
        this.pet = p;
        this.active = act;
    }
}

class DataSet {
    public label: string;
    public values: int[];
    constructor(l: string, v: int[]) {
        this.label = l;
        this.values = v;
    }
}

class FriendGroup {
    public groupName: string;
    public members: Person[];
    constructor(g: string, m: Person[]) {
        this.groupName = g;
        this.members = m;
    }
}

interface IDumpRunnable {
    run(): void;
}

interface IDumpSerializable {
    serial(): int;
}

// Interface diamond: IDumpComparable extends two interfaces, one of which
// (IDumpRunnable) is also implemented by Mid below. Leaf ends up satisfying
// IDumpRunnable via two paths - exercises multi-interface inheritance on the
// static side. (Prefixed to avoid colliding with ETS stdlib names like
// Comparable.)
interface IDumpComparable extends IDumpRunnable, IDumpSerializable {
    compare(): int;
}

class Base {
    public static counter: int = 0;
    public static instances: Base[] = [];
    public id: int = 0;
    public name: string = '';
    public init(id: int, name: string): void {
        this.id = id;
        this.name = name;
        Base.counter = Base.counter + 1;
    }
    public increment(): int {
        Base.counter = Base.counter + 1;
        return Base.counter;
    }
    public describe(): string {
        return this.name;
    }
}

class Mid extends Base implements IDumpRunnable {
    public tag: string = '';
    public run(): void {
        this.tag = 'ran';
    }
    public greet(): string {
        return 'hi';
    }
}

class Leaf extends Mid implements IDumpSerializable, IDumpComparable {
    public score: double = 0.0;
    public serial(): int {
        return this.id;
    }
    public compare(): int {
        return 0;
    }
    public setScore(s: double): void {
        this.score = s;
    }
}

class TaggedHolder {
    public intValue: Any = undefined;
    public doubleValue: Any = undefined;
    public boolValue: Any = undefined;
    public objectValue: Any = undefined;
    public specialValue: Any = undefined;
    public values: Any[] = [];

    public static staticObjectValue: Any = undefined;
    public static staticSpecialValue: Any = undefined;
    public static staticIntValue: Any = undefined;
}

class WeakTarget {
    public marker: string;

    constructor(marker: string) {
        this.marker = marker;
    }
}

class SizedObjects {
    public shortArray: int[] = [1];
    public longArray: int[] = [1, 2, 3, 4, 5, 6, 7, 8];
    public shortText: string = 'x';
    public longText: string = 'a considerably longer string';
}

// Static holder so test instances remain reachable after main() returns
// (the snapshot is built from root reachability, not a full-heap walk).
// p1/p2 and dog are kept reachable transitively via group.members / p1.pet.
class SnapshotFixtures {
    public static ap: AllPrimitives | null = null;
    public static ap2: AllPrimitives | null = null;
    public static n1: Node | null = null;
    public static n2: Node | null = null;
    public static n3: Node | null = null;
    public static cat: Cat | null = null;
    public static ds: DataSet | null = null;
    public static group: FriendGroup | null = null;
    public static unicodeText: string = '';
    // #13: Circular reference objects - BFS traversal must not infinite loop
    public static friendA: CircularFriendA | null = null;
    public static friendB: CircularFriendB | null = null;
    public static tagged: TaggedHolder | null = null;
    public static weakRetainedTarget: WeakTarget | null = null;
    public static weakRetainedRef: WeakRef<WeakTarget> | null = null;
    public static weakOnlyRef: WeakRef<WeakTarget> | null = null;
    public static sized: SizedObjects | null = null;

    // SnapshotFixtures is never instantiated. Its metadata is discovered only
    // through the managed class mirror, including this declared method.
    public static mirrorOnlyMethod(): string {
        return 'mirror-only';
    }
}

// #13: Circular reference classes A.friend=B, B.friend=A - tests BFS termination.
// The static dump's BFS reachability traversal must handle circular references without infinite loop.
class CircularFriendA {
    public name: string;
    public friend: CircularFriendB | null; // A -> B
    constructor(n: string, f: CircularFriendB | null) {
        this.name = n;
        this.friend = f;
    }
}

class CircularFriendB {
    public name: string;
    public friend: CircularFriendA | null; // B -> A
    constructor(n: string, f: CircularFriendA | null) {
        this.name = n;
        this.friend = f;
    }
}

function main(): void {
    let ap = new AllPrimitives();
    ap.z = true;
    ap.b = 0x7F as byte;
    ap.s = 0x7FFF as short;
    ap.i = 42;
    ap.j = 9007199254740991 as long;
    ap.f = Double.toFloat(3.14);
    ap.d = 2.718281828;

    let n1 = new Node('alpha');
    let n2 = new Node('beta');
    let n3 = new Node('gamma');
    n1.next = n2;
    n2.next = n3;

    let dog = new Dog('dog', 3, 'beagle');
    let cat = new Cat('cat', 2, 'orange');

    let p1 = new Person(25, 70.5, 175.0, 0x01 as byte, 100 as short, 9999999999 as long,
                        'Alice', dog, true);
    let p2 = new Person(30, 80.0, 180.0, 0x02 as byte, 200 as short, 8888888888 as long,
                        'Bob', null, false);

    let ds = new DataSet('temperatures', [23, 25, 21, 28, 30]);

    let group = new FriendGroup('friends', [p1, p2]);

    let ap2 = new AllPrimitives();
    ap2.i = 999;

    // Multi-level inheritance + interface implementation. Exercises static
    // field writes (counter primitive, instances array reference), the
    // superClass chain Leaf->Mid->Base->Object, and per-class declared methods.
    let base = new Base();
    base.init(1, 'base');
    let mid = new Mid();
    mid.init(2, 'mid');
    mid.run();
    mid.greet();
    let leaf = new Leaf();
    leaf.init(3, 'leaf');
    leaf.setScore(98.5);
    leaf.serial();
    leaf.compare();
    Base.instances = [base, mid, leaf];
    let nextCounter = base.increment();
    let label = leaf.describe();

    // Keep test instances reachable after main() returns so a root-reachability
    // dump (BFS from roots, not a full-heap walk) still observes them. Without
    // these static holders the instances would be unreachable garbage once
    // main()'s locals go out of scope (the test runtime uses epsilon GC, which
    // doesn't collect, but reachability is what the snapshot is built from).
    SnapshotFixtures.ap = ap;
    SnapshotFixtures.ap2 = ap2;
    SnapshotFixtures.n1 = n1;
    SnapshotFixtures.n2 = n2;
    SnapshotFixtures.n3 = n3;
    SnapshotFixtures.cat = cat;
    SnapshotFixtures.ds = ds;
    SnapshotFixtures.group = group;
    SnapshotFixtures.unicodeText = '中文😀';

    let tagged = new TaggedHolder();
    let taggedObject = new Node('tagged-object-only');
    let taggedArrayObject = new Node('tagged-array-only');
    let taggedStaticObject = new Node('tagged-static-only');
    tagged.intValue = -7;
    tagged.doubleValue = 2.5;
    tagged.boolValue = true;
    tagged.objectValue = taggedObject;
    tagged.specialValue = undefined;
    tagged.values = [-9, 6.25, false, taggedArrayObject, undefined, null];
    TaggedHolder.staticObjectValue = taggedStaticObject;
    TaggedHolder.staticSpecialValue = null;
    TaggedHolder.staticIntValue = 77;
    SnapshotFixtures.tagged = tagged;

    // Keep one weak referent alive through an independent strong root. The
    // other referent remains reachable only through WeakRef; epsilon GC keeps
    // its memory allocated so the dump's own reachability rules are exercised.
    let weakRetainedTarget = new WeakTarget('weak-retained');
    let weakOnlyTarget = new WeakTarget('weak-only');
    SnapshotFixtures.weakRetainedTarget = weakRetainedTarget;
    SnapshotFixtures.weakRetainedRef = new WeakRef<WeakTarget>(weakRetainedTarget);
    SnapshotFixtures.weakOnlyRef = new WeakRef<WeakTarget>(weakOnlyTarget);
    SnapshotFixtures.sized = new SizedObjects();

    // #13: Create circular reference A.friend=B, B.friend=A - BFS must not infinite loop.
    // The static dump's reachability BFS uses a visited-set so circular references are handled.
    let friendA = new CircularFriendA('Alice', null);
    let friendB = new CircularFriendB('Bob', null);
    friendA.friend = friendB; // A -> B
    friendB.friend = friendA; // B -> A (circular)
    SnapshotFixtures.friendA = friendA;
    SnapshotFixtures.friendB = friendB;
}
