/*---
desc: Method declaration with different modifiers 
assert: It is a compile-tme error if the same method modifier appears more than once in a method declaration.
params: {{mod}}
tags: [compile-only, negative]
---*/

class A {
  abstract abstract run(): void { 

  }
}