struct data;
struct data* gdp;
void take_data(struct data* pd)
{
  gdp = pd;
}

struct data
{
  int x;
  double y;
  struct data* next;
};

struct data gd;
struct data* return_data(void)
{
  gd.x = 1;
  gd.y = 19 * 82.43;
  return &gd;
}

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
