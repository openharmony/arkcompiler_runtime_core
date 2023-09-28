const correct_res : int = 4;

let cb: (a: int, b: int): int;

class A {
    foo(a: int, b: int): int {
        return a + b + this.c;
    }

    c: int = 1;
}

function main(): void {
    let a = new A();
    cb = a.foo;
    assert cb(1, 2) == correct_res: "ERROR: Wrong res after lambda call";
}
