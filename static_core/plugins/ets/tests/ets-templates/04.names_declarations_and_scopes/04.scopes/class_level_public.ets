/*---
desc: Class level scope, public access
---*/

let x = -10

class C {
    public x: int = 1;
}

class D {
    private x: C;

    public constructor(p: C) {
        this.x = p;
    }
    public get():int {
        return this.x.x;
    }
    public put(p: int) {
        this.x.x = p;
    }
}

function main(): void {
    // access from D
    let d:D = new D(new C())
    assert d.get() == 1
    d.put(42)
    assert d.get() == 42

    // access from module
    let c:C = new C();
    assert c.x == 1
    c.x = 100
    assert c.x == 100

    // module level scope is not affected
    assert x == -10
}
