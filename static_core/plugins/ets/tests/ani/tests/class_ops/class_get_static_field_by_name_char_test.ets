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

class GetCharStatic {
    static name: char = 'b';
    static age: int = 20;
    static specia2: string = "";
    static specia3: null = null;
    static specia4: undefined  = undefined ;
    static specia5: char = "\n";
    static specia6: char = "\r";
    static specia7: char = "\t";
    static specia8: char = "\b";
    static specia9: char = "\a";
    static specia10: char = "\f";
    static specia11: char = "\v";
    static specia12: char = "\0";
    static charMin: char = 0;
    static charMax: char = 65535;
}

class CharStaticA extends GetCharStatic {
    static char_value: char = 'a';
}

final class CharStaticFinal extends GetCharStatic {
    static char_value: char = 'a';
}