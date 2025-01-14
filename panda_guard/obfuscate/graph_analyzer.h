/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_GUARD_OBFUSCATE_GRAPH_ANALYZER_H
#define PANDA_GUARD_OBFUSCATE_GRAPH_ANALYZER_H

#include "entity.h"

namespace panda::guard {

class GraphAnalyzer {
public:
    GraphAnalyzer() = delete;

    ~GraphAnalyzer() = delete;
    NO_COPY_SEMANTIC(GraphAnalyzer);

    /**
     * Get Related Lda.Str instruction
     * @param inIns origin inst
     * @param outIns related Lda.Str inst; if not found, outIns.ins_ will be nullptr
     * @param index reg index, -1 as using default value
     */
    static void GetLdaStr(const InstructionInfo &inIns, InstructionInfo &outIns, int index = -1);

    /**
     * Get Related instruction of DefineMethod
     * @param inIns input ins: definemethod
     * @param defineIns ins info of define method
     * @param nameIns ins info of name define
     */
    static void HandleDefineMethod(const InstructionInfo &inIns, InstructionInfo &defineIns, InstructionInfo &nameIns);

    /**
     * Get Related instruction of definepropertybyname
     * @param inIns input ins
     * @param defineIns related define ins
     */
    static void HandleDefineProperty(const InstructionInfo &inIns, InstructionInfo &defineIns);

    /**
     * Get class is component
     *
     * @Component
     * struct ClassName{}
     *
     * @ComponentV2
     * struct ClassName{}
     *
     * @param inIns defineclasswithbuffer instruction
     * @return is component class
     */
    static bool IsComponentClass(const InstructionInfo &inIns);
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_OBFUSCATE_GRAPH_ANALYZER_H
