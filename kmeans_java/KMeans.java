import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import javax.swing.JFrame;
import javax.swing.JPanel;

public class KMeans extends JPanel {
	private int k;
	private int numPoints;
	private static final int NUM_THREADS = 4;
	private List<Point> dataset;
	
	public KMeans(int k, int n) {
		this.k = k;
		this.numPoints = n;
		dataset = new ArrayList<>();
	}	
	/* used to move results on the chart */
	public KMeans(List<Point> dataset2, int k2) {
		this.k = k2;
		this.numPoints = dataset2.size();
		dataset = new ArrayList<>();
		for(int i = 0; i < dataset2.size(); i++) {
			this.dataset.add(dataset2.get(i));
		}
	}
	
	public static List<Point> initDataset(int n) {
		List<Point> dataset = new ArrayList<>();
		try {
			BufferedReader br = new BufferedReader(new FileReader("points.txt"));
			String line;
			int i = 0;
		    while ( i < n && (line = br.readLine()) != null) {
		    	String[] splitted = line.split("\\s+");
		    	int x = Integer.parseInt(splitted[0]);
		    	int y = Integer.parseInt(splitted[1]);
		    	Point point = new Point(i, x, y);
				dataset.add(point);
		    	i++;
		    }
		    br.close();	    
		} catch (IOException e) {
			e.printStackTrace();
		}
		return dataset;
	}
	
	public static List<Point> initCenters(int k, List<Point> dataset) {
		List<Point> centers = new ArrayList<>(k);
		for(int i = 0; i < k; i++ ) {
			dataset.get(i).setCluster(i);
			centers.add(dataset.get(i));
			//System.out.println("Cluster "+i+": "+centers.get(i).toString());
		}	
		return centers;
	}
	
	public static int getNearestClusterId(Point p, List<Point> centers) {
		int index = 0;
		double minDist = p.getDistance(centers.get(0));
		for (int i = 1; i < centers.size(); i++) {
			double dist = p.getDistance(centers.get(i));
			if (dist < minDist) {
				minDist = dist;
				index = i;
         	}
		}
		return index;
	}
	  
	private static <V> List<List<V>> partition(List<V> list, int parts) {
		List<List<V>> lists = new ArrayList<List<V>>(parts);
	    for (int i = 0; i < parts; i++) {
	        lists.add(new ArrayList<V>());
	    }
	    for (int i = 0; i < list.size(); i++) {
	        lists.get(i % parts).add(list.get(i));
	    }
	    return lists;
	}
	
	public List<Point> serialKMeans() {
		List<Point> dataset = KMeans.initDataset(numPoints);
		List<Point> centers = KMeans.initCenters(k, dataset);
		List<Point> newCenters;
	    double dist;
	    do {
	    	newCenters = getNewCenters(dataset, centers);
	    	dist = getDistance(centers, newCenters);
	    	centers = newCenters;
	    } while(dist != 0);		/*The algorithm converge when dist = 0 */
	    // show results
	    /*for( int i = 0; i < k; i++ ) {
	    	System.out.println("Cluster "+i+": "+newCenters.get(i).toString());
	    }*/
	    return dataset;
	}

	public List<Point> parallelKMeans() {
		List<Point> dataset = KMeans.initDataset(numPoints);
		List<Point> centers = KMeans.initCenters(k, dataset);
		List<Point> newCenters;
		double dist;
	    do {
	    	newCenters = parallelGetNewCenters(dataset, centers);
	        dist = getDistance(centers, newCenters);
	        centers = newCenters;
	    } while (dist != 0);
	    // show results
	    /*for( int i = 0; i < k; i++ ) {
	    	System.out.println("Cluster "+i+": "+newCenters.get(i).toString());
	    }*/
	    return dataset;
	}

	public static List<Point> getNewCenters(List<Point> dataset, List<Point> centers) {
		List<List<Point>> clusters = new ArrayList<>(centers.size());
		for (int i = 0; i < centers.size(); i++) {
	          clusters.add(new ArrayList<Point>());
	    }
		for (Point data : dataset) {
			int nearestClusterId = getNearestClusterId(data, centers);
			clusters.get(nearestClusterId).add(data);
			if(data.getCluster() != nearestClusterId) {
				//System.out.println("point "+data.getID()+" moved from " + data.getCluster()+" to "+nearestClusterId);
				data.setCluster(nearestClusterId);	
			}
		}
		List<Point> newCenters = new ArrayList<>(centers.size());
		for (List<Point> c : clusters) {
			newCenters.add(Point.getMean(c));
	    }
	    return newCenters;
	}
	
	public static List<Point> parallelGetNewCenters(List<Point> dataset, List<Point> centers) {
		List<List<Point>> clusters = new ArrayList<>(centers.size());
		for (int i = 0; i < centers.size(); i++) {
			clusters.add(new ArrayList<Point>());
	    }
		List<List<Point>> partitionedDataset = partition(dataset, NUM_THREADS);
		ExecutorService executor = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors());
	    List<Callable<Void>> workers = new ArrayList<>();
	    for (int i = 0; i < NUM_THREADS; i++) {
	    	workers.add(createWorker(partitionedDataset.get(i), centers, clusters));
	    }
	    try {
	    	executor.invokeAll(workers);
	    } catch (InterruptedException e) {
	    	e.printStackTrace();
	        System.exit(-1);
	    }
	    List<Point> newCenters = new ArrayList<>(centers.size());
	    for (List<Point> cluster : clusters) {
	    	newCenters.add(Point.getMean(cluster));
	    }
	    return newCenters;
	}
	
	private static Callable<Void> createWorker(final List<Point> partition, final List<Point> centers,
	          final List<List<Point>> clusters) {
		return new Callable<Void>() {
			@Override
	        public Void call() throws Exception {
				int indexes[] = new int[partition.size()];
	            for (int i = 0; i < partition.size(); i++) {
	            	Point data = partition.get(i);
	            	int nearestClusterId = getNearestClusterId(data, centers);
	                indexes[i] = nearestClusterId;
	            }
	            synchronized (clusters) {
	            	for (int i = 0; i < indexes.length; i++) {
	            		if(partition.get(i).getCluster() != indexes[i]) {
	        				//System.out.println("point "+partition.get(i).getID()+" moved from " + partition.get(i).getCluster()+" to "+indexes[i]);
	        				partition.get(i).setCluster(indexes[i]);
	            		}
	            		clusters.get(indexes[i]).add(partition.get(i));
	                }    
	            }
	            return null;
			}
		};
	}
	
	public static double getDistance(List<Point> oldCenters, List<Point> newCenters) {
		double sum = 0;
	    for (int i = 0; i < oldCenters.size(); i++) {
	    	double dist = oldCenters.get(i).getDistance(newCenters.get(i));
	        sum += dist;
	    }
	    return sum;
	}
	
	@Override
	public void paintComponent(Graphics g) {
		super.paintComponent(g);
		
		Graphics2D g2d = (Graphics2D) g;
		int borderGap = 30, maxX = 1000, maxY = 1000;
		double xScale = ((double) getWidth() - 2 * borderGap) / (maxX-1);
	    double yScale = ((double) getHeight() - 2 * borderGap) / (maxY-1);
	    
	    g2d.setColor(Color.black);
	    // create x and y axes 
	    g2d.drawLine(borderGap, getHeight() - borderGap, borderGap, borderGap);
	    g2d.drawLine(borderGap, getHeight() - borderGap, getWidth() - borderGap, getHeight() - borderGap);

	      
		Color[] colors = {Color.black, Color.blue, Color.red, Color.green, Color.cyan,
				Color.darkGray, Color.gray,	Color.lightGray, Color.magenta, Color.orange, Color.pink, 
				Color.yellow};
		
		for(int i = 0; i < dataset.size(); i++ ) {
			g2d.setColor(colors[dataset.get(i).getCluster()%colors.length]);
			int x = (int)(dataset.get(i).getX()*xScale+borderGap);
			int y = (int)(getHeight()-dataset.get(i).getY()*yScale-borderGap-5);
			g2d.fillOval(x, y, 7, 7);
		}
	}
	
	public static void main(String[] args ) {
		int k = 150;
		int numPoints = 100000;
		List<Point> res;
		
		KMeans km1 = new KMeans(k, numPoints);
		System.out.println("Serial version...");
		long start = System.currentTimeMillis();
		res = km1.serialKMeans();
	    System.out.println("Time elapsed: " + (System.currentTimeMillis() - start) + "ms");
	    
	    KMeans km2 = new KMeans(k, numPoints);
	    System.out.println("Parallel version...");
	    start = System.currentTimeMillis();
	    km2.parallelKMeans();
	    System.out.println("Time elapsed: " + (System.currentTimeMillis() - start) + "ms");
		
	    JFrame frame = new JFrame("KMeans Clustering with n="+numPoints+", k="+k);
	    frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	    frame.setSize(800, 600);
	    KMeans km = new KMeans(res, k);
	    frame.add(km);
	    frame.setVisible(true);
	    
	    //System.exit(0);
	}
}