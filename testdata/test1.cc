#include <errno.h>
#include <stdint.h>
#include <stdio.h>
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
  foo(R"(hello
 world)");
  return foo("hello");
}

int foo(const char* arg)
{
  printf("%s\n", arg);
  return 1234;
}
