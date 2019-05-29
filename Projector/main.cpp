/*
DLPC350 投影仪类测试文件
*/
#include "Projector.h"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace std;

int main()
{
	Projector proj;
	string Log;
	while (true)
	{
		if (proj.Discover_Timer_Call())
		{
			cout << proj.getLog();
			break;
		}
		else
		{
			cout << "No projector aviliable...Waitting..." << endl;
			cout << proj.getLog();
		}
		this_thread::sleep_for(chrono::seconds(1));
	}
	proj.exposure_time = 10000;
	proj.period_time = 10000;
	//time must set before Projector init.(will use it)
	proj.Projector_Init("test.ini");
	cout << proj.getLog();

	proj.Play();
	system("pause");
	proj.Stop();
	system("pause");
}