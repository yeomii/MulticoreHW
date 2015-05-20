#define CLASS 64
typedef struct Point {
    float x, y;
} Point;

__kernel void kmeans(__global const Point* data,
                    __global Point* centroids,
                    __global int* partition,
                    __global int* counts,
                    int data_n,
                    int class_n)
{
  int gid = get_group_id(0);
  int lid = get_local_id(0);
  int ws = get_local_size(0);
  int pid = get_global_id(0);
 
  float min_dist = 100000000000000.0;
  int part;
  for (int ci = 0; ci < class_n; ++ci)
  {
    Point t;
    t.x = data[pid].x - centroids[ci].x;
    t.y = data[pid].y - centroids[ci].y;
    float dist = t.x*t.x + t.y*t.y;
    if (dist < min_dist)
    {
      part = ci;
      min_dist = dist;
    }
  }
  partition[pid] = part;
}
}