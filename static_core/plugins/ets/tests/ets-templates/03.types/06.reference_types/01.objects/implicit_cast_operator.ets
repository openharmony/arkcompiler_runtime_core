/*---
desc: Class instance implicit cast to Object.
---*/

class A {
}

function main(): void {
    let a: A = new A();
    let o: Object = a;
}
