#include "Point.h"

Point::Point(int id, int x, int y) {
	id_point = id;
	this->x = x;
	this->y = y;

	id_cluster = -1;
}

Point::Point(int x, int y) {
	this->id_point = -1;
	this->x = x;
	this->y = y;

	id_cluster = -1;
}

void Point::setCluster(int id_cluster) {
	this->id_cluster = id_cluster;
}

int Point::getID() { return id_point;  }

int Point::getCluster() { return id_cluster; }

int Point::getX() { return x; }

int Point::getY() { return y; }

void Point::setX(int x) { this->x = x; }

void Point::setY(int y) { this->y = y; }

double Point::getDistance(Point other) {
        return sqrt(pow(this->x - other.getX(), 2)+ pow(this->y - other.getY(), 2));
}
