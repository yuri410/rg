//===--- UnicodeCharRanges.h - Types and functions for character ranges ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_SUPPORT_UNICODECHARRANGES_H
#define LLVM_SUPPORT_UNICODECHARRANGES_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Mutex.h"
#include "llvm/Support/MutexGuard.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>

namespace llvm {
namespace sys {

#define DEBUG_TYPE "unicode"

/// \brief Represents a closed range of Unicode code points [Lower, Upper].
struct UnicodeCharRange {
  uint32_t Lower;
  uint32_t Upper;
};

inline bool operator<(uint32_t Value, UnicodeCharRange Range) {
  return Value < Range.Lower;
}
inline bool operator<(UnicodeCharRange Range, uint32_t Value) {
  return Range.Upper < Value;
}

/// \brief Holds a reference to an ordered array of UnicodeCharRange and allows
/// to quickly check if a code point is contained in the set represented by this
/// array.
class UnicodeCharSet {
public:
  typedef ArrayRef<UnicodeCharRange> CharRanges;

  /// \brief Constructs a UnicodeCharSet instance from an array of
  /// UnicodeCharRanges.
  ///
  /// Array pointed by \p Ranges should have the lifetime at least as long as
  /// the UnicodeCharSet instance, and should not change. Array is validated by
  /// the constructor, so it makes sense to create as few UnicodeCharSet
  /// instances per each array of ranges, as possible.
#ifdef NDEBUG
  LLVM_CONSTEXPR UnicodeCharSet(CharRanges Ranges) : Ranges(Ranges) {}
#else
  UnicodeCharSet(CharRanges Ranges) : Ranges(Ranges) {
    assert(rangesAreValid());
  }
#endif

  /// \brief Returns true if the character set contains the Unicode code point
  /// \p C.
  bool contains(uint32_t C) const {
    return std::binary_search(Ranges.begin(), Ranges.end(), C);
  }

private:
  /// \brief Returns true if each of the ranges is a proper closed range
  /// [min, max], and if the ranges themselves are ordered and non-overlapping.
  bool rangesAreValid() const {
    uint32_t Prev = 0;
    for (CharRanges::const_iterator I = Ranges.begin(), E = Ranges.end();
         I != E; ++I) {
      if (I != Ranges.begin() && Prev >= I->Lower) {
        DEBUG(dbgs() << "Upper bound 0x");
        DEBUG(dbgs().write_hex(Prev));
        DEBUG(dbgs() << " should be less than succeeding lower bound 0x");
        DEBUG(dbgs().write_hex(I->Lower) << "\n");
        return false;
      }
      if (I->Upper < I->Lower) {
        DEBUG(dbgs() << "Upper bound 0x");
        DEBUG(dbgs().write_hex(I->Lower));
        DEBUG(dbgs() << " should not be less than lower bound 0x");
        DEBUG(dbgs().write_hex(I->Upper) << "\n");
        return false;
      }
      Prev = I->Upper;
    }

    return true;
  }

  const CharRanges Ranges;
};

#undef DEBUG_TYPE // "unicode"

} // namespace sys
} // namespace llvm


#endif // LLVM_SUPPORT_UNICODECHARRANGES_H
