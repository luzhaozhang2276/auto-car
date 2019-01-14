#pragma once

/*
//A*�㷨������
*/
#include<iostream>
#include <vector> 
#include <list> 
#include "BMP.h"
#include "point.h"

const double kCost1 = 10; //ֱ��һ������ 
const double kCost2 = 14; //б��һ������ 
const double kG = 5; //
const double kH = 1; //����




class Astar
{
public:
	void InitAstar(std::vector<std::vector<int>> &_maze);
	std::list<Point *> GetPath(Point &startPoint, Point &endPoint, bool isIgnoreCorner);

	void setBMP(BMP* pImg){ img = pImg; };
	BMP* img;

	static void show_path(std::list<Point *>& path, BMP* img, char color='r');

private:
	Point *findPath(Point &startPoint, Point &endPoint, bool isIgnoreCorner);
	std::vector<Point *> getSurroundPoints(const Point *point, bool isIgnoreCorner) const;
	bool isCanreach(const Point *point, const Point *target, bool isIgnoreCorner) const; //�ж�ĳ���Ƿ����������һ���ж� 
	Point *isInList(const std::list<Point *> &list, const Point *point) const; //�жϿ���/�ر��б����Ƿ����ĳ�� 
	Point *getLeastFpoint(); //�ӿ����б��з���Fֵ��С�Ľڵ� 
	//����FGHֵ 
	int calcG(Point *temp_start, Point *point);
	int calcH(Point *point, Point *end);
	int calcF(Point *point);
private:
	std::vector<std::vector<int>> maze;
	std::list<Point *> openList;  //�����б� 
	std::list<Point *> closeList; //�ر��б� 
};