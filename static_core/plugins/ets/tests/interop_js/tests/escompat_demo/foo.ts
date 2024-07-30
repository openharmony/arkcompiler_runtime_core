// Should be autogenerated from foo.sts

(function __startup() {
    let etsVm = globalThis.etsVm;
    module.exports = {
        FooClass: etsVm.getClass("LFooClass;"),
        FooFunction: etsVm.getFunction("LETSGLOBAL;", "FooFunction"),
        BarFunction: etsVm.getFunction("LETSGLOBAL;", "BarFunction"),
    }
})();

export declare class FooClass {
    constructor(name: String);
    toString(): String;
    name: String;
}
export declare function FooFunction(arr: Array<FooClass>): String;
export declare function BarFunction(): Array<FooClass>;
