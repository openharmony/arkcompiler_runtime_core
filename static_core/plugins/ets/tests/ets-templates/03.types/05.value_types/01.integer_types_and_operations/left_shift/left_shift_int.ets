{% for v in values %}
/*---
desc: check left shift operation for integer
---*/

function main(): void {
    const a: int = {{v.left}}
    const b: int = {{v.right}}
    assert (a << b) == {{v.result}}
}

{% endfor %}
