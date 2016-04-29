struct data;
struct data* gdp;
void take_data(struct data* pd)
{
  gdp = pd;
}

struct page_frag { void* p; };
struct data
{
	int	x;
	double	y;
	struct data*		next;
	struct page_frag	mib;
};

struct data gd;
struct data* return_data(void)
{
  gd.x = 1;
  gd.y = 19 * 82.43;
  return &gd;
}

struct decl_only;
struct decl_only* gdo;
void set_gdo(struct decl_only* x)
{
  gdo = x;
}

struct {
  int x, y;
} unname;

typedef struct {
  int a;
} unnamed_t;

typedef struct name {
  int value;
} name_t;

unnamed_t* use_unname(name_t* arg)
{
  unname.x = 12;
  unnamed_t un;
  un.a = 34;
  name_t nn;
  nn.value = 56;
  return 0;
}

enum Gender
{
  kMale, kFemale
};

void use_enum(enum Gender x);

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
