{% for v in values %}
/*---
desc: check ternary if-else operation with condition of unsigned integer operand
---*/

function main(): void {
    const a: uint = {{v.condition}}
    const b: uint = {{v.ifTrue}}
    const c: uint = {{v.ifFalse}}
    assert (a ? b : c) == {{v.result}}
}

{% endfor %}
