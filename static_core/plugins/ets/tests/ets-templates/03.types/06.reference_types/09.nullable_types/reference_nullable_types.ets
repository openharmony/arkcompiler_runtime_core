{% for n in nullables %}

/*---
desc: Nullable types
assert: To specify a nullable version of type T, one needs to use T|null as a type.
---*/

class A {}

function main(): void {
  let x: {{n.type}} = {{n.value}};

  assert x == {{n.value}};
}

{% endfor %}
