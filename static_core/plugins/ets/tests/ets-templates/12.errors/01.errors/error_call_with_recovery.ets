/*---
desc: Call Error with try...recovery.
assert: In most case errors are caused by Virtial Machine or by standard libraries. In case it is necessary to cause an error explicitly, use “Error Statements”.
---*/

class TestError extends Error {}

function main(): void {
  let i: int = 1;
  
  try {
    throw new TestError();
  }
  catch (p: TestError) {
    i = 5;
  }

  assert i == 5;
}