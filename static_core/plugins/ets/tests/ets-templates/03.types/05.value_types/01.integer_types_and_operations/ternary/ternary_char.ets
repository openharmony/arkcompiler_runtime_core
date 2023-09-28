{% for v in values %}
/*---
desc: check ternary if-else operation with condition of char operand
---*/

function main(): void {
    const a: char = {{v.condition}}
    const b: char = {{v.ifTrue}}
    const c: char = {{v.ifFalse}}
    assert (a ? b : c) == {{v.result}}
}

{% endfor %}
