#include <iostream>
#include <string>
#include "MyDLL.h"
using namespace std; 

void Main() {
	int a, b;
	cout << "input a and b:";
	cin >> a;
	cin >> b;

	int result = Add(a, b);
	std::cout << result << std::endl;
}