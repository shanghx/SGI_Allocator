#include"Alloc.h"

int main()
{
	try
	{
		test2();
	}
	catch (exception& e)
	{
		cout << e.what() << endl;
	}
	
	system("pause");
	return 0;
}
