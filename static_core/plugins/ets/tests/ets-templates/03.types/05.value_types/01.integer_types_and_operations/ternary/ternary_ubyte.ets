{% for v in values %}
/*---
desc: check ternary if-else operation with condition of unsigned byte operand
---*/

function main(): void {
    const a: ubyte = {{v.condition}}
    const b: ubyte = {{v.ifTrue}}
    const c: ubyte = {{v.ifFalse}}
    assert (a ? b : c) == {{v.result}}
}

{% endfor %}
