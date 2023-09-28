{% for member in members %}
/*---
desc: Default access modifiers.
assert: If no modifier is explicitly provided, iy is implicitly declared public dy default.
params: {{member}}
---*/

class A {
  y: int;

  {{member.body}}
}

function main(): void {
    let instance: A = new A();

    {{member.call}};
}

{% endfor %}
