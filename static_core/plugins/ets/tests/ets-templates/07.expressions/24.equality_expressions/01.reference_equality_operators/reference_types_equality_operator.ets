{% for eq in reference_eq %}

/*---
desc: The value equality operator a == b semantically equals a === null ? b === null a.equals(b) if both of its operands are of reference types.
params: {{eq.left}} {{eq.op}} {{eq.right}} is {{eq.result}}
---*/

class A{}
class B{}

function main(): void {
  let a = new A()	
  let b = new B()
  assert ({{eq.left}} {{eq.op}} {{eq.right}}) == {{eq.result}};
}

{% endfor %}