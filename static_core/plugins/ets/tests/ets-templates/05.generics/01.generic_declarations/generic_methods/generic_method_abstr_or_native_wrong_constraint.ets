{% for wc in wrong_constraints %}
{% for mm in abstr_or_native_method_modifiers %}

/*---
desc: wrong constraints for type variable
tags: [compile-only, negative]
params: >
  {{wc}}
  {{mm}}
---*/

class Point {
	{{mm}} foo<X extends {{wc}}>(): void;
}

function main(): void {}

{% endfor %}
{% endfor %}

