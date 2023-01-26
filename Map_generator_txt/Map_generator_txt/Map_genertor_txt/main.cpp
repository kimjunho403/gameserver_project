#include <fstream>
#include <iostream>
#include <array>

using namespace std;
 constexpr int MAX_OBSTACLE = 10000;
//struct Obstacle
//{
//	static const int MAX_OBSTACLE = 10000;
//public:
//	int _id;
//	short _x;
//	short _y;
//};
//
//array < Obstacle, Obstacle::MAX_OBSTACLE > obsatcles;
int main()
{
	ofstream os;
	os.open("map.bin",ios::binary);
	
	if (os)
	{
		int i = 0;
		for (int x = 500; x < 1500; x++) {
			for (int y = 500; y < 1500; y++) {
				if (rand() % 50 == 1 && i <= MAX_OBSTACLE) {
					os << "ID " << i << "\n";
					os << "x " << x << "\n";
					os << "y " << y << "\n";
					i++;
				}
			}
		}
	}
	os.close();

	/*ifstream is("map.bin", ios::binary);
	int i = 0;
	string ignore;
	
	for (int x = 500; x < 1500; x++) {
		for (int y = 500; y < 1500; y++) {
			if (rand() % 50 == 1 && i <= Obstacle::MAX_OBSTACLE) {
				int a=0;
				int b=0;
				int c=0;
				is >> ignore >> a ;
				is >> ignore >> b ;
				is >> ignore >> c;
				cout << a << " " << b << " " << c <<endl ;
				i++;
			}
		}
	}
	is.close();*/

}