#include <vector>
#include "Point.h"

class KMeans {
private:
	int k;		// number of clusters
	int num_points;
	const int NUM_THREADS = 4;

	int getNearestClusterID(Point, std::vector<Point>);
	Point getMean(std::vector<Point>);
	double getDistance(std::vector<Point>, std::vector<Point>);
public:
	std::vector<Point> serial_kmeans(int, std::vector<Point>);
	std::vector<Point> openmp_kmeans(int, std::vector<Point>);
	void drawResults(std::vector<Point>);
};
