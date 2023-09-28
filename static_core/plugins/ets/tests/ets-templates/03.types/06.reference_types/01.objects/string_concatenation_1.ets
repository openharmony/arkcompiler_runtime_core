/*---
desc: Concatenation object with declared .toString() method.
---*/

class A {
  override toString(): String {
    return "a";
  }
}

function main(): void {
    let a: Object = new A();
    let b: String = "sa";
    let c: String = a + b;
    assert c == "asa";
}
