
#include <iostream>
#include <vector>
using namespace std;


struct A {
  int x;
  A() {
    cout << "initialize A.." <<endl;
    x = 1;
  }

  A (A& a) {
    cout << "A: another copy constructor is called." << endl;

  }

  A (const A & a) {
    cout << "A: copy constructor is called." << endl;
  }

  A (A && a) {
    cout << "A: rvalue copy constructor is called." << endl;
    a.x = -1;
    x = 99;
  }


  void show() {
    cout << "A: x=" << x << endl;
  }

  ~A() {
    cout << "destruct A..  x=" << x <<endl;
  }

};


struct B {
  int y;
  A a;

  B () {
    cout << "initialize B ..." <<endl;
    y = 2;
  }


  void show() {
    cout <<  "B: y = " << y << ", A: x=" << a.x << endl;
  }

  ~ B () {
    cout << "destruct B ...  y = "<< y << endl;
  }

};


A f( B& b) {
  //b.a.x = 3;
  //return std::move(b.a);

  return b.a;
}

int main()
{
  B b;

  b.show();

  //A a = f(std::move(b));
  A a = f(b);
 
  a.show();
  a.x = 5;
  a.show();

  b.show();

  return 0;
}
