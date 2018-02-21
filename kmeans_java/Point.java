import java.util.List;

public class Point {
	private int idPoint;
	private int x;
	private int y;
	private int idCluster;
  
	public Point(int id, int x, int y) {
		this.idPoint = id;
		this.x = x;
		this.y = y;
      
		idCluster = -1;
	}
  
	public Point(int x, int y) {
		this.idPoint = -1;
		this.x = x;
		this.y = y;
		
		idCluster = -1;
	}
	
	public double getDistance(Point other) {
        return Math.sqrt(Math.pow(this.x - other.x, 2)+ Math.pow(this.y - other.y, 2));
    }
	
	public static Point getMean(List<Point> points) {
        int sumX = 0;
        int sumY = 0;
        if (points.size() == 0) return new Point(sumX, sumY);
        for (Point point : points) {
            sumX += point.x;
            sumY += point.y;
        }
        return new Point(sumX / points.size(), sumY / points.size());
    }
  
	public void setCluster(int id) { this.idCluster = id; }
  
	public int getCluster() { return this.idCluster; }
  
	public int getID() { return this.idPoint; }
  
	public void setX(int x) { this.x = x; }
  
	public void setY(int y) { this.y = y; }
  
	public int getX() { return this.x; }
  
	public int getY() { return this.y; }
  
	@Override
	public String toString() {
		return "[x:" + this.x + ", y:" + this.y + ", pointId: "+this.idPoint 
				+ ", clusterId: "+ this.idCluster+ "]";
    }
      
	@Override
    public boolean equals(Object obj) {
        Point other = (Point) obj;
        return this.x == other.x && this.y == other.y;
	}
      
}