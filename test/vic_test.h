///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2020, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// This file is part of the vic library.
//
// \brief Minimal self contained unit test framework for the vic tests
// - TEST_CASE(name) registers a test function, the name must be a valid identifier
//   and should be prefixed with the module under test (util_, color_, gpr_ ...)
// - CHECK(expr), CHECK_MSG(expr, msg), CHECK_EQ(a, b) record a failure and continue
// - REQUIRE(expr) records a failure and returns from the test function
// - run_all(argc, argv) runs all (or the --filter <substring> selected) tests,
//   prints a summary and returns 0 on success, 1 on any failure
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VIC_TEST_H_
#define _VIC_TEST_H_

#include <cstdio>
#include <limits>
#include <cstring>
#include <cstdint>


namespace vic_test {


struct test_case {
  const char* name;
  void (*fn)();
  test_case*  next;
};


// registry is a function local static, so it is initialized before the first registrar
inline test_case*& registry()
{
  static test_case* head = nullptr;
  return head;
}


struct registrar {
  explicit registrar(test_case* tc)
  {
    // append at the end to keep the registration (link) order
    test_case** p = &registry();
    while (*p) { p = &(*p)->next; }
    tc->next = nullptr;
    *p = tc;
  }
};


struct context {
  unsigned    tests;
  unsigned    checks;
  unsigned    failures;
  unsigned    failures_in_current;
  const char* current;
};


inline context& ctx()
{
  static context c = { 0U, 0U, 0U, 0U, "" };
  return c;
}


inline void fail_header(const char* file, int line, const char* expr)
{
  ++ctx().failures;
  ++ctx().failures_in_current;
  std::printf("%s(%d): FAILED [%s]: %s", file, line, ctx().current, expr);
}


// value printing, integers (and colors) are printed as decimal and hex
inline void print_val(unsigned long long v) { std::printf("%llu (0x%llX)", v, v); }
inline void print_val(long long v)          { std::printf("%lld (0x%llX)", v, static_cast<unsigned long long>(v)); }
inline void print_val(bool v)               { std::printf("%s", v ? "true" : "false"); }
inline void print_val(const char* v)        { std::printf("\"%s\"", v ? v : "(null)"); }
inline void print_val(char* v)              { print_val(static_cast<const char*>(v)); }
inline void print_val(const void* v)        { std::printf("%p", v); }

template<typename T>
inline void print_val(const T& v)
{
  // generic integer fallback
  if (std::numeric_limits<T>::is_signed) {
    print_val(static_cast<long long>(v));
  }
  else {
    print_val(static_cast<unsigned long long>(v));
  }
}


template<typename A, typename B>
inline bool check_eq(const char* file, int line, const char* ea, const char* eb, const A& a, const B& b)
{
  ++ctx().checks;
  if (a == b) {
    return true;
  }
  fail_header(file, line, ea);
  std::printf(" == %s\n    lhs: ", eb);
  print_val(a);
  std::printf("\n    rhs: ");
  print_val(b);
  std::printf("\n");
  return false;
}


inline bool check(const char* file, int line, const char* expr, bool ok, const char* msg = nullptr)
{
  ++ctx().checks;
  if (ok) {
    return true;
  }
  fail_header(file, line, expr);
  if (msg) {
    std::printf("  -- %s", msg);
  }
  std::printf("\n");
  return false;
}


inline int run_all(int argc, char** argv)
{
  const char* filter = nullptr;
  bool list = false;
  for (int i = 1; i < argc; ++i) {
    if (!std::strcmp(argv[i], "--filter") && (i + 1 < argc)) {
      filter = argv[++i];
    }
    else if (!std::strcmp(argv[i], "--list")) {
      list = true;
    }
    else if (!std::strcmp(argv[i], "--help")) {
      std::printf("usage: %s [--filter <substring>] [--list]\n", argv[0]);
      return 0;
    }
  }

  for (test_case* tc = registry(); tc; tc = tc->next) {
    if (filter && !std::strstr(tc->name, filter)) {
      continue;
    }
    if (list) {
      std::printf("%s\n", tc->name);
      continue;
    }
    ctx().current = tc->name;
    ctx().failures_in_current = 0U;
    ++ctx().tests;
    tc->fn();
    if (ctx().failures_in_current) {
      std::printf("  [FAIL] %s\n", tc->name);
    }
  }

  if (!list) {
    std::printf("\n%u tests, %u checks, %u failures\n", ctx().tests, ctx().checks, ctx().failures);
    if (ctx().tests == 0U) {
      std::printf("WARNING: no test matched the filter\n");
      return 1;
    }
  }
  return ctx().failures ? 1 : 0;
}

} // namespace vic_test


#define VIC_TEST_CAT2(a, b) a##b
#define VIC_TEST_CAT(a, b)  VIC_TEST_CAT2(a, b)

#define TEST_CASE(name)                                                                                           \
  static void VIC_TEST_CAT(vic_test_fn_, name)();                                                                 \
  static vic_test::test_case VIC_TEST_CAT(vic_test_tc_, name) = { #name, &VIC_TEST_CAT(vic_test_fn_, name), nullptr }; \
  static vic_test::registrar VIC_TEST_CAT(vic_test_reg_, name)(&VIC_TEST_CAT(vic_test_tc_, name));                \
  static void VIC_TEST_CAT(vic_test_fn_, name)()

#define CHECK(expr)          vic_test::check(__FILE__, __LINE__, #expr, !!(expr))
#define CHECK_MSG(expr, msg) vic_test::check(__FILE__, __LINE__, #expr, !!(expr), (msg))
#define CHECK_EQ(a, b)       vic_test::check_eq(__FILE__, __LINE__, #a, #b, (a), (b))
#define REQUIRE(expr)        do { if (!vic_test::check(__FILE__, __LINE__, #expr, !!(expr), "REQUIRE failed, test aborted")) { return; } } while (0)

#endif  // _VIC_TEST_H_
