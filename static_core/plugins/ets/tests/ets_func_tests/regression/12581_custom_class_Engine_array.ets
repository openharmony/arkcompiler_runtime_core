/*---
desc:  {function: Engine_array}
---*/


interface Iface<T> {
    returnArray() : T[];
}

class Provider<T> implements Iface<T> {
    override returnArray(): T[] {
        return new T[5];
    }
}

function main() : int {
    let provider = new Provider<Engine<Propan>>();
    let result = provider.returnArray();
    if (result.length != 5) return 1;

    for (let i = 0; i< result.length; i++) {
        result[i] = new Engine<Propan>();
    }
    for (let i = 0; i< result.length; i++) {
        console.println(result[i]);
        if (!result[i].name.equals("Propan")) return 1;
    }
    return 0;
}

class Engine<T> {
    public name : T;
}


class Propan {
    constructor() {
	  this.name = "Propan"
    }
    public name: String
}
