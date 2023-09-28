const correct_res : int = 42;
let res : int = 0;

interface A {
    static foo() : void;
}

class B implements A {
    public override foo() : void {
        res = 42;
    }
}

function call(a: A) : void {
    a.foo();
}

function main(): void {
    let b: B = new B();
    call(b);
    assert res == correct_res: "ERROR: Wrong res after virtual call";
}
