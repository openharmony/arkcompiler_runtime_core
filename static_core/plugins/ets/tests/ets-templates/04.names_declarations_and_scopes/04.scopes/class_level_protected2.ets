/*---
desc: Class level scope, mixed access
---*/

let x = -10

class C {
    protected x: int = 100;
}

class D extends C {
    private x: int = 10;

    public get():int {
        return this.x;
    }
    public put(p: int) {
        this.x = p;
    }

    public getSuper():int {
        return super.x;
    }
    public putSuper(p: int) {
        super.x = p;
    }
}

function main(): void {
    let d: D = new D()

    assert d.get() == 10
    assert d.getSuper() == 100

    d.put(42)
    assert d.get() == 42
    assert d.getSuper() == 100

    d.putSuper(22)
    assert d.get() == 42
    assert d.getSuper() == 22

    // module level scope is not affected
    assert x == -10
}
