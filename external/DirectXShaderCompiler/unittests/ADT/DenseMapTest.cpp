//===- llvm/unittest/ADT/DenseMapMap.cpp - DenseMap unit tests --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "gtest/gtest.h"
#include "llvm/ADT/DenseMap.h"
#include <map>
#include <set>

using namespace llvm;

namespace {

uint32_t getTestKey(int i, uint32_t *) { return i; }
uint32_t getTestValue(int i, uint32_t *) { return 42 + i; }

uint32_t *getTestKey(int i, uint32_t **) {
  static uint32_t dummy_arr1[8192];
  assert(i < 8192 && "Only support 8192 dummy keys.");
  return &dummy_arr1[i];
}
uint32_t *getTestValue(int i, uint32_t **) {
  static uint32_t dummy_arr1[8192];
  assert(i < 8192 && "Only support 8192 dummy keys.");
  return &dummy_arr1[i];
}

/// \brief A test class that tries to check that construction and destruction
/// occur correctly.
class CtorTester {
  static std::set<CtorTester *> Constructed;
  int Value;

public:
  explicit CtorTester(int Value = 0) : Value(Value) {
    EXPECT_TRUE(Constructed.insert(this).second);
  }
  CtorTester(uint32_t Value) : Value(Value) {
    EXPECT_TRUE(Constructed.insert(this).second);
  }
  CtorTester(const CtorTester &Arg) : Value(Arg.Value) {
    EXPECT_TRUE(Constructed.insert(this).second);
  }
  CtorTester &operator=(const CtorTester &) = default;
  ~CtorTester() {
    EXPECT_EQ(1u, Constructed.erase(this));
  }
  operator uint32_t() const { return Value; }

  int getValue() const { return Value; }
  bool operator==(const CtorTester &RHS) const { return Value == RHS.Value; }
};

std::set<CtorTester *> CtorTester::Constructed;

struct CtorTesterMapInfo {
  static inline CtorTester getEmptyKey() { return CtorTester(-1); }
  static inline CtorTester getTombstoneKey() { return CtorTester(-2); }
  static unsigned getHashValue(const CtorTester &Val) {
    return Val.getValue() * 37u;
  }
  static bool isEqual(const CtorTester &LHS, const CtorTester &RHS) {
    return LHS == RHS;
  }
};

CtorTester getTestKey(int i, CtorTester *) { return CtorTester(i); }
CtorTester getTestValue(int i, CtorTester *) { return CtorTester(42 + i); }

// Test fixture, with helper functions implemented by forwarding to global
// function overloads selected by component types of the type parameter. This
// allows all of the map implementations to be tested with shared
// implementations of helper routines.
template <typename T>
class DenseMapTest : public ::testing::Test {
protected:
  T Map;

  static typename T::key_type *const dummy_key_ptr;
  static typename T::mapped_type *const dummy_value_ptr;

  typename T::key_type getKey(int i = 0) {
    return getTestKey(i, dummy_key_ptr);
  }
  typename T::mapped_type getValue(int i = 0) {
    return getTestValue(i, dummy_value_ptr);
  }
};

template <typename T>
typename T::key_type *const DenseMapTest<T>::dummy_key_ptr = nullptr;
template <typename T>
typename T::mapped_type *const DenseMapTest<T>::dummy_value_ptr = nullptr;

// Register these types for testing.
typedef ::testing::Types<DenseMap<uint32_t, uint32_t>,
                         DenseMap<uint32_t *, uint32_t *>,
                         DenseMap<CtorTester, CtorTester, CtorTesterMapInfo>,
                         SmallDenseMap<uint32_t, uint32_t>,
                         SmallDenseMap<uint32_t *, uint32_t *>,
                         SmallDenseMap<CtorTester, CtorTester, 4,
                                       CtorTesterMapInfo>
                         > DenseMapTestTypes;
TYPED_TEST_CASE(DenseMapTest, DenseMapTestTypes);

// Empty map tests
TYPED_TEST(DenseMapTest, EmptyIntMapTest) {
  // Size tests
  EXPECT_EQ(0u, this->Map.size());
  EXPECT_TRUE(this->Map.empty());

  // Iterator tests
  EXPECT_TRUE(this->Map.begin() == this->Map.end());

  // Lookup tests
  EXPECT_FALSE(this->Map.count(this->getKey()));
  EXPECT_TRUE(this->Map.find(this->getKey()) == this->Map.end());
#if !defined(_MSC_VER) || defined(__clang__)
  EXPECT_EQ(typename TypeParam::mapped_type(),
            this->Map.lookup(this->getKey()));
#else
  // MSVC, at least old versions, cannot parse the typename to disambiguate
  // TypeParam::mapped_type as a type. However, because MSVC doesn't implement
  // two-phase name lookup, it also doesn't require the typename. Deal with
  // this mutual incompatibility through specialized code.
  EXPECT_EQ(TypeParam::mapped_type(),
            this->Map.lookup(this->getKey()));
#endif
}

// Constant map tests
TYPED_TEST(DenseMapTest, ConstEmptyMapTest) {
  const TypeParam &ConstMap = this->Map;
  EXPECT_EQ(0u, ConstMap.size());
  EXPECT_TRUE(ConstMap.empty());
  EXPECT_TRUE(ConstMap.begin() == ConstMap.end());
}

// A map with a single entry
TYPED_TEST(DenseMapTest, SingleEntryMapTest) {
  this->Map[this->getKey()] = this->getValue();

  // Size tests
  EXPECT_EQ(1u, this->Map.size());
  EXPECT_FALSE(this->Map.begin() == this->Map.end());
  EXPECT_FALSE(this->Map.empty());

  // Iterator tests
  typename TypeParam::iterator it = this->Map.begin();
  EXPECT_EQ(this->getKey(), it->first);
  EXPECT_EQ(this->getValue(), it->second);
  ++it;
  EXPECT_TRUE(it == this->Map.end());

  // Lookup tests
  EXPECT_TRUE(this->Map.count(this->getKey()));
  EXPECT_TRUE(this->Map.find(this->getKey()) == this->Map.begin());
  EXPECT_EQ(this->getValue(), this->Map.lookup(this->getKey()));
  EXPECT_EQ(this->getValue(), this->Map[this->getKey()]);
}

// Test clear() method
TYPED_TEST(DenseMapTest, ClearTest) {
  this->Map[this->getKey()] = this->getValue();
  this->Map.clear();

  EXPECT_EQ(0u, this->Map.size());
  EXPECT_TRUE(this->Map.empty());
  EXPECT_TRUE(this->Map.begin() == this->Map.end());
}

// Test erase(iterator) method
TYPED_TEST(DenseMapTest, EraseTest) {
  this->Map[this->getKey()] = this->getValue();
  this->Map.erase(this->Map.begin());

  EXPECT_EQ(0u, this->Map.size());
  EXPECT_TRUE(this->Map.empty());
  EXPECT_TRUE(this->Map.begin() == this->Map.end());
}

// Test erase(value) method
TYPED_TEST(DenseMapTest, EraseTest2) {
  this->Map[this->getKey()] = this->getValue();
  this->Map.erase(this->getKey());

  EXPECT_EQ(0u, this->Map.size());
  EXPECT_TRUE(this->Map.empty());
  EXPECT_TRUE(this->Map.begin() == this->Map.end());
}

// Test insert() method
TYPED_TEST(DenseMapTest, InsertTest) {
  this->Map.insert(std::make_pair(this->getKey(), this->getValue()));
  EXPECT_EQ(1u, this->Map.size());
  EXPECT_EQ(this->getValue(), this->Map[this->getKey()]);
}

// Test copy constructor method
TYPED_TEST(DenseMapTest, CopyConstructorTest) {
  this->Map[this->getKey()] = this->getValue();
  TypeParam copyMap(this->Map);

  EXPECT_EQ(1u, copyMap.size());
  EXPECT_EQ(this->getValue(), copyMap[this->getKey()]);
}

// Test copy constructor method where SmallDenseMap isn't small.
TYPED_TEST(DenseMapTest, CopyConstructorNotSmallTest) {
  for (int Key = 0; Key < 5; ++Key)
    this->Map[this->getKey(Key)] = this->getValue(Key);
  TypeParam copyMap(this->Map);

  EXPECT_EQ(5u, copyMap.size());
  for (int Key = 0; Key < 5; ++Key)
    EXPECT_EQ(this->getValue(Key), copyMap[this->getKey(Key)]);
}

// Test copying from a default-constructed map.
TYPED_TEST(DenseMapTest, CopyConstructorFromDefaultTest) {
  TypeParam copyMap(this->Map);

  EXPECT_TRUE(copyMap.empty());
}

// Test copying from an empty map where SmallDenseMap isn't small.
TYPED_TEST(DenseMapTest, CopyConstructorFromEmptyTest) {
  for (int Key = 0; Key < 5; ++Key)
    this->Map[this->getKey(Key)] = this->getValue(Key);
  this->Map.clear();
  TypeParam copyMap(this->Map);

  EXPECT_TRUE(copyMap.empty());
}

// Test assignment operator method
TYPED_TEST(DenseMapTest, AssignmentTest) {
  this->Map[this->getKey()] = this->getValue();
  TypeParam copyMap = this->Map;

  EXPECT_EQ(1u, copyMap.size());
  EXPECT_EQ(this->getValue(), copyMap[this->getKey()]);

  // test self-assignment.
  copyMap = copyMap;
  EXPECT_EQ(1u, copyMap.size());
  EXPECT_EQ(this->getValue(), copyMap[this->getKey()]);
}

// Test swap method
TYPED_TEST(DenseMapTest, SwapTest) {
  this->Map[this->getKey()] = this->getValue();
  TypeParam otherMap;

  this->Map.swap(otherMap);
  EXPECT_EQ(0u, this->Map.size());
  EXPECT_TRUE(this->Map.empty());
  EXPECT_EQ(1u, otherMap.size());
  EXPECT_EQ(this->getValue(), otherMap[this->getKey()]);

  this->Map.swap(otherMap);
  EXPECT_EQ(0u, otherMap.size());
  EXPECT_TRUE(otherMap.empty());
  EXPECT_EQ(1u, this->Map.size());
  EXPECT_EQ(this->getValue(), this->Map[this->getKey()]);

  // Make this more interesting by inserting 100 numbers into the map.
  for (int i = 0; i < 100; ++i)
    this->Map[this->getKey(i)] = this->getValue(i);

  this->Map.swap(otherMap);
  EXPECT_EQ(0u, this->Map.size());
  EXPECT_TRUE(this->Map.empty());
  EXPECT_EQ(100u, otherMap.size());
  for (int i = 0; i < 100; ++i)
    EXPECT_EQ(this->getValue(i), otherMap[this->getKey(i)]);

  this->Map.swap(otherMap);
  EXPECT_EQ(0u, otherMap.size());
  EXPECT_TRUE(otherMap.empty());
  EXPECT_EQ(100u, this->Map.size());
  for (int i = 0; i < 100; ++i)
    EXPECT_EQ(this->getValue(i), this->Map[this->getKey(i)]);
}

// A more complex iteration test
TYPED_TEST(DenseMapTest, IterationTest) {
  bool visited[100];
  std::map<typename TypeParam::key_type, unsigned> visitedIndex;

  // Insert 100 numbers into the map
  for (int i = 0; i < 100; ++i) {
    visited[i] = false;
    visitedIndex[this->getKey(i)] = i;

    this->Map[this->getKey(i)] = this->getValue(i);
  }

  // Iterate over all numbers and mark each one found.
  for (typename TypeParam::iterator it = this->Map.begin();
       it != this->Map.end(); ++it)
    visited[visitedIndex[it->first]] = true;

  // Ensure every number was visited.
  for (int i = 0; i < 100; ++i)
    ASSERT_TRUE(visited[i]) << "Entry #" << i << " was never visited";
}

// const_iterator test
TYPED_TEST(DenseMapTest, ConstIteratorTest) {
  // Check conversion from iterator to const_iterator.
  typename TypeParam::iterator it = this->Map.begin();
  typename TypeParam::const_iterator cit(it);
  EXPECT_TRUE(it == cit);

  // Check copying of const_iterators.
  typename TypeParam::const_iterator cit2(cit);
  EXPECT_TRUE(cit == cit2);
}

// Make sure DenseMap works with StringRef keys.
TEST(DenseMapCustomTest, StringRefTest) {
  DenseMap<StringRef, int> M;

  M["a"] = 1;
  M["b"] = 2;
  M["c"] = 3;

  EXPECT_EQ(3u, M.size());
  EXPECT_EQ(1, M.lookup("a"));
  EXPECT_EQ(2, M.lookup("b"));
  EXPECT_EQ(3, M.lookup("c"));

  EXPECT_EQ(0, M.lookup("q"));

  // Test the empty string, spelled various ways.
  EXPECT_EQ(0, M.lookup(""));
  EXPECT_EQ(0, M.lookup(StringRef()));
  EXPECT_EQ(0, M.lookup(StringRef("a", 0)));
  M[""] = 42;
  EXPECT_EQ(42, M.lookup(""));
  EXPECT_EQ(42, M.lookup(StringRef()));
  EXPECT_EQ(42, M.lookup(StringRef("a", 0)));
}

// Key traits that allows lookup with either an unsigned or char* key;
// In the latter case, "a" == 0, "b" == 1 and so on.
struct TestDenseMapInfo {
  static inline unsigned getEmptyKey() { return ~0; }
  static inline unsigned getTombstoneKey() { return ~0U - 1; }
  static unsigned getHashValue(const unsigned& Val) { return Val * 37U; }
  static unsigned getHashValue(const char* Val) {
    return (unsigned)(Val[0] - 'a') * 37U;
  }
  static bool isEqual(const unsigned& LHS, const unsigned& RHS) {
    return LHS == RHS;
  }
  static bool isEqual(const char* LHS, const unsigned& RHS) {
    return (unsigned)(LHS[0] - 'a') == RHS;
  }
};

// find_as() tests
TEST(DenseMapCustomTest, FindAsTest) {
  DenseMap<unsigned, unsigned, TestDenseMapInfo> map;
  map[0] = 1;
  map[1] = 2;
  map[2] = 3;

  // Size tests
  EXPECT_EQ(3u, map.size());

  // Normal lookup tests
  EXPECT_EQ(1u, map.count(1));
  EXPECT_EQ(1u, map.find(0)->second);
  EXPECT_EQ(2u, map.find(1)->second);
  EXPECT_EQ(3u, map.find(2)->second);
  EXPECT_TRUE(map.find(3) == map.end());

  // find_as() tests
  EXPECT_EQ(1u, map.find_as("a")->second);
  EXPECT_EQ(2u, map.find_as("b")->second);
  EXPECT_EQ(3u, map.find_as("c")->second);
  EXPECT_TRUE(map.find_as("d") == map.end());
}

struct ContiguousDenseMapInfo {
  static inline unsigned getEmptyKey() { return ~0; }
  static inline unsigned getTombstoneKey() { return ~0U - 1; }
  static unsigned getHashValue(const unsigned& Val) { return Val; }
  static bool isEqual(const unsigned& LHS, const unsigned& RHS) {
    return LHS == RHS;
  }
};

// Test that filling a small dense map with exactly the number of elements in
// the map grows to have enough space for an empty bucket.
TEST(DenseMapCustomTest, SmallDenseMapGrowTest) {
  SmallDenseMap<unsigned, unsigned, 32, ContiguousDenseMapInfo> map;
  // Add some number of elements, then delete a few to leave us some tombstones.
  // If we just filled the map with 32 elements we'd grow because of not enough
  // tombstones which masks the issue here.
  for (unsigned i = 0; i < 20; ++i)
    map[i] = i + 1;
  for (unsigned i = 0; i < 10; ++i)
    map.erase(i);
  for (unsigned i = 20; i < 32; ++i)
    map[i] = i + 1;

  // Size tests
  EXPECT_EQ(22u, map.size());

  // Try to find an element which doesn't exist.  There was a bug in
  // SmallDenseMap which led to a map with num elements == small capacity not
  // having an empty bucket any more.  Finding an element not in the map would
  // therefore never terminate.
  EXPECT_TRUE(map.find(32) == map.end());
}

}
