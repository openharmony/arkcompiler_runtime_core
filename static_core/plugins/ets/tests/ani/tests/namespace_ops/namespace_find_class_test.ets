/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace test {
    namespace A {
        class B {
            x: int = 0;
            public intMethod(a0: int, a1: int): int
            {
                return 0;
            }
        }
    }

    namespace C {
        class D {
            x: int = 0;
            public intMethod(a0: int, a1: int): int
            {
                return a0 + a1;
            }
        }

        final class E {
            x: int = 0;
            public intMethod(a0: int, a1: int): int
            {
                return a0 + a1;
            }
        }

        abstract class F {
            public abstract Area(a0: int, a1: int) : int;
        }

        class G extends F {
            private x: int = 0;

            public Area(a0: int, a1: int) : int
            {
                return a0 + a1;
            }
            public methodA(a0: int, a1: int): int
            {
                return a0 + a1;
            }

            static methodB(a0: int, a1: int): int
            {
                return a0 + a1;
            }
        }

        class H {
        }
    }
}

namespace I {
    namespace A {
        class B {
            x: int = 0;
            public intMethod(a0: int, a1: int): int
            {
                return a0 + a1;
            }
        }
    }
}

class C {
    x: int = 0;
    public intMethod(a0: int, a1: int): int
    {
        return a0 + a1;
    }
}

namespace J {
    class C {
        x: int = 0;
        public intMethod(a0: int, a1: int): int
        {
            return a0 + a1;
        }
    }

    class B extends C{

    }

    class A extends B {

    }
}

namespace A {
    class C {
        x: int = 0;
        public intMethod(a0: int, a1: int): int
        {
            return a0 + a1;
        }
    }
    namespace B {
        class C {
            x: int = 0;
            public intMethod(a0: int, a1: int): int
            {
                return a0 + a1;
            }
        }
    }
}