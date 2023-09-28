
/*---
desc: Cast to Object class.
---*/

class A {}

function main(): void {
    let a: A = new A()
    let o: Object = a as Object; // check for explicit cast
}
