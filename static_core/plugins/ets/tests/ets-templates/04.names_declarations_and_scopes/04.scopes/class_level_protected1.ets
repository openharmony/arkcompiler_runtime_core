/*---
desc: Class level scope, protected access
---*/

let x = -10

class C {
    protected x: int = 1;
}

class D extends C {
    public get():int {
        return this.x;
    }
    public put(p: int) {
        this.x = p;
    }
}

function main(): void {
    // access from D
    let d:D = new D()
    assert d.get() == 1
    d.put(42)
    assert d.get() == 42

    // module level scope is not affected
    assert x == -10
}
