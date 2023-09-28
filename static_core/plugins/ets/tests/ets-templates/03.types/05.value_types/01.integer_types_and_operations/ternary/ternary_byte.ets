{% for v in values %}
/*---
desc: check ternary if-else operation with condition of byte operand
---*/

function main(): void {
    const a: byte = {{v.condition}}
    const b: byte = {{v.ifTrue}}
    const c: byte = {{v.ifFalse}}
    assert (a ? b : c) == {{v.result}}
}

{% endfor %}
