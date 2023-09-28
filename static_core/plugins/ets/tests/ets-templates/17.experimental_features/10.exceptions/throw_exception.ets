/*---
desc: Define new Exception and used it.
assert: An exception can be thrown explicitly using the throw statement or implicitlyby the Virtual Machine or Runtime system.
tags: [compile-only]
---*/

class TestException extends Exception {}

function bar(): void throws {
  throw new TestException();
}