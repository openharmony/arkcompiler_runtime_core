---
name: internationalization
description: Important skill covering Unicode (UTF-8, UTF-16), ICU4C library, locale handling, collation, number/date formatting, and i18n APIs for ETS stdlib. Use this skill when: (1) Implementing i18n features, (2) Using ICU library, (3) Formatting for locales, (4) Handling Unicode text, (5) Supporting RTL languages, or (6) Designing locale-aware APIs.
---

# Internationalization

## Quick Start

**Locale-aware formatting:**
```typescript
// Format number with locale
formatNumber(1234.56, "en_US");  // "1,234.56"
formatNumber(1234.56, "de_DE");  // "1.234,56"
```

**ICU integration:**
```cpp
// C++ side with ICU
auto formatter = icu::NumberFormat::createInstance(locale);
formatter->format(number, result);
```

**Unicode handling:**
- UTF-8, UTF-16 encodings
- Code points vs graphemes
- Normalization (NFC, NFD)

## Key Concepts

**Unicode:**
- Code points: Unique numbers
- Graphemes: User-perceived characters
- Surrogate pairs: Emoji, > 0xFFFF

**ICU Library:**
- Locale handling
- Number/currency formatting
- Date/time formatting
- Collation (sorting)

