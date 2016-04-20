extern void extern_used();
extern void extern_notused();
void notused();
void notdefined();
int used(int x);

static void static_define(void)
{
}

int main()
{
  extern_used();
  static_define();
}

static void static_notused()
{
}
