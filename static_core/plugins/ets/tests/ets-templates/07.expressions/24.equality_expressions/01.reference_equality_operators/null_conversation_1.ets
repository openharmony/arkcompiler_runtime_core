/*---
desc: The result of === at runtime is true if both operand values are null
---*/


function main(): void {
  assert (null === null) == true;
  assert (null == null) == true;
  assert (null !== null) == false;
  assert (null != null) == false;
}
