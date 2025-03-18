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

class GetFloatStatic {
    static name: String = "Bob";
    static age: float = 20.0f;
    static specia1: int = 0;
    static specia3: string = "";
    static specia4: null = null;
    static specia5: undefined  = undefined ;
    static floatMin: float = -3.40282346638528860e+38;
    static minpositive: float =  1.17549435e-38;
    static floatMax: float = 3.40282346638528860e+38;
}

class FloatStaticA extends GetFloatStatic {
    static float_value: float = 0.0f;
}

final class FloatStaticFinal extends GetFloatStatic {
    static float_value: float = 0.0f;
}