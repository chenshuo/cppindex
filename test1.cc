#include <stdint.h>
#include "cell.h"

struct User
{
};

int foo();

int bar()
{
  // hello
  User u;
  /*
   *
   * bad
   */
  return foo();
  "hello";
}
