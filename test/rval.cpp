
#include <iostream>
#include <vector>
using namespace std;

struct A {
	int x;
	vector<int> v;

	void show(){
		cout << "A: x=" << x << ", v= ";
		for(auto x: v) cout << x << ", " ;
		cout<<endl;
	}
};

A&& f(A&& a) {
	a.x = 10;
	a.v.push_back(1);
	a.v.push_back(2);
	return std::move(a);
}

A g(A& a){
	a.x = 10;
	a.v.push_back(1);
	a.v.push_back(2);
	return std::move(a);
}

int main(){
	A a;
	a.x = 1;
	a.v.push_back(0);
	a.v.push_back(1);

	a.show();

	vector<int>&& v = std::move(a.v);
	v.push_back(2);

	a.show();

	
	vector<int> v2 = std::move(v); // v;
	v.push_back(4);
	v2.push_back(3);

	a.show();


	cout << "v2: ";
	for(auto x : v2) cout << " " << x;
	cout << endl;

}

int main2()
{
	A a;
	a.x = 1;
	a.v.push_back(0);

	a.show();

	//A a2 = f( std::move(a) );
	A a2 = g( a );
	//a2.x = 11;
	a2.v.push_back(3);


	cout << "a: " ; 
	a.show();


	cout << "a2: ";
	a2.show();


	a.v.push_back(99);
	a.show();

	return 0;
}
