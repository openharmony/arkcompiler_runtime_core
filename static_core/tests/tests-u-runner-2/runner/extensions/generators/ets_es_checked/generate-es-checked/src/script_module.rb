# Copyright (c) 2023-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

module ScriptModule
    def paramOf(*args)
        Proc.new {
            args
        }
    end

    def paramGroupsConcat(*every)
        Proc.new {
            ans = []
            every.each { |e|
                ans.cocnat(e.())
            }
            ans
        }
    end

    def emptyRest
        Proc.new {
            [[]]
        }
    end

    def combinationRest(*a)
        Proc.new {
            (0..a.size).collect { |i|
                a.combination(i).to_a
            }.flatten(1)
        }
    end

    def Inifinity
        "Infinity"
    end

    def NaN
        "NaN"
    end

    def undefined
        "undefined"
    end
end

class Vars < OpenStruct
    include ScriptModule
end
