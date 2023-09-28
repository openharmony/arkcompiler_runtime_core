/*---
desc: null safety -> Not-null operator
assert: For an expression e!, if the type of e is nullable, an ensure not-null expression checks whether the evaluation of e is equal to null and, if it is, throws a NullPointerException. 
---*/

function main(): void {
  let res = 0;
  let a: Int|null = null;
  let b: Int = 4;

  try {
    let x = a! + b;
  } catch (e: NullPointerException) {
    res = 1;
  } catch (e) {
    res = 0;
  }

  assert res == 1;
}
