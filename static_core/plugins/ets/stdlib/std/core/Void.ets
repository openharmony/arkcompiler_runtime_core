package std.core;

export final class void {
    static void_instance = new void();
    private constructor() {}

}
export const Void = void.void_instance;
