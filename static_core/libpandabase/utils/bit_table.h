/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_BITTABLE_H
#define PANDA_BITTABLE_H

#include "utils/arena_containers.h"
#include "utils/bit_memory_region.h"
#include "utils/bit_memory_stream.h"
#include "utils/bit_vector.h"
#include "utils/hash.h"
#include <iomanip>
#include "securec.h"

namespace ark {

class VarintPack {
public:
    static constexpr size_t INLINE_BITS = 4;
    static constexpr size_t INLINE_MAX = 11;

    template <size_t N>
    static std::array<uint32_t, N> Read(BitMemoryStreamIn *stream)
    {
        static_assert(sizeof(uint64_t) * BITS_PER_BYTE >= N * INLINE_BITS);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
        std::array<uint32_t, N> values;
        auto data = stream->Read<uint64_t>(N * INLINE_BITS);
        for (size_t i = 0; i < N; i++) {
            values[i] = ExtractBits(data, i * INLINE_BITS, INLINE_BITS);
        }
        for (size_t i = 0; i < N; i++) {
            if (UNLIKELY(values[i] > INLINE_MAX)) {
                values[i] = stream->Read<uint32_t>((values[i] - INLINE_MAX) * BITS_PER_BYTE);
            }
        }
        return values;
    }

    template <size_t N, typename Container>
    static void Write(BitMemoryStreamOut<Container> &stream, const std::array<uint32_t, N> &data)
    {
        for (auto value : data) {
            if (value > INLINE_MAX) {
                stream.Write(INLINE_MAX + BitsToBytesRoundUp(MinimumBitsToStore(value)), INLINE_BITS);
            } else {
                stream.Write(value, INLINE_BITS);
            }
        }
        for (auto value : data) {
            if (value > INLINE_MAX) {
                stream.Write(value, BitsToBytesRoundUp(MinimumBitsToStore(value)) * BITS_PER_BYTE);
            }
        }
    }
};

template <typename>
class BitTable;

template <size_t N_COLUMNS, typename Accessor, bool REVERSE_ITERATION = false>
class BitTableRow {
    using BitTableType = BitTable<Accessor>;

public:
    // NOLINTBEGIN(readability-identifier-naming)
    using iterator_category = std::random_access_iterator_tag;
    using value_type = Accessor;
    using difference_type = int32_t;
    using pointer = value_type *;
    using reference = value_type &;
    // NOLINTEND(readability-identifier-naming)

    static constexpr size_t NUM_COLUMNS = N_COLUMNS;
    static constexpr uint32_t NO_VALUE = BitTableType::NO_VALUE;

    using Reversed = BitTableRow<N_COLUMNS, Accessor, !REVERSE_ITERATION>;

    BitTableRow() = default;
    BitTableRow(const BitTableType *table, int rowIndex) : table_(table), rowIndex_(rowIndex) {}
    explicit BitTableRow(Reversed *rhs) : table_(rhs->table_), rowIndex_(rhs->row_index_) {}

    uint32_t GetRow() const
    {
        return rowIndex_;
    }

    std::string GetColumnStr(size_t column) const
    {
        if (Get(column) == NO_VALUE) {
            return "-";
        }
        return std::to_string(Get(column));
    }

    Accessor GetAccessor()
    {
        return Accessor(this);
    }

    uint32_t Get(size_t index) const
    {
        return table_->ReadColumn(rowIndex_, index);
    }

    bool Has(size_t index) const
    {
        return Get(index) != BitTableType::NO_VALUE;
    }

    size_t ColumnsCount() const
    {
        return table_->GetColumnsCount();
    }

    bool IsValid() const
    {
        return rowIndex_ != -1;
    }

    bool operator==(const BitTableRow &rhs) const
    {
        return table_ == rhs.table_ && rowIndex_ == rhs.rowIndex_;
    }

    bool operator!=(const BitTableRow &rhs) const
    {
        return !(operator==(rhs));
    }

    ~BitTableRow() = default;

    DEFAULT_COPY_SEMANTIC(BitTableRow);
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(BitTableRow);

protected:
    auto GetTable() const
    {
        return table_;
    }

private:
    const BitTableType *table_ {nullptr};
    int rowIndex_ {-1};

    friend Reversed;
    template <typename, bool>
    friend class BitTableIterator;
};

template <typename Accessor, bool REVERSE_ITERATION = false>
class BitTableIterator {
public:
    // NOLINTBEGIN(readability-identifier-naming)
    using iterator_category = std::random_access_iterator_tag;
    using value_type = Accessor;
    using difference_type = int32_t;
    using pointer = value_type *;
    using reference = value_type &;
    // NOLINTEND(readability-identifier-naming)

    BitTableIterator(const typename Accessor::BitTableType *table, int rowIndex) : row_(table, rowIndex) {}
    explicit BitTableIterator(Accessor &row) : row_(row) {}

    BitTableIterator &operator++()
    {
        if constexpr (REVERSE_ITERATION) {  // NOLINT
            row_.rowIndex_--;
        } else {  // NOLINT
            row_.rowIndex_++;
        }
        return *this;
    }

    BitTableIterator &operator--()
    {
        if constexpr (REVERSE_ITERATION) {  // NOLINT
            row_.row_index_++;
        } else {  // NOLINT
            row_.rowIndex_--;
        }
        return *this;
    }

    explicit operator bool() const
    {
        return row_.row_index_ >= 0 && helpers::ToUnsigned(row_.row_index_) < row_.table_->GetRowsCount();
    }

    bool operator==(const BitTableIterator &rhs) const
    {
        return row_ == rhs.row_;
    }

    bool operator!=(const BitTableIterator &rhs) const
    {
        return row_ != rhs.row_;
    }

    auto operator+(int32_t n) const
    {
        if constexpr (REVERSE_ITERATION) {  // NOLINT
            return BitTableIterator(row_.table_, row_.rowIndex_ - n);
        } else {  // NOLINT
            return BitTableIterator(row_.table_, row_.rowIndex_ + n);
        }
    }
    auto operator-(int32_t n) const
    {
        if constexpr (REVERSE_ITERATION) {  // NOLINT
            return BitTableIterator(row_.table_, row_.row_index_ + n);
        } else {  // NOLINT
            return BitTableIterator(row_.table_, row_.row_index_ - n);
        }
    }
    int32_t operator-(const BitTableIterator &rhs) const
    {
        if constexpr (REVERSE_ITERATION) {  // NOLINT
            return rhs.row_.rowIndex_ - row_.rowIndex_;
        } else {  // NOLINT
            return row_.rowIndex_ - rhs.row_.rowIndex_;
        }
    }

    auto &operator+=(int32_t n)
    {
        if constexpr (REVERSE_ITERATION) {  // NOLINT
            row_.row_index_ -= n;
        } else {  // NOLINT
            row_.rowIndex_ += n;
        }
        return *this;
    }

    Accessor &operator*()
    {
        return row_;
    }
    Accessor *operator->()
    {
        return &row_;
    }

    ~BitTableIterator() = default;

    DEFAULT_COPY_SEMANTIC(BitTableIterator);
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(BitTableIterator);

private:
    Accessor row_;
};

template <size_t NUM_COLUMNS>
struct BitTableDefault : public BitTableRow<NUM_COLUMNS, BitTableDefault<NUM_COLUMNS>> {
    using Base = BitTableRow<NUM_COLUMNS, BitTableDefault<NUM_COLUMNS>>;
    using Base::Base;
};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BIT_TABLE_HEADER(columns, name)            \
    using Base = BitTableRow<columns, name>;       \
    using BitTableRow<columns, name>::BitTableRow; \
    template <int Column, int Dummy>               \
    struct ColumnName;                             \
    static constexpr const char *TABLE_NAME = #name

// CC-OFFNXT(G.PRE.06) solid logic
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BIT_TABLE_COLUMN(index, name, upname)                                \
    static constexpr size_t COLUMN_##upname = (index);                       \
    static constexpr const char COLUMN_NAME_##upname[] = #name; /* NOLINT */ \
    template <int Dummy>                                                     \
    struct ColumnName<index, Dummy> {                                        \
        static constexpr const char VALUE[] = #name; /* NOLINT */            \
    };                                                                       \
    uint32_t Get##name() const                                               \
    {                                                                        \
        /* CC-OFFNXT(G.PRE.05) function gen */                               \
        return Get(index);                                                   \
    }                                                                        \
    bool Has##name() const                                                   \
    {                                                                        \
        /* CC-OFFNXT(G.PRE.05) function gen */                               \
        return Get(index) != NO_VALUE;                                       \
    }

template <typename Accessor, size_t... COLUMNS>
// NOLINTNEXTLINE(readability-named-parameter)
static const char *const *GetBitTableColumnNamesImpl(std::index_sequence<COLUMNS...>)
{
    static const std::array<const char *, sizeof...(COLUMNS)> NAMES = {
        Accessor::template ColumnName<COLUMNS, 0>::VALUE...};
    return NAMES.data();
}

template <typename AccessorT>
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class BitTable {
public:
    static constexpr uint32_t NO_VALUE = -1;
    static constexpr uint32_t NO_VALUE_DIFF = -1;

    static constexpr size_t NUM_COLUMNS = AccessorT::NUM_COLUMNS;

    using Accessor = AccessorT;
    using RegionType = BitMemoryRegion<const uint8_t>;

public:
    template <bool REVERSED>
    class Range {
    public:
        using RangeIterator =
            std::conditional_t<REVERSED, BitTableIterator<Accessor, true>, BitTableIterator<Accessor, false>>;

        Range(BitTable *table, int start, int end) : table_(table), start_(start), end_(end) {}

        // NOLINTBEGIN(readability-identifier-naming)
        auto begin()
        {
            return RangeIterator(table_, start_);
        }
        auto end()
        {
            return RangeIterator(table_, end_);
        }
        // NOLINTEND(readability-identifier-naming)

        Accessor operator[](size_t index)
        {
            return *(RangeIterator(table_, start_) + index);
        }

    private:
        BitTable *table_;
        int start_;
        int end_;
    };

    // NOLINTBEGIN(readability-identifier-naming)
    auto begin() const
    {
        return BitTableIterator<Accessor, false>(this, 0);
    }

    auto begin()
    {
        return BitTableIterator<Accessor, false>(this, 0);
    }

    auto end() const
    {
        return BitTableIterator<Accessor, false>(this, GetRowsCount());
    }

    auto end()
    {
        return BitTableIterator<Accessor, false>(this, GetRowsCount());
    }
    // NOLINTEND(readability-identifier-naming)

    auto GetRange(int start, int end)
    {
        return Range<false>(this, start, end);
    }
    auto GetRangeReversed(int start, int end)
    {
        return Range<true>(this, end - 1, start - 1);
    }
    auto GetRangeReversed()
    {
        if (GetRowsCount() == 0) {
            return Range<true>(this, -1, -1);
        }
        return Range<true>(this, GetRowsCount() - 1, -1);
    }

    constexpr size_t GetColumnsCount() const
    {
        return NUM_COLUMNS;
    }

    size_t GetRowsCount() const
    {
        return rowsCount_;
    }

    size_t GetRowSizeInBits() const
    {
        return columnsOffsets_[NUM_COLUMNS];
    }

    size_t GetColumnWidth(size_t index) const
    {
        return columnsOffsets_[index + 1] - columnsOffsets_[index];
    }

    uint32_t ReadColumn(size_t rowIndex, size_t column) const
    {
        ASSERT(column < GetColumnsCount());
        return region_.Read(rowIndex * GetRowSizeInBits() + columnsOffsets_[column], GetColumnWidth(column)) +
               NO_VALUE_DIFF;
    }

    auto GetRow(size_t index)
    {
        ASSERT(index < GetRowsCount());
        return Accessor(this, index);
    }

    auto GetRow(size_t index) const
    {
        ASSERT(index < GetRowsCount());
        return Accessor(this, index);
    }

    auto GetInvalidRow() const
    {
        return Accessor(this, -1);
    }

    RegionType GetBitMemoryRegion(uint32_t row) const
    {
        if (row == NO_VALUE) {
            return RegionType();
        }
        size_t offset = row * GetRowSizeInBits() + columnsOffsets_[0];
        return region_.Subregion(offset, GetColumnWidth(0));
    }

    void Decode(BitMemoryStreamIn *stream)
    {
        auto columns = VarintPack::Read<NUM_COLUMNS + 1>(stream);
        rowsCount_ = columns[NUM_COLUMNS];

        // Calculate offsets
        columnsOffsets_[0] = 0;
        for (size_t i = 0; i < NUM_COLUMNS; i++) {
            columnsOffsets_[i + 1] = columnsOffsets_[i] + columns[i];
        }

        region_ = stream->ReadRegion(GetRowsCount() * GetRowSizeInBits());
    }

    void Dump(std::ostream &out = std::cout) const
    {
        out << "BitTable: " << Accessor::TABLE_NAME << ", rows=" << GetRowsCount()
            << ", row_size=" << GetRowSizeInBits() << std::endl;
        static constexpr size_t INDENT = 4;
        std::array<uint32_t, NUM_COLUMNS> width {};
        out << "    ";
        for (size_t i = 0; i < NUM_COLUMNS; i++) {
            out << GetColumnName(i) << " ";
            width[i] = strlen(GetColumnName(i)) + 1;
        }
        out << std::endl;
        int index = 0;
        for (auto row : *this) {
            static constexpr size_t INDEX_WIDTH = 2;
            out << std::right << std::setw(INDEX_WIDTH) << index++ << ": ";
            for (size_t i = 0; i < NUM_COLUMNS; i++) {
                std::string indent(INDENT, ' ');
                out << std::left << std::setw(width[i]);
                out << row.GetColumnStr(i);
            }
            out << std::endl;
        }
    }

    const char *GetColumnName(size_t index) const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return GetBitTableColumnNamesImpl<Accessor>(std::make_index_sequence<NUM_COLUMNS>())[index];
    }

private:
    BitMemoryRegion<const uint8_t> region_;
    std::array<uint16_t, NUM_COLUMNS + 1> columnsOffsets_ {};
    uint16_t rowsCount_ {0};
};

template <typename Accessor>
class BitTableBuilder {
public:
    static constexpr size_t NUM_COLUMNS = Accessor::NUM_COLUMNS;
    static constexpr uint32_t NO_VALUE = BitTable<Accessor>::NO_VALUE;
    static constexpr uint32_t NO_VALUE_DIFF = BitTable<Accessor>::NO_VALUE_DIFF;

    class Entry {
    public:
        Entry()
        {
            std::fill(data_.begin(), data_.end(), NO_VALUE);
        }

        Entry(std::initializer_list<uint32_t> data)
        {
            std::copy(data.begin(), data.end(), data_.begin());
        }

        uint32_t operator[](size_t index) const
        {
            return data_[index];
        }

        uint32_t &operator[](size_t index)
        {
            return data_[index];
        }

        bool operator==(const Entry &rhs) const
        {
            return data_ == rhs.data_;
        }

        void SetColumn(size_t index, uint32_t value)
        {
            data_[index] = value;
        }

        template <typename... Args>
        void SetColumns(Args... values)
        {
            static_assert(sizeof...(Args) == NUM_COLUMNS);
            int i = 0;
            (SetColumn(i++, values), ...);
        }

        ~Entry() = default;

        DEFAULT_COPY_SEMANTIC(Entry);
        DEFAULT_NOEXCEPT_MOVE_SEMANTIC(Entry);

    private:
        std::array<uint32_t, NUM_COLUMNS> data_;
    };

public:
    explicit BitTableBuilder(ArenaAllocator *allocator)
        : entries_(allocator->Adapter()), dedupMap_(allocator->Adapter())
    {
    }

    size_t GetRowsCount() const
    {
        return entries_.size();
    }

    const Entry &GetLast() const
    {
        return entries_.back();
    }

    const Entry &GetEntry(size_t index) const
    {
        return entries_[index];
    }

    size_t GetSize() const
    {
        return entries_.size();
    }

    ALWAYS_INLINE void Emplace(Entry entry)
    {
        entries_.emplace_back(entry);
    }

    ALWAYS_INLINE size_t Add(Entry entry)
    {
        return AddArray(Span(&entry, 1));
    }

    size_t AddArray(Span<Entry> entries)
    {
        uint32_t hash = FnvHash(entries.template SubSpan<uint32_t>(0, entries.size() * NUM_COLUMNS));
        auto range = dedupMap_.equal_range(hash);
        for (auto it = range.first; it != range.second; ++it) {
            uint32_t row = it->second;
            if (entries.size() + row <= entries_.size() &&
                std::equal(entries.begin(), entries.end(), entries_.begin() + row)) {
                return row;
            }
        }
        uint32_t row = GetRowsCount();
        entries_.insert(entries_.end(), entries.begin(), entries.end());
        dedupMap_.emplace(hash, row);
        return row;
    }

    void CalculateColumnsWidth(Span<uint32_t> columnsWidth)
    {
        std::fill(columnsWidth.begin(), columnsWidth.end(), 0);
        for (const auto &entry : entries_) {
            for (size_t i = 0; i < NUM_COLUMNS; i++) {
                columnsWidth[i] |= entry[i] - NO_VALUE_DIFF;
            }
        }
        for (auto &value : columnsWidth) {
            value = MinimumBitsToStore(value);
        }
    }

    template <typename Container>
    void Encode(BitMemoryStreamOut<Container> &stream)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
        std::array<uint32_t, NUM_COLUMNS + 1> columnsWidth;
        CalculateColumnsWidth(Span(columnsWidth.data(), NUM_COLUMNS));
        columnsWidth[NUM_COLUMNS] = entries_.size();
        VarintPack::Write(stream, columnsWidth);

        for (const auto &entry : entries_) {
            for (size_t i = 0; i < NUM_COLUMNS; i++) {
                stream.Write(entry[i] - NO_VALUE_DIFF, columnsWidth[i]);
            }
        }
    }

    virtual ~BitTableBuilder() = default;

    NO_COPY_SEMANTIC(BitTableBuilder);
    NO_MOVE_SEMANTIC(BitTableBuilder);

private:
    ArenaDeque<Entry> entries_;
    ArenaUnorderedMultiMap<uint32_t, uint32_t> dedupMap_;
};

class BitmapTableBuilder {
public:
    using Entry = std::pair<uint32_t *, uint32_t>;

    explicit BitmapTableBuilder(ArenaAllocator *allocator)
        : allocator_(allocator), rows_(allocator->Adapter()), dedupMap_(allocator->Adapter())
    {
    }

    size_t GetRowsCount() const
    {
        return rows_.size();
    }

    auto GetEntry(size_t index) const
    {
        return BitMemoryRegion<uint32_t>(rows_[index].first, rows_[index].second);
    }

    size_t Add(BitVectorSpan vec)
    {
        if (vec.empty()) {
            return BitTableDefault<1>::NO_VALUE;
        }
        uint32_t hash = FnvHash(vec.GetContainerDataSpan());
        auto range = dedupMap_.equal_range(hash);
        for (auto it = range.first; it != range.second; ++it) {
            auto &row = rows_[it->second];
            if (BitVectorSpan(row.first, row.second) == vec) {
                return it->second;
            }
        }
        size_t vecSizeInBytes = BitsToBytesRoundUp(vec.size());
        size_t dataSizeInBytes = RoundUp(vecSizeInBytes, sizeof(uint32_t));
        auto data = allocator_->AllocArray<uint32_t>(dataSizeInBytes / sizeof(uint32_t));
        if (dataSizeInBytes != 0) {
            errno_t res = memcpy_s(data, vecSizeInBytes, vec.data(), vecSizeInBytes);
            if (res != EOK) {
                UNREACHABLE();
            }
        }

        // Clear extra bytes, which are not be covered by vector's data
        size_t extraBytes = dataSizeInBytes - vecSizeInBytes;
        if (extraBytes != 0) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic,-warnings-as-errors)
            memset_s(reinterpret_cast<uint8_t *>(data) + vecSizeInBytes, extraBytes, 0, extraBytes);
        }
        uint32_t index = rows_.size();
        rows_.push_back({data, vec.size()});
        dedupMap_.emplace(hash, index);
        return index;
    }

    template <typename Container>
    void Encode(BitMemoryStreamOut<Container> &stream)
    {
        static constexpr size_t COLUMNS_SIZE = 2;
        std::array<uint32_t, COLUMNS_SIZE> columns = {0, static_cast<uint32_t>(rows_.size())};
        std::for_each(rows_.begin(), rows_.end(),
                      [&columns](const auto &row) { columns[0] = std::max(columns[0], row.second); });
        VarintPack::Write(stream, columns);

        for (const auto &row : rows_) {
            stream.Write(row.first, row.second, columns[0]);
        }
    }

    virtual ~BitmapTableBuilder() = default;

    NO_COPY_SEMANTIC(BitmapTableBuilder);
    NO_MOVE_SEMANTIC(BitmapTableBuilder);

private:
    ArenaAllocator *allocator_ {nullptr};
    ArenaDeque<Entry> rows_;
    ArenaUnorderedMultiMap<uint32_t, uint32_t> dedupMap_;
};

}  // namespace ark

#endif  // PANDA_BITTABLE_H
