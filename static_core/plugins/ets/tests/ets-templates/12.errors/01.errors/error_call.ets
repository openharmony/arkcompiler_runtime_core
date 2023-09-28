/*---
desc: Call Error.
assert: In most case errors are caused by Virtial Machine or by standard libraries. In case it is necessary to cause an error explicitly, use “Error Statements”.
tags: [negative]
---*/

class TestError extends Error {}

function main(): void {
  throw new TestError();
}