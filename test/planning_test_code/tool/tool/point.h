#pragma once
#include <vector> 
#include <list> 

class Point
{
public:
	int x, y; //�����꣬����Ϊ�˷��㰴��C++�����������㣬x������ţ�y�������� 
	double F, G, H; //F=G+H 
	Point *parent; //parent�����꣬����û����ָ�룬�Ӷ��򻯴��� 
	Point(int _x, int _y) :x(_x), y(_y), F(0), G(0), H(0), parent(NULL)  //������ʼ�� 
	{
	}
};