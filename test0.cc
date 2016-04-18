void foo();

int
bar()
{
  void (*pf)() = foo;
  pf();
  return 1;
}

#define FOO foo()
int main()
{
  foo();
  bar();
  FOO;
}

void foo()
{
}
