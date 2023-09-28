{% for v in values %}
/*---
desc: check left shift operation for unsigned byte
---*/

function main(): void {
    const a: ubyte = {{v.left}}
    const b: int = {{v.right}}
    assert (a << b) == {{v.result}}
}

{% endfor %}
