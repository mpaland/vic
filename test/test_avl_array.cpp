///////////////////////////////////////////////////////////////////////////////
// \brief Smoke tests for lib/avl_array.h (used by the sprites)
///////////////////////////////////////////////////////////////////////////////

#include "vic_test.h"
#include "util.h"
#include "lib/avl_array.h"

using namespace vic;

namespace {

template<bool Fast>
void run_basic()
{
  typedef avl_array<vertex_type, std::uint32_t, std::uint16_t, 8U, Fast> array_type;
  array_type a;
  CHECK(a.empty());
  CHECK_EQ(a.size(), 0U);
  CHECK_EQ(a.max_size(), 8U);
  CHECK(a.begin() == a.end());

  // insert 8 elements in random order
  static const vertex_type keys[8] = { { 3, 1 }, { 0, 0 }, { 5, 2 }, { 1, 0 }, { 2, 1 }, { 9, 9 }, { 7, 2 }, { 4, 1 } };
  for (std::uint16_t i = 0U; i < 8U; ++i) {
    CHECK(a.insert(keys[i], 100U + i));
  }
  CHECK_EQ(a.size(), 8U);
  CHECK(!a.empty());
  CHECK(a.check());

  // the container is full
  CHECK(!a.insert({ 8, 8 }, 999U));
  CHECK_EQ(a.size(), 8U);

  // find all
  for (std::uint16_t i = 0U; i < 8U; ++i) {
    std::uint32_t v = 0U;
    CHECK(a.find(keys[i], v));
    CHECK_EQ(v, 100U + i);
    CHECK(a.find(keys[i]) != a.end());
    CHECK_EQ(a.count(keys[i]), 1U);
  }
  std::uint32_t v = 0U;
  CHECK(!a.find({ 8, 8 }, v));
  CHECK(a.find({ 8, 8 }) == a.end());
  CHECK_EQ(a.count({ 8, 8 }), 0U);

  // iteration is sorted by key (y major, then x)
  vertex_type last = { -1, -1 };
  std::size_t n = 0U;
  for (typename array_type::iterator it = a.begin(); it != a.end(); ++it, ++n) {
    CHECK(last < it.key());
    last = it.key();
  }
  CHECK_EQ(n, 8U);

  // update an existing key
  CHECK(a.insert(keys[2], 555U));
  CHECK_EQ(a.size(), 8U);
  CHECK(a.find(keys[2], v));
  CHECK_EQ(v, 555U);

  // modify through the iterator
  typename array_type::iterator it = a.find(keys[0]);
  *it = 42U;
  CHECK(a.find(keys[0], v));
  CHECK_EQ(v, 42U);

  // erase by key
  CHECK(a.erase(keys[5]));
  CHECK(!a.erase(keys[5]));
  CHECK_EQ(a.size(), 7U);
  CHECK(!a.find(keys[5], v));
  CHECK(a.check());

  // erase by iterator
  CHECK(a.erase(a.find(keys[1])));
  CHECK_EQ(a.size(), 6U);
  CHECK(!a.find(keys[1], v));
  CHECK(a.check());
  // erasing end() fails
  CHECK(!a.erase(a.end()));

  // now there is room again
  CHECK(a.insert({ 8, 8 }, 999U));
  CHECK_EQ(a.size(), 7U);

  // erase all remaining via iterator and check the order each time
  while (!a.empty()) {
    const std::size_t before = a.size();
    CHECK(a.erase(a.begin()));
    CHECK_EQ(a.size(), before - 1U);
    CHECK(a.check());
  }
  CHECK(a.begin() == a.end());

  // clear
  a.insert({ 1, 1 }, 1U);
  a.insert({ 2, 2 }, 2U);
  a.clear();
  CHECK(a.empty());
  CHECK(a.insert({ 1, 1 }, 1U));
}

} // namespace


TEST_CASE(avl_array_basic_fast)
{
  run_basic<true>();
}


TEST_CASE(avl_array_basic_slow)
{
  run_basic<false>();
}


TEST_CASE(avl_array_stress)
{
  // insert and erase a lot in a pseudo random order, the tree must stay balanced and consistent
  typedef avl_array<std::uint16_t, std::uint16_t, std::uint16_t, 64U, true> array_type;
  array_type a;
  std::uint32_t seed = 12345U;
  bool present[200] = { false };
  std::size_t count = 0U;
  for (int round = 0; round < 2000; ++round) {
    seed = seed * 1103515245U + 12345U;
    const std::uint16_t key = static_cast<std::uint16_t>((seed >> 16U) % 200U);
    if (present[key]) {
      CHECK(a.erase(key));
      present[key] = false;
      --count;
    }
    else if (count < 64U) {
      CHECK(a.insert(key, key));
      present[key] = true;
      ++count;
    }
    else {
      CHECK(!a.insert(key, key));
    }
    CHECK_EQ(a.size(), count);
  }
  CHECK(a.check());
  // every present key is found, the iteration order is ascending
  std::uint16_t last = 0U;
  bool first = true;
  for (array_type::iterator it = a.begin(); it != a.end(); ++it) {
    CHECK(present[it.key()]);
    CHECK(first || (it.key() > last));
    last = it.key();
    first = false;
  }
  for (std::uint16_t k = 0U; k < 200U; ++k) {
    std::uint16_t v;
    CHECK_EQ(a.find(k, v), present[k]);
  }
}
