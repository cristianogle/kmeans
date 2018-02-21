#include "KMeans.h"
#include <iostream>
#include <stdlib.h>
#include <sys/time.h>
#include <fstream>
#include <omp.h>

int main(int argc, char*argv[]) {
	int num_points = 100000, k = 150;


	std::vector<Point> dataset;
	std::ifstream infile("points.txt");
	int x, y;
	for( int i = 0; i < num_points; i++ ) {
		infile >> x >> y;
		Point p(i, x, y);
		dataset.push_back(p);
		//std::cout << "Created point (" << x << ", " << y << ")" << std::endl;
	}

	// Serial version of K-Means
	timeval t1, t2;
	gettimeofday(&t1, NULL);
	KMeans kmeans_s;
	kmeans_s.serial_kmeans(k, dataset);
	gettimeofday(&t2, NULL);
	double elapsedTimeMs = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
    elapsedTimeMs += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
	std::cout << "Time elapsed (serial version): " << elapsedTimeMs << " mS" << std::endl;

	std::cout << "-----------------------------------------------" << std::endl;

	// Parallel version of K-Means
	KMeans kmeans_p;
	gettimeofday(&t1, NULL);
	std::vector<Point> res = kmeans_p.openmp_kmeans(k, dataset);
	gettimeofday(&t2, NULL);
	elapsedTimeMs = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
	elapsedTimeMs += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
	std::cout << "Time elapsed (parallel version): " << elapsedTimeMs << " mS" << std::endl;

	//for(int i = 0; i< res.size(); i++ )
		//std::cout << "point " << i <<" cluster: " << res[i].getCluster() << std::endl;
	// clustering representation
	kmeans_p.drawResults(res);

	return 0;
}
