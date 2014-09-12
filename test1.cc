#include <stdint.h>

struct User
{
};

int foo();

int bar()
{
  User u;
  return foo();
}
