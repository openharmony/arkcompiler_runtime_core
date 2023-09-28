{% for v in values %}
/*---
desc: check left shift operation for unsigned integer
---*/

function main(): void {
    const a: uint = {{v.left}}
    const b: int = {{v.right}}
    assert (a << b) == {{v.result}}
}

{% endfor %}
