#include <stdio.h>
#include <list>

//template <class T>
class foo {
public:
  foo();
private:
  typedef struct _foostruct {
    int x, y;
  } foostruct;
};

//template <class T>
//foo<T>::foo()
foo::foo()
{
  std::list<foostruct*>::const_iterator x;
  return;
}

int main(int argc, char *argv[])
{
  //foo<char> x;
  foo x;
  printf("done\n");
  return 1;
}
