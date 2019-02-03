#include"Alloc.h"
#include"MyList.h"

int main()
{
	
	try
	{
		testList();
	}
	catch (exception& e)
	{
		cout << e.what() << endl;
	}
	
	system("pause");
	return 0;
}
