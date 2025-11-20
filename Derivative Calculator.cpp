#include <iostream>
using namespace std;
int main()
{
	double* power;
	int size;
	double* coefficient;
	cout << "Welcome to derivative calculator\n";
	cout << "Enter number of variables in polynomial function ";
	cin >> size;
	power = new double[size];
	coefficient = new double[size];
	for (int i = 0; i < size; ++i)
	{
		cout << "Enter power of variable " << i + 1 << endl;
		cin >> power[i];
		cout << "Enter co-efficient of variable " << i + 1 << endl;
		cin >> coefficient[i];
	}
	cout << "Polynomial function :\nXn + Xn-1 .... X1 + Xo \n";
	cout << "Entered polynomial function is : \n";
	for (int i = 0; i < size; ++i)
	{
		cout << "( " << coefficient[i] << " x^ " << power[i] << " ) + ";
	}
	cout << endl;
	cout << endl;
	cout << endl;
	cout << "\n\nDerivative of polynomial function is : \n";
	cout << endl;
	for (int i = 0; i < size; ++i)
	{
		cout << "( " << coefficient[i] * power[i] << " x^ " << power[i] - 1 << " ) +";
	}
	cout << endl;
	system("pause");
}
