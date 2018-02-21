#include "KMeans.h"
#include <stdlib.h>
#include <iostream>
#include <math.h>

#include <omp.h>
#include <graphics.h>

int KMeans::getNearestClusterID(Point p, std::vector<Point> centers) {
	int nearest_cluster_id = 0;
	double min_dist = p.getDistance(centers[0]);

	for( unsigned int i = 1; i < centers.size(); i++ ) {
		double dist = p.getDistance(centers[i]);

		if(dist < min_dist) {
			min_dist = dist;
			nearest_cluster_id = i;
		}
	}
	return nearest_cluster_id;
}

Point KMeans::getMean(std::vector<Point> points ) {
	int sum_x = 0;
	int sum_y = 0;
	if (points.size() == 0) {
		Point p(sum_x, sum_y);
		return p;
	}
	for(unsigned int i = 0; i < points.size(); i++ ) {
		sum_x += points[i].getX();
		sum_y += points[i].getY();
	}
	Point p((int)(sum_x / points.size()), (int)(sum_y / points.size()));
	return p;
}

std::vector<Point> KMeans::serial_kmeans(int k, std::vector<Point> dataset) {
	this->k = k;
	this->num_points = dataset.size();

	std::vector<Point> centers;
	// choose first k observations from the data set as the initial means
	for( int i = 0; i < k; i++ ) {
		dataset[i].setCluster(i);
		centers.push_back(dataset[i]);
	}
	//for(int i = 0; i < points.size(); i++ )
		//std::cout << "point " << points[i].getID() << ": [" << points[i].getX() << ", " << points[i].getY() << "]" << "--- getCluster():" << points[i].getCluster() << std::endl;

	std::vector< std::vector<Point> > clusters;
	std::vector<Point> newCenters;
	double dist;
	do {
		clusters.clear();
		newCenters.clear();

		for(unsigned int i = 0; i < centers.size(); i++ ) {
			std::vector<Point> inner_vector;
			clusters.push_back(inner_vector);
		}
		for(unsigned int i = 0; i < dataset.size(); i++ ) {
			int nearestClusterId = getNearestClusterID(dataset[i], centers);
			clusters[nearestClusterId].push_back(dataset[i]);
			if(dataset[i].getCluster() != nearestClusterId ) {
				//std::cout << "point " << points[i].getID() <<" moved from " << points[i].getCluster() << " to " << nearestClusterId << std::endl;
				dataset[i].setCluster(nearestClusterId);
			}
		}
		for(unsigned int i = 0; i < clusters.size(); i++ ) {
			newCenters.push_back(getMean(clusters[i]));
		}

		dist = getDistance(centers, newCenters);
		centers = newCenters;
	} while(dist != 0);

	// show results
	//for(int i = 0; i < k; i++)
		//std::cout<< "Cluster" << i <<": [" << newCenters[i].getX() <<", " << newCenters[i].getY() <<"]" << std::endl;

	// show dataset's current infos
	//for(int i = 0; i< dataset.size(); i++)
		//std::cout << "point " << dataset[i].getID() << ": [" << dataset[i].getX() << ", " << dataset[i].getY() << "]" << "--- getCluster():" << dataset[i].getCluster() << std::endl;
	return dataset;
}

std::vector<Point> KMeans::openmp_kmeans(int k, std::vector<Point> dataset) {
	this->k = k;
	this->num_points = dataset.size();

	std::vector<Point> centers;
	// choose first k observations from the data set as the initial means
	for( int i = 0; i < k; i++ ) {
		dataset[i].setCluster(i);
		centers.push_back(dataset[i]);
	}
	//for(int i = 0; i < points.size(); i++ )
		//std::cout << "point " << points[i].getID() << ": [" << points[i].getX() << ", " << points[i].getY() << "]" << "--- getCluster():" << points[i].getCluster() << std::endl;

	std::vector< std::vector<Point> > clusters;
	std::vector<Point> newCenters;
	int dist;
	unsigned int indexes[dataset.size()];
	do {
		dist = 0;
		clusters.clear();
		newCenters.clear();

		for(unsigned int i = 0; i < centers.size(); i++ ) {
			std::vector<Point> inner_vector;
			clusters.push_back(inner_vector);
		}
#pragma omp parallel for num_threads(NUM_THREADS) schedule(static) reduction(+:dist)
		for(unsigned int i = 0; i < dataset.size(); i++ ) {
			int nearestClusterId = getNearestClusterID(dataset[i], centers);
			indexes[i] = nearestClusterId;
			if(dataset[i].getCluster() != nearestClusterId ) {
				//std::cout << "point " << points[i].getID() <<" moved from " << points[i].getCluster() << " to " << nearestClusterId << std::endl;
				dataset[i].setCluster(nearestClusterId);
				dist++;
			}
		}
		for(unsigned int i = 0; i < dataset.size(); i++) {
			dataset[i].setCluster(indexes[i]);
			clusters[indexes[i]].push_back(dataset[i]);
		}
		for(unsigned int i = 0; i < clusters.size(); i++ ) {
			newCenters.push_back(getMean(clusters[i]));
		}
		// getDistance replaced by openmp reduction
		//dist = getDistance(centers, newCenters);
		centers = newCenters;
	} while(dist > 0);

	// show results
	//for(int i = 0; i < k; i++)
		//std::cout<< "Cluster" << i <<": [" << newCenters[i].getX() <<", " << newCenters[i].getY() <<"]" << std::endl;

	// show dataset's current infos
	//for(int i = 0; i< dataset.size(); i++)
		//std::cout << "point " << dataset[i].getID() << ": [" << dataset[i].getX() << ", " << dataset[i].getY() << "]" << "--- getCluster():" << dataset[i].getCluster() << std::endl;
	return dataset;
}




double KMeans::getDistance(std::vector<Point> oldCenters, std::vector<Point> newCenters) {
	double sum = 0;
	for (unsigned int i = 0; i < oldCenters.size(); i++) {
		double dist = oldCenters[i].getDistance(newCenters[i]);
		sum += dist;
	}
	return sum;
}

void KMeans::drawResults(std::vector<Point> points) {
	int gd = DETECT,gm;
	initgraph(&gd,&gm,NULL);
	int border_gap = 30, max_x = 1000, max_y = 1000;
	int width = getmaxx();
	int height = getmaxy();

	double x_scale = ((double) width - 2 * border_gap) / (max_x-1);
	double y_scale = ((double) height - 2 * border_gap) / (max_y-1);

	line(border_gap, height - border_gap, border_gap, border_gap);
    line(border_gap, height - border_gap, width - border_gap, height - border_gap);

    for( unsigned int i = 0; i < points.size(); i++ ) {
    	setcolor((points[i].getCluster()+1)%15);
    	int x = points[i].getX() * x_scale + border_gap;
    	int y = height - points[i].getY() * y_scale - border_gap - 5;
    	circle(x, y, 3);
    }

    getch();
	closegraph();
}

