{% for c in cases %}
/*---
desc: >-
    Let G name a generic type declaration with n type parameters.
    There is an unchecked conversion from the raw class or interface type G
    to any parameterized type of the form G<T1,...,Tn>.
    There is an unchecked conversion from the raw array type G[]k to any array
    type of the form G<T1,...,Tn>[]k.
---*/

{{c.defs}}

function main(): int {
  let src: {{c.stype}} = {{c.val|safe}};
  let dst: {{c.dtype}} = src as {{c.dtype}};
  if (src !== dst ) {  // reference equality
      return 1;
  }
  return 0;
}
{% endfor %}
