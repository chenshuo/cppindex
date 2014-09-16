#include <errno.h>
#include <stdint.h>
#include "cell.h"

#define HELLO 42
struct User
{

};

int foo()
{
  return HELLO;
}

int foo(int);
int foo(const char*);
#undef HELLO
int bar()
{
  // hello
  User u;
  /*
   *
   * bad
   */
  int HELLO = 123;
  foo(errno);
  return foo("hello");
}
