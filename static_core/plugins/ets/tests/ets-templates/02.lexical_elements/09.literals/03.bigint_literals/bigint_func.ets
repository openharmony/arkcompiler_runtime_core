{% for f in bigint_functions %}
/*---
desc: >-
    Built-in functions allow to convert strings representing numbers or any integer values into bigint ones.
---*/

function main() {
    let x: bigint = {{f.func}};
    assert x == {{f.res}}
}
{% endfor %}
