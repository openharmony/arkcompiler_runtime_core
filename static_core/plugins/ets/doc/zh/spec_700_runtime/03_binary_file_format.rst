..
    Copyright (c) 2021-2026 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


.. _RT Binary File Format:

二进制文件格式
********************************************************************************

|LANG| 的二进制文件格式设计时考虑了以下目标：

- 紧凑性；
- 快速访问信息；
- 低内存占用；
- 可扩展性；以及
- 兼容性。

**紧凑性**。许多移动应用会使用大量类型、方法和字段。
其数量非常庞大，无法装入 16 位无符号整数。
结果是，应用开发者需要创建多个文件，而且某些数据无法去重。

当前的二进制文件格式必须扩展这些限制以符合现代需求。
为此，二进制文件中的所有直接引用都采用 4 字节长度，以支持 4GB 寻址。

字节码和元数据中使用 16 位索引来引用类、方法和字段，以获得更好的紧凑性。
一个文件可以包含多个索引，每个索引覆盖文件的一部分，具体见 :ref:`RegionHeader <RT RegionHeader>`。

文件格式使用 :ref:`TaggedValue <RT TaggedValue>` 来实现紧凑值存储，
只存储可用信息，并避免为缺失信息写入零偏移。

**快速访问信息**。二进制文件格式必须支持快速访问信息。
这意味着应避免冗余引用。二进制文件格式也应尽量避免数据索引
（例如已排序的字符串列表）。不过，当前二进制文件格式支持一个索引，
它是指向类的偏移量排序列表。
这个紧凑索引使得在加载二进制文件时能够快速查找类型定义。

当前二进制格式区分两类类、字段和方法：

- *Foreign* 实体：在当前二进制文件中被引用，但定义在其他二进制文件中；
- *Local* 实体：定义在当前二进制文件中。

由于某个 *local* 实体和对应的 *foreign* 实体具有相同的头部，
因此当二进制文件引用类、字段或方法时，指向 *foreign* 与 *local* 实体的偏移可以互换使用。

在运行时，只要检查某个偏移是否属于 foreign 区域
（``[foreign_off; foreign_off + foreign_size)``），就可以推断某个类、字段或方法的类型。
*foreign* 实体的定义通过 foreign 实体头中的信息在其他二进制文件中找到。
*local* 实体的定义则通过 *local* 实体偏移在当前二进制文件中找到。

**低内存占用**。实践证明，应用通常不会使用文件中的大部分数据。
通过把高频使用的数据聚集在一起，可以显著降低文件的内存占用。
所描述的二进制文件格式通过使用偏移量，并且不要求实体彼此之间必须按特定相对位置放置，来支持这一特性。

**可扩展性与兼容性**。二进制文件格式通过版本号支持未来变更。
头部中的版本字段长度为 4 字节，并编码为字节数组，以避免在不同字节序平台上产生误解释。
支持二进制文件格式版本 ``N`` 的工具，也必须支持版本 ``N-1`` 的格式。

|

.. _RT Alignment:

对齐
================================================================================

为提高数据访问速度，大多数实体都按 4 字节对齐。
所有例外情况都将在后文相应描述中显式说明。

|

.. _RT Endianness:

字节序
================================================================================

所有多字节值都采用小端序，因为大多数目标体系结构都使用小端序。

|

.. _RT Offsets:

偏移
================================================================================

除非另有说明，所有偏移都从文件起始位置开始计算。

除某些明确说明的特殊情况外，偏移值不能落在区间 ``[0; sizeof(Header))`` 内。

|

.. _RT Data Types:

数据类型
================================================================================

.. _RT Integer Types:

整数类型
--------------------------------------------------------------------------------

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Type
     - Description

   * - ``uint8_t``
     - 8 位无符号整数值

   * - ``uint16_t``
     - 16 位无符号整数值

   * - ``uint32_t``
     - 32 位小端无符号整数值

   * - ``uleb128``
     - 使用 leb128 编码的无符号整数值

   * - ``sleb128``
     - 使用 leb128 编码的有符号整数值

.. _RT String:

字符串格式
--------------------------------------------------------------------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``utf16_length``
     - ``uleb128``
     - ``len << 1 | is_ascii``，其中 ``len`` 为字符串以 UTF-16 码元计的长度

   * - ``data``
     - ``uint8_t[]``
     - 以 MUTF-8（Modified UTF-8）编码的、以 ``\0`` 结尾的字符序列

.. _RT TaggedValue:

TaggedValue 格式
--------------------------------------------------------------------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``tag_value``
     - ``uint8_t``
     - 决定 ``data`` 编码方式的标签。

   * - ``data``
     - ``uint8_t[]``
     - 可选载荷

.. _RT Source Language:

源语言
--------------------------------------------------------------------------------

类和方法都可以指定源语言。方法的默认语言与其所属类相同。
一个文件可以由以下语言编写的源码生成：

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Source
     - Encoding
   * - ``Ark Assembly``
     - ``0x01``

``0x00``、``0x02`` - ``0x0f`` 保留。

|

.. _RT File Layout:

文件布局
================================================================================

二进制文件以位于偏移 ``0`` 的 :ref:`Header <RT Header>` 开始。
其余任何数据都可以从该头部到达。

.. _RT Header:

头部格式
--------------------------------------------------------------------------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 25 15 60
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``magic``
     - ``uint8_t[8]``
     - 魔数字符串 - 'PANDA\0\0\0'。

   * - ``checksum``
     - ``uint8_t[4]``
     - 除 ``magic`` 和 ``checksum`` 字段外的文件 ``adler32`` 校验和。

   * - ``version``
     - ``uint8_t[4]``
     - 文件格式版本。

   * - ``file_size``
     - ``uint32_t``
     - 文件字节大小。

   * - ``foreign_off``
     - ``uint32_t``
     - foreign 区域偏移。该区域只能包含 :ref:`ForeignField <RT ForeignField>`、
       :ref:`ForeignMethod <RT ForeignMethod>` 或 :ref:`ForeignClass <RT ForeignClass>`。
       ``foreign_off`` 不一定指向第一个 *foreign* 实体。
       运行时必须结合 ``foreign_off`` 与 ``foreign_size`` 来判断给定偏移是 *foreign* 还是 *local*。

   * - ``foreign_size``
     - ``uint32_t``
     - foreign 区域的字节大小。

   * - ``num_classes``
     - ``uint32_t``
     - 文件中定义的类数量，也即 ``class_idx_off`` 所指实体中的元素数量。

   * - ``class_idx_off``
     - ``uint32_t``
     - 指向类索引的偏移。该偏移必须指向符合 :ref:`ClassIndex <RT ClassIndex>` 格式的实体。

   * - ``export_data_size``
     - ``uint32_t``
     - 导出数据区域的字节大小。其区域元素由 ``export_data_off`` 指向。

   * - ``export_data_off``
     - ``uint32_t``
     - 导出数据区域偏移。该偏移必须指向符合 :ref:`ExportData <RT ExportData>` 格式的区域。

   * - ``num_lnps``
     - ``uint32_t``
     - 文件中的行号程序数量，也即 ``lnp_idx_off`` 所指实体中的元素数量。

   * - ``lnp_idx_off``
     - ``uint32_t``
     - 指向行号程序索引的偏移。该偏移必须指向符合 :ref:`LineNumberProgramIndex <RT LineNumberProgramIndex>` 格式的实体。

   * - ``num_literalarrays``
     - ``uint32_t``
     - 文件中定义的 literal array 数量，也即 ``literalarray_idx_off`` 所指实体中的元素数量。

   * - ``literalarray_idx_off``
     - ``uint32_t``
     - 指向 literal array 索引的偏移。该偏移必须指向符合 :ref:`LiteralArrayIndex <RT LiteralArrayIndex>` 格式的实体。

   * - ``num_index_regions``
     - ``uint32_t``
     - 文件中的索引区域数量，也即 ``index_section_off`` 所指实体中的元素数量。

   * - ``index_section_off``
     - ``uint32_t``
     - 指向索引区段的偏移。该偏移必须指向符合 :ref:`RegionIndex <RT RegionIndex>` 格式的实体。

**Constraint**: 头部大小必须大于 16 字节。:ref:`FieldType <RT FieldType>` 依赖这一事实。

.. _RT ClassIndex:

ClassIndex 格式
--------------------------------------------------------------------------------

``ClassIndex`` 实体用于在运行时按名称快速查找类型定义。
它组织为一个偏移数组，数组元素指向 :ref:`Class <RT Class>` 或 :ref:`ForeignClass <RT ForeignClass>`。
所有偏移按对应类名排序。

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``offsets``
     - ``uint32_t[]``
     - 指向 :ref:`Class <RT Class>` 或 :ref:`ForeignClass <RT ForeignClass>` 的有序偏移数组。

.. _RT LineNumberProgramIndex:

LineNumberProgramIndex 格式
--------------------------------------------------------------------------------

``LineNumberProgramIndex`` 实体用于更紧凑地引用 :ref:`Line Number Program <RT Line Number Program>`。
它被组织为指向 :ref:`Line Number Program <RT Line Number Program>` 的偏移数组。

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``offsets``
     - ``uint32_t[]``
     - 指向 :ref:`Line Number Program <RT Line Number Program>` 的偏移数组。

.. _RT LiteralArrayIndex:

LiteralArrayIndex 格式
--------------------------------------------------------------------------------

``LiteralArrayIndex`` 实体用于在运行时按索引查找 :ref:`LiteralArray <RT LiteralArray>` 定义。
它被组织为指向 :ref:`LiteralArray <RT LiteralArray>` 的偏移数组。

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``offsets``
     - ``uint32_t[]``
     - 指向 :ref:`LiteralArray <RT LiteralArray>` 的有序偏移数组。

.. _RT RegionIndex:

RegionIndex 格式
--------------------------------------------------------------------------------

``RegionIndex`` 实体用于在运行时查找覆盖指定文件偏移的索引。
它被组织为按顺序排列的 :ref:`RegionHeader <RT RegionHeader>` 数组。

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 20 20 60
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``region_headers``
     - ``RegionHeader[]``
     - 按区域起始偏移排序的文件区域。每个元素都必须符合 :ref:`RegionHeader <RT RegionHeader>` 格式。

**Constraint**: 两个不同区域之间不得彼此重叠。

.. _RT RegionHeader:

RegionHeader 格式
--------------------------------------------------------------------------------

为通过 16 位索引定位文件实体，一个二进制文件会被划分为多个区域。
每个区域的信息都存储在 ``RegionHeader`` 格式中。

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``start_off``
     - ``uint32_t``
     - 区域起始偏移。

   * - ``end_off``
     - ``uint32_t``
     - 区域结束偏移。

   * - ``class_idx_size``
     - ``uint32_t``
     - ``class_idx_off`` 所指索引中的元素数量。最大值为 65536。

   * - ``class_idx_off``
     - ``uint32_t``
     - 指向类索引的偏移。该偏移必须指向 :ref:`ClassRegionIndex <RT ClassRegionIndex>`。

   * - ``method_idx_size``
     - ``uint32_t``
     - ``method_idx_off`` 所指索引中的元素数量。最大值为 65536。

   * - ``method_idx_off``
     - ``uint32_t``
     - 指向方法索引的偏移。该偏移必须指向 :ref:`MethodRegionIndex <RT MethodRegionIndex>`。

   * - ``field_idx_size``
     - ``uint32_t``
     - ``field_idx_off`` 所指索引中的元素数量。最大值为 65536。

   * - ``field_idx_off``
     - ``uint32_t``
     - 指向字段索引的偏移。该偏移必须指向 :ref:`FieldRegionIndex <RT FieldRegionIndex>`。

   * - ``proto_idx_size``
     - ``uint32_t``
     - ``proto_idx_off`` 所指索引中的元素数量。最大值为 65536。

   * - ``proto_idx_off``
     - ``uint32_t``
     - 指向 proto 索引的偏移。该偏移必须指向 :ref:`ProtoRegionIndex <RT ProtoRegionIndex>`。

.. _RT ClassRegionIndex:

ClassRegionIndex 格式
--------------------------------------------------------------------------------

``ClassRegionIndex`` 实体用于在运行时按索引查找类型定义。
它被组织为一个偏移数组，数组元素指向 :ref:`Class <RT Class>` 或 :ref:`ForeignClass <RT ForeignClass>`。

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``types``
     - ``uint32_t[]``
     - 指向 :ref:`Class <RT Class>` 或 :ref:`ForeignClass <RT ForeignClass>` 的偏移。

.. _RT MethodRegionIndex:

MethodRegionIndex 格式
--------------------------------------------------------------------------------

``MethodRegionIndex`` 实体用于在运行时按索引查找方法定义。
它被组织为一个偏移数组，数组元素指向 :ref:`Method <RT Method>` 或 :ref:`ForeignMethod <RT ForeignMethod>`。

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``methods``
     - ``uint32_t[]``
     - 指向 :ref:`Method <RT Method>` 或 :ref:`ForeignMethod <RT ForeignMethod>` 的偏移数组。

.. _RT FieldRegionIndex:

FieldRegionIndex 格式
--------------------------------------------------------------------------------

``FieldRegionIndex`` 实体用于在运行时按索引查找字段定义。
它被组织为一个偏移数组，数组元素指向 :ref:`Field <RT Field>` 或 :ref:`ForeignField <RT ForeignField>`。

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``fields``
     - ``uint32_t[]``
     - 指向 :ref:`Field <RT Field>` 或 :ref:`ForeignField <RT ForeignField>` 的偏移数组。

.. _RT ProtoRegionIndex:

ProtoRegionIndex 格式
--------------------------------------------------------------------------------

``ProtoRegionIndex`` 实体用于在运行时按索引查找 proto 定义。

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``protos``
     - ``uint32_t[]``
     - 指向 :ref:`Proto <RT Proto>` 的偏移数组。

.. _RT ExportData:

ExportData 格式
--------------------------------------------------------------------------------

``ExportData`` 区域提供导出声明的信息，以便其他源文件导入该二进制文件并在编译期解析到其声明。
如果二进制文件作为库分发且源码不可用，这些信息就是必需的。
这些信息由编译器前端读取，并在解析和检查对导入文件的引用时使用。

它按目前已有的两种导入解析机制被组织为两个子部分。

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``metadataEnabled``
     - ``uint32_t``
     - 表示该文件是否输出元数据的标志。
       0 表示未启用元数据输出，其他任意值表示已启用。

   * - ``metadataSize``
     - ``uint32_t``
     - 可选。metadata 字段的字节大小。
       当 ``metadataEnabled`` 为 0 时，该字段不存在。

   * - ``exportTable``
     - ``ClassIndex``
     - 已导出类的类索引。
       如果 ``metadataEnabled`` 为 0，则 ``exportTable`` 的字节大小等于
       ``export_data_size - sizeof(uint32_t)``。
       否则，``exportTable`` 的字节大小按以下公式计算：
       ``export_data_size - sizeof(uint32_t) * 2 - metadataSize``。

   * - ``metadata``
     - ``uint8_t[]``
     - 可选。导出声明的编码后二进制数据。
       当 ``metadataEnabled`` 为 0 时，该字段不存在。

.. _RT ForeignField:

ForeignField 格式
--------------------------------------------------------------------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 15 10 75
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``class_idx``
     - ``uint16_t``
     - 在对应 :ref:`ClassRegionIndex <RT ClassRegionIndex>` 中声明类的索引。
       相应索引项必须是指向 :ref:`Class <RT Class>` 或 :ref:`ForeignClass <RT ForeignClass>` 的偏移。

   * - ``type_idx``
     - ``uint16_t``
     - 在对应 :ref:`ClassRegionIndex <RT ClassRegionIndex>` 中字段类型的索引。
       相应索引项必须符合 :ref:`FieldType <RT FieldType>` 格式。

   * - ``name_off``
     - ``uint32_t``
     - 字段名偏移。该偏移必须指向 :ref:`String <RT String>`。

   * - ``access_flags``
     - ``uleb128``
     - 字段访问标志。该值必须是 :ref:`Field Access Flags <RT Field Access Flags>` 的组合。

.. note::
   用于解析 ``class_idx`` 与 ``type_idx`` 的正确 region index 可以通过 foreign field 的偏移找到。

.. _RT Field:

Field 格式
--------------------------------------------------------------------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 15 20 65
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``class_idx``
     - ``uint16_t``
     - 在对应 :ref:`ClassRegionIndex <RT ClassRegionIndex>` 中声明类的索引。
       相应索引项必须是指向 :ref:`Class <RT Class>` 或 :ref:`ForeignClass <RT ForeignClass>` 的偏移。

   * - ``type_idx``
     - ``uint16_t``
     - 在对应 :ref:`ClassRegionIndex <RT ClassRegionIndex>` 中字段类型的索引。
       相应索引项必须符合 :ref:`FieldType <RT FieldType>` 格式。

   * - ``name_off``
     - ``uint32_t``
     - 字段名偏移。该偏移必须指向 :ref:`String <RT String>`。

   * - ``access_flags``
     - ``uleb128``
     - 字段访问标志。该值必须是 :ref:`Field Access Flags <RT Field Access Flags>` 的组合。

   * - ``field_data``
     - ``TaggedValue[]``
     - 变长 tagged value 列表。每个元素都必须符合 :ref:`TaggedValue <RT TaggedValue>` 格式。
       标签值必须来自 :ref:`Field Tags <RT Field Tags>`，并按标签升序排列
       （``0x00`` 标签除外，它必须位于最后）。

.. note::
   用于解析 ``class_idx`` 与 ``type_idx`` 的正确 region index 可以通过 foreign field 的偏移找到。

.. _RT FieldType:

FieldType
--------------------------------------------------------------------------------

位于区间 ``[0; sizeof(Header))`` 的任意偏移都无效，因为文件前几个字节存放的是头部。
``FieldType`` 编码利用这一事实，在低 4 位中编码字段的基本类型。
非基本类型的值则是指向 :ref:`Class <RT Class>` 或 :ref:`ForeignClass <RT ForeignClass>` 的偏移。
无论哪种情况，:ref:`FieldType <RT FieldType>` 都是 ``uint32_t``。基本类型编码如下：

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Type
     - Encoding

   * - ``u1``
     - ``0x00``

   * - ``i8``
     - ``0x01``

   * - ``u8``
     - ``0x02``

   * - ``i16``
     - ``0x03``

   * - ``u16``
     - ``0x04``

   * - ``i32``
     - ``0x05``

   * - ``u32``
     - ``0x06``

   * - ``f32``
     - ``0x07``

   * - ``f64``
     - ``0x08``

   * - ``i64``
     - ``0x09``

   * - ``u64``
     - ``0x0a``

   * - ``any``
     - ``0x0b``

.. _RT Field Access Flags:

字段访问标志
--------------------------------------------------------------------------------

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``ACC_PUBLIC``
     - ``0x0001``
     - 声明为 public。

   * - ``ACC_PRIVATE``
     - ``0x0002``
     - 声明为 private。

   * - ``ACC_PROTECTED``
     - ``0x0004``
     - 声明为 protected。

   * - ``ACC_STATIC``
     - ``0x0008``
     - 声明为 static。

   * - ``ACC_FINAL``
     - ``0x0010``
     - 声明为 final。

   * - ``ACC_READONLY``
     - ``0x0020``
     - 声明为 readonly。

   * - ``ACC_VOLATILE``
     - ``0x0040``
     - 声明为 volatile。

   * - ``ACC_TRANSIENT``
     - ``0x0080``
     - 声明为 transient。

   * - ``ACC_SYNTHETIC``
     - ``0x1000``
     - 声明为 synthetic；不出现在源代码中。

   * - ``ACC_ENUM``
     - ``0x4000``
     - 声明为枚举元素。

**Constraint**: 对于 :ref:`ForeignField <RT ForeignField>`，仅使用 ``ACC_STATIC`` 标志。
其他标志会被忽略。

.. _RT Field Tags:

字段标签
--------------------------------------------------------------------------------

.. list-table::
   :width: 100%
   :align: left
   :widths: 30 10 10 15 35
   :header-rows: 1

   * - Name
     - Tag
     - Quantity
     - Data Format
     - Description

   * - ``NOTHING``
     - ``0x00``
     - ``1``
     - ``none``
     - 没有更多值。带有该标签的值必须是最后一个。

   * - ``INT_VALUE``
     - ``0x01``
     - ``0-1``
     - ``sleb128``
     - 字段的整型值。当字段类型为 ``boolean``、``byte``、``char``、``short`` 或 ``int`` 时使用该标签。


   * - ``VALUE``
     - ``0x02``
     - ``0-1``
     - ``uint8_t[4]``
     - 包含一个 :ref:`Value <RT Value>`。其编码依赖于 ``type_idx``。

   * - ``RUNTIME_ANNOTATIONS``
     - ``0x03``
     - ``>=0``
     - ``uint8_t[4]``
     - 指向在运行时**可见**的字段注解的偏移。
       如果一个字段具有多个注解，则该标签可以重复。
       该偏移必须指向 :ref:`Annotation <RT Annotation>`。

   * - ``ANNOTATIONS``
     - ``0x04``
     - ``>=0``
     - ``uint8_t[4]``
     - 指向在运行时**不可见**的字段注解的偏移。
       如果一个字段具有多个注解，则该标签可以重复。
       该偏移必须指向 :ref:`Annotation <RT Annotation>`。

   * - ``RUNTIME_TYPE_ANNOTATION``
     - ``0x05``
     - ``>=0``
     - ``uint8_t[4]``
     - 指向在运行时**可见**的字段类型注解的偏移。
       如果一个字段具有多个注解，则该标签可以重复。
       该偏移必须指向 :ref:`Annotation <RT Annotation>`。

   * - ``TYPE_ANNOTATION``
     - ``0x06``
     - ``>=0``
     - ``uint8_t[4]``
     - 指向在运行时**不可见**的字段类型注解的偏移。
       如果一个字段具有多个注解，则该标签可以重复。
       该偏移必须指向 :ref:`Annotation <RT Annotation>`。

**Constraint**: ``INT_VALUE`` 和 ``VALUE`` 标签互斥。

.. _RT ForeignMethod:

ForeignMethod 格式
--------------------------------------------------------------------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 15 10 75
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``class_idx``
     - ``uint16_t``
     - 在对应 :ref:`ClassRegionIndex <RT ClassRegionIndex>` 中声明类的索引。
       相应索引项必须是指向 :ref:`Class <RT Class>` 或 :ref:`ForeignClass <RT ForeignClass>` 的偏移。

   * - ``proto_idx``
     - ``uint16_t``
     - 在对应 :ref:`ProtoRegionIndex <RT ProtoRegionIndex>` 中方法原型的索引。
       相应索引项必须是指向 :ref:`Proto <RT Proto>` 的偏移。

   * - ``name_off``
     - ``uint32_t``
     - 方法名偏移。该偏移必须指向 :ref:`String <RT String>`。

   * - ``access_flags``
     - ``uleb128``
     - 方法访问标志。该值必须是 :ref:`Method Access Flags <RT Method Access Flags>` 的组合。

.. note::
   用于解析 ``class_idx`` 与 ``proto_idx`` 的正确 region index 可以通过 foreign method 的偏移找到。

.. _RT Method:

Method 格式
--------------------------------------------------------------------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 15 20 65
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``class_idx``
     - ``uint16_t``
     - 在对应 :ref:`ClassRegionIndex <RT ClassRegionIndex>` 中声明类的索引。
       相应索引项必须是指向 :ref:`Class <RT Class>` 或 :ref:`ForeignClass <RT ForeignClass>` 的偏移。

   * - ``proto_idx``
     - ``uint16_t``
     - 在对应 :ref:`ProtoRegionIndex <RT ProtoRegionIndex>` 中方法原型的索引。
       相应索引项必须是指向 :ref:`Proto <RT Proto>` 的偏移。

   * - ``name_off``
     - ``uint32_t``
     - 方法名偏移。该偏移必须指向 :ref:`String <RT String>`。

   * - ``access_flags``
     - ``uleb128``
     - 方法访问标志。该值必须是 :ref:`Method Access Flags <RT Method Access Flags>` 的组合。

   * - ``method_data``
     - ``TaggedValue[]``
     - 变长 tagged value 列表。每个元素都必须符合 :ref:`TaggedValue <RT TaggedValue>` 格式。
       标签值必须来自 :ref:`Method Tags <RT Method Tags>`，并按标签升序排列
       （``0x00`` 标签除外，它必须位于最后）。

.. note::
   用于解析 ``class_idx`` 与 ``proto_idx`` 的正确 region index 可以通过 foreign method 的偏移找到。

.. _RT Proto:

Proto 格式
--------------------------------------------------------------------------------

**Alignment**: 2 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 20 15 65
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``shorty``
     - ``uint16_t[]``
     - 原型的简短表示。其语法见 :ref:`Shorty <RT Shorty>`。

   * - ``reference_types``
     - ``uint16_t[]``
     - 方法签名中非基本类型的索引数组。
       ``shorty`` 中的每个 ``ref`` 类型都在该数组中有一个对应元素。
       该数组大小等于 ``shorty`` 中 ``ref`` 类型的数量。

.. note::
   用于解析引用类型索引的正确 region index 可以通过 proto 偏移找到。

.. _RT Shorty:

Shorty 语法
--------------------------------------------------------------------------------

``Shorty`` 是对方法签名的简短描述，不包含引用类型的详细信息。
一个 ``shorty`` 以返回类型开始，接着是方法参数，并以 ``0x0`` 结束。

``shorty`` 语法如下：

.. code-block:: abnf

    Shorty:
        ReturnType ParamTypeList 0x0
        ;
    ReturnType:
        Type
        ;
    ParamTypeList:
        (Type)*
        ;

``Type`` 的编码：

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Type
     - Encoding

   * - ``void``
     - ``0x01``

   * - ``u1``
     - ``0x02``

   * - ``i8``
     - ``0x03``

   * - ``u8``
     - ``0x04``

   * - ``i16``
     - ``0x05``

   * - ``u16``
     - ``0x06``

   * - ``i32``
     - ``0x07``

   * - ``u32``
     - ``0x08``

   * - ``f32``
     - ``0x09``

   * - ``f64``
     - ``0x0a``

   * - ``i64``
     - ``0x0b``

   * - ``u64``
     - ``0x0c``

   * - ``any``
     - ``0x0d``

   * - ``ref``
     - ``0x0e``

所有 ``Shorty`` 元素从起始位置开始，每四个为一组。
每组编码为一个 ``uint16_t``，而每个元素编码为 4 位。
四个元素 ``v1``、``v2``、``v3`` 和 ``v4`` 编码到 ``uint16_t`` 中的方式如下：

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Values
     - Bits

   * - ``v1``
     - ``0-3``

   * - ``v2``
     - ``4-7``

   * - ``v3``
     - ``8-11``

   * - ``v4``
     - ``12-15``

如果某组不足 4 个元素，则剩余位补为 ``0x0``。

.. _RT Method Access Flags:

方法访问标志
--------------------------------------------------------------------------------

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``ACC_PUBLIC``
     - ``0x0001``
     - 声明为 public。

   * - ``ACC_PRIVATE``
     - ``0x0002``
     - 声明为 private。

   * - ``ACC_PROTECTED``
     - ``0x0004``
     - 声明为 protected。

   * - ``ACC_STATIC``
     - ``0x0008``
     - 声明为 static。

   * - ``ACC_FINAL``
     - ``0x0010``
     - 声明为 final。

   * - ``ACC_SYNCHRONIZED``
     - ``0x0020``
     - 声明为 synchronized。

   * - ``ACC_BRIDGE``
     - ``0x0040``
     - 桥接方法，由编译器生成。

   * - ``ACC_VARARGS``
     - ``0x0080``
     - 声明为可变参数。

   * - ``ACC_NATIVE``
     - ``0x0100``
     - 声明为 native。

   * - ``ACC_ABSTRACT``
     - ``0x0400``
     - 声明为 abstract。

   * - ``ACC_STRICT``
     - ``0x0800``
     - 声明为 strictfp。

   * - ``ACC_SYNTHETIC``
     - ``0x1000``
     - 声明为 synthetic；不出现在源代码中。

**Constraint**: 对于 :ref:`ForeignMethod <RT ForeignMethod>`，仅使用 ``ACC_STATIC`` 标志。
其他标志会被忽略。

.. _RT Method Tags:

方法标签
--------------------------------------------------------------------------------

.. list-table::
   :width: 100%
   :align: left
   :widths: 30 10 10 15 35
   :header-rows: 1

   * - Name
     - Tag
     - Quantity
     - Data Format
     - Description

   * - ``NOTHING``
     - ``0x00``
     - ``1``
     - ``none``
     - 没有更多值。带有该标签的值必须是最后一个。

   * - ``CODE``
     - ``0x01``
     - ``0-1``
     - ``uint8_t[4]``
     - 指向方法代码的偏移。该偏移必须指向 :ref:`Code <RT Code>`。

   * - ``SOURCE_LANG``
     - ``0x02``
     - ``0-1``
     - ``uint8_t``
     - 方法的 :ref:`Source Language <RT Source Language>`。

   * - ``RUNTIME_ANNOTATION``
     - ``0x03``
     - ``>=0``
     - ``uint8_t[4]``
     - 指向在运行时**可见**的方法注解的偏移。
       如果方法具有多个注解，则该标签可以重复。
       该偏移必须指向 :ref:`Annotation <RT Annotation>`。

   * - ``RUNTIME_PARAM_ANNOTATION``
     - ``0x04``
     - ``0-1``
     - ``uint8_t[4]``
     - 指向在运行时**可见**的方法参数注解的偏移。
       该偏移必须指向 :ref:`ParamAnnotations <RT ParamAnnotations>`。

   * - ``DEBUG_INFO``
     - ``0x05``
     - ``0-1``
     - ``uint8_t[4]``
     - 指向与方法相关的调试信息的偏移。
       该偏移必须指向 :ref:`InstDebugInfo <RT InstDebugInfo>`。

   * - ``ANNOTATION``
     - ``0x06``
     - ``>=0``
     - ``uint8_t[4]``
     - 指向在运行时**不可见**的方法注解的偏移。
       如果方法具有多个注解，则该标签可以重复。
       该偏移必须指向 :ref:`Annotation <RT Annotation>`。

   * - ``PARAM_ANNOTATION``
     - ``0x07``
     - ``0-1``
     - ``uint8_t[4]``
     - 指向在运行时**不可见**的方法参数注解的偏移。
       该偏移必须指向 :ref:`ParamAnnotations <RT ParamAnnotations>`。

   * - ``TYPE_ANNOTATION``
     - ``0x08``
     - ``>=0``
     - ``uint8_t[4]``
     - 指向在运行时**不可见**的方法类型注解的偏移。
       如果方法具有多个注解，则该标签可以重复。
       该偏移必须指向 :ref:`Annotation <RT Annotation>`。

   * - ``RUNTIME_TYPE_ANNOTATION``
     - ``0x09``
     - ``>=0``
     - ``uint8_t[4]``
     - 指向在运行时**可见**的方法注解的偏移。
       如果方法具有多个注解，则该标签可以重复。
       该偏移必须指向 :ref:`Annotation <RT Annotation>`。

   * - ``PROFILE_INFO``
     - ``0x0a``
     - ``>=0``
     - ``uint8_t[]``
     - Profile 信息。其格式未指定，由文件所使用的语言决定。

.. _RT Code:

Code 格式
--------------------------------------------------------------------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``num_vregs``
     - ``uleb128``
     - 寄存器数量（不含参数寄存器）。

   * - ``num_args``
     - ``uleb128``
     - 参数数量。

   * - ``code_size``
     - ``uleb128``
     - 指令的字节大小。

   * - ``tries_size``
     - ``uleb128``
     - try 块数量。

   * - ``instructions``
     - ``uint8_t[]``
     - 指令。

   * - ``try_blocks``
     - ``TryBlock[]``
     - try 块数组。该数组有 ``tries_size`` 个元素。
       每个元素都必须符合 :ref:`TryBlock <RT TryBlock>` 格式。

.. _RT TryBlock:

TryBlock 格式
--------------------------------------------------------------------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 15 15 70
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``start_pc``
     - ``uleb128``
     - ``TryBlock`` 的起始 ``pc``。该 ``pc`` 指向被该 ``TryBlock`` 覆盖的第一条指令。

   * - ``length``
     - ``uleb128``
     - 被 ``TryBlock`` 覆盖的指令数量。

   * - ``num_catches``
     - ``uleb128``
     - 与该 ``TryBlock`` 关联的 catch 块数量。

   * - ``catch_blocks``
     - ``CatchBlock[]``
     - 与该 ``TryBlock`` 关联的 :ref:`CatchBlock <RT CatchBlock>` 数组。
       该数组有 ``num_catches`` 个元素，且每个元素都符合 :ref:`CatchBlock <RT CatchBlock>` 格式。
       catch 块按运行时应检查其异常类型的顺序排列。
       如果存在 *catch all* 块，则它必须位于最后。

.. _RT CatchBlock:

CatchBlock 格式
--------------------------------------------------------------------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 15 10 75
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``type_idx``
     - ``uleb128``
     - 由某个 :ref:`CatchBlock <RT CatchBlock>` 处理的异常类型的索引 + 1；
       若为 *catch all* 块，则该值为 ``0``。
       相应索引项必须是指向 :ref:`ForeignClass <RT ForeignClass>` 或 :ref:`Class <RT Class>` 的偏移。
       如果索引为 0，则表示它是一个捕获所有异常的 *catch all* 块。

   * - ``handler_pc``
     - ``uleb128``
     - 异常处理器第一条指令的 ``pc``。

   * - ``code_size``
     - ``uleb128``
     - 处理器代码的字节大小。

.. note::
   用于解析 ``type_idx`` 的正确 region index 可以通过对应方法的偏移找到。

.. _RT ParamAnnotations:

ParamAnnotations 格式
--------------------------------------------------------------------------------

**Alignment**: 8 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 15 20 65
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``count``
     - ``uint32_t``
     - 方法参数数量。该数量包括 synthetic 参数和 mandated 参数。

   * - ``annotations``
     - ``AnnotationArray[]``
     - 每个参数对应的注解列表数组。该数组有 ``count`` 个元素。
       每个元素都必须符合 :ref:`AnnotationArray <RT AnnotationArray>` 格式。

.. _RT AnnotationArray:

AnnotationArray 格式
--------------------------------------------------------------------------------

**Alignment**: 8 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 10 15 75
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``count``
     - ``uint32_t``
     - 数组中的元素数量。

   * - ``offsets``
     - ``uint32_t[]``
     - 指向参数注解的偏移数组。每个偏移都必须引用 :ref:`Annotation <RT Annotation>`。
       该数组有 ``count`` 个元素。

.. _RT ForeignClass:

ForeignClass 格式
--------------------------------------------------------------------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``name``
     - ``String``
     - foreign 类型的名称。该名称必须符合 :ref:`Type Descriptor <RT Type Descriptor>` 语法。

.. _RT Class:

Class 格式
--------------------------------------------------------------------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 20 20 60
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``name``
     - ``String``
     - 类型名称。该名称必须符合 :ref:`Type Descriptor <RT Type Descriptor>` 语法。

   * - ``super_class_off``
     - ``uint8_t[4]``
     - 指向超类名称的偏移；对于根对象类则为 ``0``。
       非零偏移必须指向 :ref:`ForeignClass <RT ForeignClass>` 或 :ref:`Class <RT Class>`。

   * - ``access_flags``
     - ``uleb128``
     - 类访问标志。该值必须是 :ref:`Class Access Flags <RT Class Access Flags>` 的组合。

   * - ``num_fields``
     - ``uleb128``
     - 该类拥有的字段数量。

   * - ``num_methods``
     - ``uleb128``
     - 该类拥有的方法数量。

   * - ``class_data``
     - ``TaggedValue[]``
     - 变长 tagged value 列表。每个元素都必须符合 :ref:`TaggedValue <RT TaggedValue>` 格式。
       标签值必须来自 :ref:`Class Tags <RT Class Tags>`，并按标签升序排列
       （``0x00`` 标签除外，它必须位于最后）。

   * - ``fields``
     - ``Field[]``
     - 类字段。元素数量为 ``num_fields``。每个元素都必须符合 :ref:`Field <RT Field>` 格式。

   * - ``methods``
     - ``Method[]``
     - 类方法。元素数量为 ``num_methods``。每个元素都必须符合 :ref:`Method <RT Method>` 格式。

.. _RT Type Descriptor:

类型描述符
--------------------------------------------------------------------------------

二进制文件中的类型描述符语法如下：

.. code-block:: abnf

    TypeDescriptor:
        PrimitiveType
        | ArrayType
        | RefType
        | UnionType
        ;
    PrimitiveType:
        'Z'
        | 'B'
        | 'H'
        | 'S'
        | 'C'
        | 'I'
        | 'U'
        | 'F'
        | 'D'
        | 'J'
        | 'Q'
        | 'A'
        ;
    ArrayType:
        '[' TypeDescriptor
        ;
    RefType:
        'L' RefTypeName ';'
        ;
    UnionType:
        '{U' TypeDescriptor TypeDescriptor (TypeDescriptor)* '}'
        ;

``RefTypeName`` 是该类型的 :ref:`全限定名 <RT Fully Qualified Name>`，并将其中所有点号 ``.`` 替换为斜杠 ``/``。

``PrimitiveType`` 的编码：

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Type
     - Encoding
     - Description

   * - ``u1``
     - ``Z``
     - 1 位无符号整数

   * - ``i8``
     - ``B``
     - 8 位有符号整数

   * - ``u8``
     - ``H``
     - 8 位无符号整数

   * - ``i16``
     - ``S``
     - 16 位有符号整数

   * - ``u16``
     - ``C``
     - 16 位无符号整数

   * - ``i32``
     - ``I``
     - 32 位有符号整数

   * - ``u32``
     - ``U``
     - 32 位无符号整数

   * - ``f32``
     - ``F``
     - 单精度浮点数

   * - ``f64``
     - ``D``
     - 双精度浮点数

   * - ``i64``
     - ``J``
     - 64 位有符号整数

   * - ``u64``
     - ``Q``
     - 64 位无符号整数

   * - ``any``
     - ``A``
     - any

**Constraint**: 二进制文件中的 ``UnionType`` 必须被规范化，以确保不存在等价的联合类型。
规范化要求对所有 *组成类型* 的 ``TypeDescriptor`` 按字母顺序排序。

.. _RT Class Access Flags:

类访问标志
--------------------------------------------------------------------------------

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``ACC_PUBLIC``
     - ``0x0001``
     - 声明为 public。

   * - ``ACC_FINAL``
     - ``0x0010``
     - 声明为 final。

   * - ``ACC_SUPER``
     - ``0x0020``
     - 没有特殊含义，仅为兼容性而加入。

   * - ``ACC_INTERFACE``
     - ``0x0200``
     - 是一个接口。

   * - ``ACC_ABSTRACT``
     - ``0x0400``
     - 声明为 abstract。

   * - ``ACC_SYNTHETIC``
     - ``0x1000``
     - 声明为 synthetic；不出现在源代码中。

   * - ``ACC_ANNOTATION``
     - ``0x2000``
     - 声明为注解类型。

   * - ``ACC_ENUM``
     - ``0x4000``
     - 声明为枚举类型。

.. _RT Class Tags:

类标签
--------------------------------------------------------------------------------

.. list-table::
   :width: 100%
   :align: left
   :widths: 30 10 10 15 35
   :header-rows: 1

   * - Name
     - Tag
     - Quantity
     - Data Format
     - Description

   * - ``NOTHING``
     - ``0x00``
     - ``1``
     - ``none``
     - 没有更多值。带有该标签的值必须位于最后。

   * - ``INTERFACES``
     - ``0x01``
     - ``0-1``
     - ``uleb128 uint8_t[]``
     - 某个类实现的接口列表。
       数据中先给出以 ``uleb128`` 编码的接口数量，随后是 :ref:`ClassRegionIndex <RT ClassRegionIndex>` 格式中的接口索引。
       每个索引长度为 2 字节，解析后必须指向 :ref:`ForeignClass <RT ForeignClass>` 或 :ref:`Class <RT Class>`。
       索引数量等于接口数量。

   * - ``SOURCE_LANG``
     - ``0x02``
     - ``0-1``
     - ``uint8_t``
     - 数据表示 :ref:`Source Language <RT Source Language>`。

   * - ``RUNTIME_ANNOTATION``
     - ``0x03``
     - ``>=0``
     - ``uint8_t[4]``
     - 指向在运行时**可见**的类注解的偏移。
       如果一个类具有多个注解，则该标签可以重复。
       该偏移必须指向 :ref:`Annotation <RT Annotation>`。

   * - ``ANNOTATION``
     - ``0x04``
     - ``>=0``
     - ``uint8_t[4]``
     - 指向在运行时**不可见**的类注解的偏移。
       如果一个类具有多个注解，则该标签可以重复。
       该偏移必须指向 :ref:`Annotation <RT Annotation>`。

   * - ``RUNTIME_TYPE_ANNOTATION``
     - ``0x05``
     - ``>=0``
     - ``uint8_t[4]``
     - 指向在运行时**可见**的类类型注解的偏移。
       如果一个类具有多个注解，则该标签可以重复。
       该偏移必须指向 :ref:`Annotation <RT Annotation>`。

   * - ``TYPE_ANNOTATION``
     - ``0x06``
     - ``>=0``
     - ``uint8_t[4]``
     - 指向在运行时**不可见**的类类型注解的偏移。
       如果一个类具有多个注解，则该标签可以重复。
       该偏移必须指向 :ref:`Annotation <RT Annotation>`。

   * - ``SOURCE_FILE``
     - ``0x07``
     - ``0-1``
     - ``uint8_t[4]``
     - 指向包含类源码的文件名字符串的偏移。

.. note::
   用于解析接口索引的正确 region index 可以通过类偏移找到。

.. _RT Annotation:

Annotation 格式
--------------------------------------------------------------------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 15 25 60
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``class_idx``
     - ``uint16_t``
     - 在 :ref:`ClassRegionIndex <RT ClassRegionIndex>` 中声明类的索引。
       相应索引项必须是指向 :ref:`Class <RT Class>` 或 :ref:`ForeignClass <RT ForeignClass>` 的偏移。

   * - ``count``
     - ``uint16_t``
     - 注解中 name-value 对的数量（即 ``elements`` 数组中的元素数量）。

   * - ``elements``
     - ``AnnotationElement[]``
     - 注解元素数组。每个元素都必须符合 :ref:`AnnotationElement <RT AnnotationElement>` 格式。
       元素顺序必须与注解类中的顺序一致。

   * - ``element_types``
     - ``uint8_t[]``
     - 注解元素类型数组。``element_types`` 中的每个元素都描述 ``elements`` 中对应
       :ref:`AnnotationElement <RT AnnotationElement>` 的类型。
       该数组中的元素顺序与 ``elements`` 字段顺序一致。
       每个元素的值都必须取自 :ref:`Annotation Element Type <RT Annotation Element Type>`。

.. note::
   用于解析 ``class_idx`` 的正确 region index 可以通过 annotation 偏移找到。

.. _RT Annotation Element Type:

注解元素类型
--------------------------------------------------------------------------------

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Type
     - Tag

   * - ``u1``
     - ``1``

   * - ``i8``
     - ``2``

   * - ``u8``
     - ``3``

   * - ``i16``
     - ``4``

   * - ``u16``
     - ``5``

   * - ``i32``
     - ``6``

   * - ``u32``
     - ``7``

   * - ``i64``
     - ``8``

   * - ``u64``
     - ``9``

   * - ``f32``
     - ``A``

   * - ``f64``
     - ``B``

   * - ``string``
     - ``C``

   * - ``record``
     - ``D``

   * - ``method``
     - ``E``

   * - ``enum``
     - ``F``

   * - ``annotation``
     - ``G``

   * - ``method_handle``
     - ``J``

   * - ``array``
     - ``H``

   * - ``u1[]``
     - ``K``

   * - ``i8[]``
     - ``L``

   * - ``u8[]``
     - ``M``

   * - ``i16[]``
     - ``N``

   * - ``u16[]``
     - ``O``

   * - ``i32[]``
     - ``P``

   * - ``u32[]``
     - ``Q``

   * - ``i64[]``
     - ``R``

   * - ``u64[]``
     - ``S``

   * - ``f32[]``
     - ``T``

   * - ``f64[]``
     - ``U``

   * - ``string[]``
     - ``V``

   * - ``record[]``
     - ``W``

   * - ``method[]``
     - ``X``

   * - ``enum[]``
     - ``Y``

   * - ``annotation[]``
     - ``Z``

   * - ``method_handle[]``
     - ``@``

   * - ``nullptr_string``
     - ``*``

.. note::
   带有 ``nullptr_string`` 标签的元素，其正确值应为 ``0``（``\x00\x00\x00\x00``）。

.. _RT AnnotationElement:

AnnotationElement 格式
--------------------------------------------------------------------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 15 15 70
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``name_off``
     - ``uint32_t``
     - 指向元素名的偏移。该偏移必须指向 :ref:`String <RT String>`。

   * - ``value``
     - ``uint32_t``
     - 元素值。如果注解元素的类型是 ``boolean``、``byte``、``short``、``char``、``int`` 或 ``float``，
       那么 ``value`` 字段直接以相应 :ref:`Value <RT Value>` 格式存储该值本身。
       否则，该字段包含一个指向 :ref:`Value <RT Value>` 的偏移。
       该 :ref:`Value <RT Value>` 的格式可以根据 :ref:`AnnotationElement <RT AnnotationElement>` 的类型确定。

.. _RT Value:

Value 格式
--------------------------------------------------------------------------------

根据值类型不同，存在不同的值编码方式。

.. _RT ByteValue:

ByteValue 格式
--------------------------------------------------------------------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t``
     - 1 字节有符号整数值。

.. _RT ShortValue:

ShortValue 格式
--------------------------------------------------------------------------------

**Alignment**: 2 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t[2]``
     - 2 字节有符号整数值。

.. _RT IntegerValue:

IntegerValue 格式
--------------------------------------------------------------------------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t[4]``
     - 4 字节有符号整数值。

.. _RT LongValue:

LongValue 格式
--------------------------------------------------------------------------------

**Alignment**: 8 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t[8]``
     - 8 字节有符号整数值。

.. _RT FloatValue:

FloatValue 格式
--------------------------------------------------------------------------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t[4]``
     - 4 字节位模式，右侧零扩展后解释为 IEEE754 32 位浮点值。

.. _RT DoubleValue:

DoubleValue 格式
--------------------------------------------------------------------------------

**Alignment**: 8 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t[8]``
     - 8 字节位模式，右侧零扩展后解释为 IEEE754 64 位浮点值。

.. _RT StringValue:

StringValue 格式
--------------------------------------------------------------------------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint32_t``
     - 该值表示一个指向 :ref:`String <RT String>` 的偏移。

.. _RT EnumValue:

EnumValue 格式
--------------------------------------------------------------------------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint32_t``
     - 该值表示一个指向枚举字段的偏移。该偏移必须指向 :ref:`Field <RT Field>` 或 :ref:`ForeignField <RT ForeignField>`。

.. _RT ClassValue:

ClassValue 格式
--------------------------------------------------------------------------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint32_t``
     - 该值表示一个指向 :ref:`Class <RT Class>` 或 :ref:`ForeignClass <RT ForeignClass>` 的偏移。

.. _RT AnnotationValue:

AnnotationValue 格式
--------------------------------------------------------------------------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint32_t``
     - 该值表示一个指向 :ref:`Annotation <RT Annotation>` 的偏移。

.. _RT MethodValue:

MethodValue 格式
--------------------------------------------------------------------------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint32_t``
     - 该值表示一个指向 :ref:`Method <RT Method>` 或 :ref:`ForeignMethod <RT ForeignMethod>` 的偏移。

.. _RT MethodHandleValue:

MethodHandleValue 格式
--------------------------------------------------------------------------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint32_t``
     - 该值表示一个指向 :ref:`MethodHandle <RT MethodHandle>` 的偏移。

.. _RT MethodTypeValue:

MethodTypeValue 格式
--------------------------------------------------------------------------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint32_t``
     - 该值表示一个指向 :ref:`Proto <RT Proto>` 的偏移。

.. _RT ArrayValue:

ArrayValue 格式
--------------------------------------------------------------------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``count``
     - ``uleb128``
     - 数组中的元素数量。

   * - ``value``
     - ``uint32_t``
     - 未对齐的 :ref:`Value <RT Value>` 实体数组。该数组有 ``count`` 个元素。
       其格式取决于 ``tag_value`` 字段。

.. _RT LiteralArray:

LiteralArray 格式
--------------------------------------------------------------------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 20 15 65
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``num_literals``
     - ``uint32_t``
     - 针对不同 :ref:`Literal Tags <RT Literal Tags>` 有不同解释。
       如果 ``literals`` 带有 ``ARRAY_*`` 标签，则 ``num_literals`` 等于数组中的元素数量。
       对于其他标签，``num_literals`` 等于 ``literals`` 数组中的元素数量乘以 2
       （因为每个 ``tag`` 都可被解释为单独的 :ref:`Literal <RT Literal>`）。

   * - ``literals``
     - ``Literal[]``
     - literal array 的元素。元素数量为 ``num_literals``。
       每个元素都必须符合 :ref:`Literal <RT Literal>` 格式。

.. _RT Literal Tags:

Literal 标签
--------------------------------------------------------------------------------

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Tag
     - Value
     - ``data`` encoding

   * - ``TAGVALUE``
     - ``0x00``
     - :ref:`ByteOne <RT ByteOne>`

   * - ``BOOL``
     - ``0x01``
     - :ref:`ByteOne <RT ByteOne>`

   * - ``INTEGER``
     - ``0x02``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``FLOAT``
     - ``0x03``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``DOUBLE``
     - ``0x04``
     - :ref:`ByteEight <RT ByteEight>`

   * - ``STRING``
     - ``0x05``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``BIGINT``
     - ``0x06``
     - :ref:`ByteEight <RT ByteEight>`

   * - ``METHOD``
     - ``0x07``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``GENERATORMETHOD``
     - ``0x08``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``ACCESSOR``
     - ``0x09``
     - :ref:`ByteOne <RT ByteOne>`

   * - ``METHODAFFILIATE``
     - ``0x0a``
     - :ref:`ByteTwo <RT ByteTwo>`

   * - ``ARRAY_U1``
     - ``0x0b``
     - :ref:`ByteOne <RT ByteOne>`

   * - ``ARRAY_U8``
     - ``0x0c``
     - :ref:`ByteOne <RT ByteOne>`

   * - ``ARRAY_I8``
     - ``0x0d``
     - :ref:`ByteOne <RT ByteOne>`

   * - ``ARRAY_U16``
     - ``0x0e``
     - :ref:`ByteTwo <RT ByteTwo>`

   * - ``ARRAY_I16``
     - ``0x0f``
     - :ref:`ByteTwo <RT ByteTwo>`

   * - ``ARRAY_U32``
     - ``0x10``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``ARRAY_I32``
     - ``0x11``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``ARRAY_U64``
     - ``0x12``
     - :ref:`ByteEight <RT ByteEight>`

   * - ``ARRAY_I64``
     - ``0x13``
     - :ref:`ByteEight <RT ByteEight>`

   * - ``ARRAY_F32``
     - ``0x14``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``ARRAY_F64``
     - ``0x15``
     - :ref:`ByteEight <RT ByteEight>`

   * - ``ARRAY_STRING``
     - ``0x16``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``ASYNCGENERATORMETHOD``
     - ``0x17``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``ASYNCMETHOD``
     - ``0x18``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``LITERALARRAY``
     - ``0x19``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``NULLVALUE``
     - ``0xff``
     - :ref:`ByteOne <RT ByteOne>`

.. _RT Literal:

Literal 格式
--------------------------------------------------------------------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 10 15 75
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``tag``
     - ``uint8_t``
     - literal 的标签。literal 的标签指示如何解释 ``num_literals`` 与 ``data`` 字段。
       literal 的标签必须是 :ref:`Literal Tags <RT Literal Tags>` 中的一种。

   * - ``data``
     - ``uint8_t[]``
     - literal array 的元素。元素数量为 ``num_literals``。
       ``data`` 的具体编码取决于 ``tag`` 的值。

.. _RT ByteOne:

ByteOne 格式
--------------------------------------------------------------------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t``
     - 1 字节值。

.. _RT ByteTwo:

ByteTwo 格式
--------------------------------------------------------------------------------

**Alignment**: 2 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t[2]``
     - 2 字节值。

.. _RT ByteFour:

ByteFour 格式
--------------------------------------------------------------------------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t[4]``
     - 4 字节值。

.. _RT ByteEight:

ByteEight 格式
--------------------------------------------------------------------------------

**Alignment**: 8 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t[8]``
     - 8 字节值。

.. _RT InstDebugInfo:

InstDebugInfo 格式
--------------------------------------------------------------------------------

调试信息包含方法程序计数器、源码行号以及局部变量信息之间的映射。
其格式派生自 DWARF Debugging Information Format, Version 3（见小节 6.2）。
映射和局部变量信息都编码在行号程序中，由 :ref:`State Machine <RT State Machine>` 解释执行。
程序引用的所有常量都会被移动到 :ref:`Constant Pool <RT Constant Pool>` 中，
以便对不同方法中相同的行号程序去重。

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 30 15 55
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``line_start``
     - ``uleb128``
     - :ref:`State Machine <RT State Machine>` 的 line 寄存器初始值。

   * - ``num_parameters``
     - ``uleb128``
     - 方法参数数量。

   * - ``parameters``
     - ``uleb128[]``
     - 方法的参数名。数组有 ``num_parameters`` 个元素。
       每个元素都是指向 :ref:`String <RT String>` 的偏移；若无名称可用，则为 ``0``。

   * - ``constant_pool_size``
     - ``uleb128``
     - 常量池字节大小。

   * - ``constant_pool``
     - ``uleb128[]``
     - :ref:`Constant Pool <RT Constant Pool>` 数据。长度为 ``constant_pool_size`` 字节。

   * - ``line_number_program_idx``
     - ``uleb128``
     - :ref:`LineNumberProgramIndex <RT LineNumberProgramIndex>` 格式中的行号程序索引。
       程序长度可变，并以 ``DBG_END_SEQUENCE`` 操作码结束。

.. _RT Constant Pool:

常量池
--------------------------------------------------------------------------------

许多方法具有相似的行号程序，仅在变量名、变量类型和文件名方面有所不同。
为了对这类程序去重，程序引用的所有常量都存储在常量池中。
在解释程序时，:ref:`State Machine <RT State Machine>` 会跟踪一个指向常量池的指针。
当 :ref:`State Machine <RT State Machine>` 解释一条需要常量实参的指令时，
它会读取 constant pool 指针所指向的值，然后递增该指针。
因此，程序本身不包含对常量的显式引用，从而能够实现去重。

.. _RT State Machine:

状态机
--------------------------------------------------------------------------------

状态机的目标是生成程序计数器、行号和局部变量信息之间的映射。
该机器具有以下寄存器：

.. list-table::
   :width: 100%
   :align: left
   :widths: 22 20 58
   :header-rows: 1

   * - Name
     - Initial value
     - Description

   * - ``address``
     - 0
     - 程序计数器（指向方法指令）。
       其值必须始终单调递增。

   * - ``line``
     - :ref:`InstDebugInfo <RT InstDebugInfo>` 中的 ``line_start``。
     - 对应于源码中行号的无符号整数。
       所有行号从 1 开始，且该寄存器的值不得小于 1。

   * - ``file``
     - ``class_data`` 中 ``SOURCE_FILE`` 标签的值（见 :ref:`Class <RT Class>`），或者 ``0``。
     - 指向源文件名的偏移。
       如果没有此类信息（即 :ref:`Class <RT Class>` 中不存在 ``SOURCE_FILE`` 标签），
       则该寄存器值为 ``0``。

   * - ``prologue_end``
     - ``false``
     - 该寄存器表示当前地址是否可作为方法的入口断点位置。

   * - ``epilogue_begin``
     - ``false``
     - 该寄存器表示当前地址是否可作为方法的出口断点位置。

   * - ``constant_pool_ptr``
     - ``constant_pool`` 中第一个字节的地址，即 :ref:`InstDebugInfo <RT InstDebugInfo>` 内的位置。
     - 指向当前常量值的指针。

.. _RT Line Number Program:

行号程序格式
--------------------------------------------------------------------------------

一个行号程序由若干指令组成。每条指令都有一个单字节操作码和可选参数。
参数值有时可直接编码在指令中；否则，指令需要从常量池中读取该值。

.. list-table::
   :width: 100%
   :align: left
   :widths: 25 10 10 10 45
   :header-rows: 1

   * - Opcode
     - Value
     - Instruction Format
     - Constant Pool Args
     - Description

   * - ``END_SEQUENCE``
     - ``0x00``
     -
     -
     - 标记行号程序结束。

   * - ``ADVANCE_PC``
     - ``0x01``
     -
     - ``uleb128``
     - 以 ``constant_pool_ptr`` 所指的值递增 ``address`` 寄存器，但不发出行号。

   * - ``ADVANCE_LINE``
     - ``0x02``
     -
     - ``sleb128``
     - 以 ``constant_pool_ptr`` 所指的值递增 ``line`` 寄存器，但不发出行号。

   * - ``START_LOCAL``
     - ``0x03``
     - ``sleb128``
     - ``uleb128 uleb128``
     - 在当前地址引入一个局部变量，其名称与类型由 ``constant_pool_ptr`` 所指。
       保存该变量的寄存器编号编码在指令中。
       寄存器值 ``-1`` 表示累加器寄存器。
       名称是指向 :ref:`String <RT String>` 的偏移，
       类型是指向 :ref:`ForeignClass <RT ForeignClass>` 或 :ref:`Class <RT Class>` 的偏移。
       若相应信息缺失，则偏移为 ``0``。

   * - ``START_LOCAL_EXTENDED``
     - ``0x04``
     - ``sleb128``
     - ``uleb128 uleb128 uleb128``
     - 在当前地址引入一个局部变量，其名称、类型和类型签名由 ``constant_pool_ptr`` 所指。
       保存该变量的寄存器编号编码在指令中。
       寄存器值 ``-1`` 表示累加器寄存器。
       名称是指向 :ref:`String <RT String>` 的偏移，
       类型是指向 :ref:`ForeignClass <RT ForeignClass>` 或 :ref:`Class <RT Class>` 的偏移。
       若相应信息缺失，则偏移为 ``0``。

   * - ``END_LOCAL``
     - ``0x05``
     - ``sleb128``
     -
     - 标记指定寄存器中的局部变量超出作用域。
       寄存器编号编码在指令中。寄存器值 ``-1`` 表示累加器寄存器。

   * - ``RESTART_LOCAL``
     - ``0x06``
     - ``sleb128``
     -
     - 在指定寄存器中重新引入一个局部变量。
       其名称和类型与该寄存器中上一次出现的局部变量相同。
       寄存器编号编码在指令中。寄存器值 ``-1`` 表示累加器寄存器。

   * - ``SET_PROLOGUE_END``
     - ``0x07``
     -
     -
     - 将 ``prologue_end`` 寄存器设为 ``true``。
       任何 special opcode 都会清除 ``prologue_end`` 寄存器。

   * - ``SET_EPILOGUE_BEGIN``
     - ``0x08``
     -
     -
     - 将 ``epilogue_end`` 寄存器设为 ``true``。
       任何 special opcode 都会清除 ``epilogue_end`` 寄存器。

   * - ``SET_FILE``
     - ``0x09``
     -
     - ``uleb128``
     - 将 ``file`` 寄存器设置为 ``constant_pool_ptr`` 所指的值。
       该参数是一个指向 :ref:`String <RT String>` 的偏移，用来表示文件名；若无则为 ``0``。

   * - ``SET_SOURCE_CODE``
     - ``0x0a``
     -
     - ``uleb128``
     - 将 ``source_code`` 寄存器设置为 ``constant_pool_ptr`` 所指的值。
       该参数是一个指向 :ref:`String <RT String>` 的偏移，用来表示源代码；若无则为 ``0``。

   * - ``SET_COLUMN``
     - ``0x0b``
     -
     - ``uleb128``
     - 以 ``constant_pool_ptr`` 所指的值设置 ``column`` 寄存器。

   * - Special opcodes
     - ``0x0c .. 0xff``
     -
     -
     -

Special opcode 的解释：

:ref:`State Machine <RT State Machine>` 对每个 special opcode 的解释如下
（见 DWARF Debugging Information Format 小节 6.2.5.1 Special Opcodes）：

1. 计算调整后的操作码：
   ``adjusted_opcode = opcode - OPCODE_BASE``。
2. 递增 ``address`` 寄存器：
   ``address += adjusted_opcode / LINE_RANGE``。
3. 递增 ``line`` 寄存器：
   ``line += LINE_BASE + (adjusted_opcode % LINE_RANGE)``。
4. 发出行号。
5. 将 ``prologue_end`` 寄存器设为 ``false``。
6. 将 ``epilogue_begin`` 寄存器设为 ``false``。

其中：

- ``OPCODE_BASE = 0x0c``：第一个 special opcode。
- ``LINE_BASE = -4``：最小行号增量。
- ``LINE_RANGE = 15``：可表示的行号增量个数。

.. _RT MethodHandle:

MethodHandle 格式
--------------------------------------------------------------------------------

Alignment: none

Format:

.. list-table::
   :width: 100%
   :align: left
   :widths: 20 20 60
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``type``
     - ``uint8_t``
     - handle 的类型。其值必须是 :ref:`Type Of MethodHandle <RT Type Of MethodHandle>` 中的一种。

   * - ``offset``
     - ``uleb128``
     - 指向相应类型实体的偏移。实体类型取决于 handle 类型
       （见 :ref:`Type of MethodHandle <RT Type of MethodHandle>`）。

.. _RT Type of MethodHandle:

MethodHandle 的类型
--------------------------------------------------------------------------------

方法句柄可用的类型如下：

.. list-table::
   :width: 100%
   :align: left
   :widths: 20 20 60
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``PUT_STATIC``
     - ``0x00``
     - 方法句柄引用一个静态 setter。:ref:`MethodHandle <RT MethodHandle>` 中的偏移必须指向
       :ref:`Field <RT Field>` 或 :ref:`ForeignField <RT ForeignField>`。

   * - ``GET_STATIC``
     - ``0x01``
     - 方法句柄引用一个静态 getter。:ref:`MethodHandle <RT MethodHandle>` 中的偏移必须指向
       :ref:`Field <RT Field>` 或 :ref:`ForeignField <RT ForeignField>`。

   * - ``PUT_INSTANCE``
     - ``0x02``
     - 方法句柄引用一个实例 getter。:ref:`MethodHandle <RT MethodHandle>` 中的偏移必须指向
       :ref:`Field <RT Field>` 或 :ref:`ForeignField <RT ForeignField>`。

   * - ``GET_INSTANCE``
     - ``0x03``
     - 方法句柄引用一个实例 setter。:ref:`MethodHandle <RT MethodHandle>` 中的偏移必须指向
       :ref:`Field <RT Field>` 或 :ref:`ForeignField <RT ForeignField>`。

   * - ``INVOKE_STATIC``
     - ``0x04``
     - 方法句柄引用一个静态方法。:ref:`MethodHandle <RT MethodHandle>` 中的偏移必须指向
       :ref:`Method <RT Method>` 或 :ref:`ForeignMethod <RT ForeignMethod>`。

   * - ``INVOKE_INSTANCE``
     - ``0x05``
     - 方法句柄引用一个实例方法。:ref:`MethodHandle <RT MethodHandle>` 中的偏移必须指向
       :ref:`Method <RT Method>` 或 :ref:`ForeignMethod <RT ForeignMethod>`。

   * - ``INVOKE_CONSTRUCTOR``
     - ``0x06``
     - 方法句柄引用一个构造器。:ref:`MethodHandle <RT MethodHandle>` 中的偏移必须指向
       :ref:`Method <RT Method>` 或 :ref:`ForeignMethod <RT ForeignMethod>`。

   * - ``INVOKE_DIRECT``
     - ``0x07``
     - 方法句柄引用一个直接方法。:ref:`MethodHandle <RT MethodHandle>` 中的偏移必须指向
       :ref:`Method <RT Method>` 或 :ref:`ForeignMethod <RT ForeignMethod>`。

   * - ``INVOKE_INTERFACE``
     - ``0x08``
     - 方法句柄引用一个接口方法。:ref:`MethodHandle <RT MethodHandle>` 中的偏移必须指向
       :ref:`Method <RT Method>` 或 :ref:`ForeignMethod <RT ForeignMethod>`。

.. _RT Argument Types:

参数类型
--------------------------------------------------------------------------------

bootstrap 方法可以接受以下类型的静态参数：

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``Integer``
     - ``0x00``
     - 对应实参采用 :ref:`IntegerValue <RT IntegerValue>` 编码。

   * - ``Long``
     - ``0x01``
     - 对应实参采用 :ref:`LongValue <RT LongValue>` 编码。

   * - ``Float``
     - ``0x02``
     - 对应实参采用 :ref:`FloatValue <RT FloatValue>` 编码。

   * - ``Double``
     - ``0x03``
     - 对应实参采用 :ref:`DoubleValue <RT DoubleValue>` 编码。

   * - ``String``
     - ``0x04``
     - 对应实参采用 :ref:`StringValue <RT StringValue>` 编码。

   * - ``Class``
     - ``0x05``
     - 对应实参采用 :ref:`ClassValue <RT ClassValue>` 编码。

   * - ``MethodHandle``
     - ``0x06``
     - 对应实参采用 :ref:`MethodHandleValue <RT MethodHandleValue>` 编码。

   * - ``MethodType``
     - ``0x07``
     - 对应实参采用 :ref:`MethodTypeValue <RT MethodTypeValue>` 编码。
