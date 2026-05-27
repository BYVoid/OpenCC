#ifndef MARISA_ASSERT_H_
#define MARISA_ASSERT_H_

#include <cstdlib>
#include <iostream>

#define ASSERT(cond)                                                       \
  (void)((!!(cond)) || ((std::cout << __LINE__ << ": Assertion `" << #cond \
                                   << "' failed.\n"),                      \
                        std::exit(-1), 0))

#define EXCEPT(code, expected_error_type)                               \
  try {                                                                 \
    code;                                                               \
    std::cout << __LINE__ << ": Exception `" << #code << "' failed.\n"; \
    std::exit(-1);                                                      \
  } catch (const expected_error_type &) {                               \
  } catch (...) {                                                       \
    ASSERT(false);                                                      \
  }

#define TEST_START() \
  (std::cout << __FILE__ << ":" << __LINE__ << ": " << __FUNCTION__ << "(): ")

#define TEST_END() (std::cout << "ok\n")

#endif  // MARISA_ASSERT_H_
