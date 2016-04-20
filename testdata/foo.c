void extern_used()
{
}

void extern_notused()
{
}

void notused()
{
}

static void static_declare(void);

int used(int x)
{
  static_declare();
  return 2*x;
}

static void static_declare(void)
{
  // defined later
}
