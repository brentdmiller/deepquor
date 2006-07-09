#include <list>

template <class T>
class foo {
public:
  foo();
private:
  typedef struct _foostruct { int x, y; } foostruct;
};

template <class T> foo<T>::foo() {
  std::list<foostruct*>::iterator x;  // Error on this line!!!
  return;
}
