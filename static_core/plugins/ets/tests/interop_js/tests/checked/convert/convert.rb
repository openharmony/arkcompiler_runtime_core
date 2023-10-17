#!/usr/bin/env ruby
# Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

TypeInfo = Struct.new(:name, :value, :type, :skip_cast) do
    def etsType
        type || name
    end
end

@type_infos = [
    TypeInfo.new('String', '"abc"'),
    TypeInfo.new('boolean', 'true'),
    TypeInfo.new('byte', '127'),
    TypeInfo.new('short', '32767'),
    TypeInfo.new('int', '2147483647'),
    TypeInfo.new('long', '9223372036854775'),
    TypeInfo.new('char', '65535'),
    TypeInfo.new('int_array', '[1, 2]', 'int[]', true),
    TypeInfo.new('String_array', '["ab", "cd"]', 'String[]', true),
    TypeInfo.new('const_object', 'const_obj', 'EtsClass', true),
    TypeInfo.new('object', 'create_obj()', 'EtsClass | null', true),
    TypeInfo.new('null_object', 'create_null_obj()', 'EtsClass | null', true),
]