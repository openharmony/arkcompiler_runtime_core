/*---
desc: Initializing a class internally
tags: [compile-only]
---*/

class MyClass {
  property: MyClass;
}
  
function main(): void {
  let obj: MyClass = { property: {} };
  obj.property = obj;
}