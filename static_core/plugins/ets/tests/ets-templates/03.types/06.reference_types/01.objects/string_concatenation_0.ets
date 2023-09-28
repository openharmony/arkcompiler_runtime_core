/*---
desc: Concatenation object with declared .toString() method.
---*/

class A {}

function main(): void {
    let a: Object = new A();
    let b: String = "as";
    let c: String = a + b;
    assert c == "Aas";
}
