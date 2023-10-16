class A {
    foo<T>(a0: int): void {}
    bar(): void {
        this.foo<Int>(2);
    }
}

function main(): void {
    let a : A = new A()
    assert (a != null) : "Generics allocating error #1!"
    a.bar()
}
