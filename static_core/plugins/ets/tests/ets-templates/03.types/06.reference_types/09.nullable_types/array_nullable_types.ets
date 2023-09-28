{% for n in nullables %}
/*---
desc: Nullable types
assert: To specify a nullable version of type T, one needs to use T|null as a type.
---*/

{{n.decl}}

function main(): void {
    let x: {{n.type}} = {{n.vals}};
    x[0] = {{n.val}}
    assert {{n.assert}};
}
{% endfor %}
