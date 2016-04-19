static void foo();

int
bar()
{
  void (*pf)() = foo;
  pf();
  return 1;
}

#define macro macro
void macro()
{
}

#define build(x) void x();
build(xxx)

#define BUG_ON(x) void hidden();
BUG_ON(3)

#define FOO foo()
int main()
{
  foo();
  bar();
  FOO;
}

static void foo()
{
}
