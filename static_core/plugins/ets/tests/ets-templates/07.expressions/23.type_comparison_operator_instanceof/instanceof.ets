{% for io in instanceof %}

/*---
desc: instanceof operator test
---*/

class A {}
class B {}

function main(): void {
  let a = new A;
  let b = new B;
  let с : Object | null = null;

  assert {{io.negation}}({{io.left}} instanceof {{io.right}});
}

{% endfor %}