#include <string>
#include <math.h>

class Point {
public:
	Point(int, int, int);
	Point(int, int);
	void setCluster(int);

	int getID();
	int getCluster();
	int getX();
	int getY();
	void setX(int);
	void setY(int);
	double getDistance(Point);

private:
	int id_point;
	int x;
	int y;
	int id_cluster;
};
