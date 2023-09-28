{% for lit in bigint_literals %}
/*---
desc: >-
    BigInt literals use decimal base only. It is a sequence of digits which ends with symbol 'n'.
    BigInt literals are always of the type bigint.
params: '{{lit}}'
---*/

function main() {
    let x = {{lit}};
    assert x == {{lit}}
}
{% endfor %}
