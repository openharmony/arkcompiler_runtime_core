/*---
desc: Subtyping relation among the generic class and Object
---*/

class B<T> {}

function main(): void {
  let instance: Object = new B<String>();
}