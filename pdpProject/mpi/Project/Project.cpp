#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
 #include <mpi.h>
using namespace std;

int N;
string comment = "";

char** adj; // adjacency matrix
char* adj_matrix; // actual store
bool** colors; // color classes
bool** free_color; // free color classes of nodes
int* sat; // saturation of a node
int* free_num; // number of free color classes for nodes
bool* OK; // is the node ready
int num_color;
vector<int> perm; // the node list of permutation
int startval, endval;

void initialize();
void memory_init();
void permutation_color();
void permto();
void testperm();
void print_colors();
void construct_graph();
void starting_point();
void write_matrix();

int main(int argc, char** argv) {
   int i, j, min_free, min_sat, min_id;
   int c = 0;
   int id, nproc, id_from;
   int sv, ev;
   string file_name = "input.txt";
   MPI_Status status;
	   // Initialize MPI:
   MPI_Init(&argc, &argv);
	   // Get my rank:
   MPI_Comm_rank(MPI_COMM_WORLD, &id);
	   // Get the total number of processors:
   MPI_Comm_size(MPI_COMM_WORLD, &nproc);
   if (id == 0) {
		//read_clq(file_name);
	   construct_graph();
   }
   MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
   if (id != 0) {
		memory_init();
   }
   startval = N * id / nproc;
   endval = N * (id + 1) / nproc;
   
   MPI_Bcast(adj_matrix, N * N, MPI_CHAR, 0, MPI_COMM_WORLD);
   
   initialize();
	
	   //we color N nodes each after other:
   for (int v = 0; v < N; ++v) {
	   min_free = N;
	   min_sat = N;
	   min_id = -1;
		     //find node of minimum freedom:
	   for (i = startval; i < endval; ++i)
		   if (!OK[i] && (min_free > free_num[i] ||
				 		    min_free == free_num[i] && sat[i] < min_sat)) {
			   min_free = free_num[i];
			   min_sat = sat[i];
			   min_id = i;
		   }
	   struct {
		   int free;
		   int id;
	   } p, tmp_p;
	   p.id = min_id;
	   p.free = min_free;
		     //not optimal,
		     //we would've need to look for minimum of free and sat together
	   MPI_Allreduce(&p, &tmp_p, 1, MPI_2INT, MPI_MINLOC, MPI_COMM_WORLD);
	   
	   min_id = tmp_p.id;
	   min_free = tmp_p.free;
	   
	   OK[min_id] = 1; //ready to color
		    //color it:
	   if (min_free == 0) {
			       //We need a new color class.
		   colors[num_color][min_id] = 1;
			       //the new color class posible class for the rest:
		   for (i = startval; i < endval; ++i)
			   if (!OK[i]) {
				   free_color[i][num_color] = 1;
				   ++free_num[i];
			   }
			       //but not for the connected nodes:
		   for (i = startval; i < endval; ++i) {
			   if (!OK[i] && adj[i][min_id]) {
					free_color[i][num_color] = 0;
					--free_num[i];
					}
			}
		   ++num_color;
		} else {
			       //We put node into an old color class.
			       //find the class:
		   int id_from;
		   for (id_from = 0; id_from < nproc; ++id_from) {
			   if (N * id_from / nproc <= min_id && min_id < N * (id_from + 1) / nproc) break;
			   if (id == id_from) {
				   for (c = 0; !free_color[min_id][c]; ++c);
				   MPI_Bcast(&c, 1, MPI_INT, id_from, MPI_COMM_WORLD);
				   colors[c][min_id] = 1;
				   //the connected nodes' freedom decreases:
				   for (i = startval; i < endval; ++i) {
					   if (!OK[i] && free_color[i][c] && adj[i][min_id]) {
						   free_color[i][c] = 0;
						   --free_num[i];
					   }
				   }
				}
			}
		}
	}
   cout << "id " << id << "\n";
   cout << "number of DSATUR colors: " << num_color << endl;
   if (id == 0) {
	   permutation_color();
	   testperm();
	   permto();
	   print_colors();
	   write_matrix();
	}
	   // Terminate MPI:
   MPI_Finalize();
}

 void initialize() {
	 int sum;
	 for (int i = 0; i < N; ++i) {
		 sum = 0;
		 for (int j = 0; j < N; ++j) {
			 colors[i][j] = 0;
			 free_color[i][j] = 0;
			 sum += adj[i][j];
		 }
		 sat[i] = sum;
		 free_num[i] = 0;
		 OK[i] = 0;
	}

	 starting_point();
}

 void print_colors() {
	 for (int i = 0; i < N; i++) {
		 cout << "Color " << i << " ";
		 for (int j = 0; j < N; j++) {
			 if (colors[i][j] != 0) {
				 cout << j << ", ";
			 }
		 }
		 cout << "\n";
	 }
 }

 void memory_init() {
	 int i, j;
	 adj = new char* [N];
	 adj_matrix = new char[N * N];
	 colors = new bool* [N];
	 free_color = new bool* [N];
	 for (i = 0; i < N; i++) {
		 adj[i] = adj_matrix + (i * N);
		 colors[i] = new bool[N];
		 free_color[i] = new bool[N];
	 }
	 sat = new int[N];
	 free_num = new int[N];
	 OK = new bool[N];
	 for (i = 0; i < N; ++i)
		 for (j = 0; j < N; ++j)
			 adj[i][j] = 0;
}

 void construct_graph() {
	 N = 6;
	 memory_init();
	 adj[0][1] = adj[1][0] = 1;
	 adj[0][2] = adj[2][0] = 1;
	 adj[1][2] = adj[2][1] = 1;
	 adj[0][3] = adj[3][0] = 1;
	 adj[1][4] = adj[4][1] = 1;
	 adj[2][5] = adj[5][2] = 1;
	 adj[4][5] = adj[5][4] = 1;
	 adj[3][4] = adj[4][3] = 1;
	 adj[3][5] = adj[5][3] = 1;
 }

 void permutation_color() {
	 int i, j, k, max, maxcol, sum;
	 int* colors_sum = new int[num_color];
	 
	 for (i = 0; i < num_color; ++i) {
		 sum = 0;
		 for (j = 0; j < N; ++j)
			 sum += colors[i][j];
		 colors_sum[i] = sum;
	 }
	 for (i = 0; i < num_color; ++i) {
		 max = 0;
		 for (j = 0; j < num_color; ++j)
			 if (colors_sum[j] > max) {
				 max = colors_sum[j];
				 maxcol = j;
			 }
		 for (j = 0; j < N; ++j) {
			 if (colors[maxcol][j])
				 perm.push_back(j);
			 }
		 colors_sum[maxcol] = 0;
	}
}

void permto() {
	int i, j;
	bool** tmp = new bool* [N];
	for (i = 0; i < N; i++)
		tmp[i] = new bool[N];
	for (i = 0; i < N; i++)
		for (j = 0; j < N; j++)
			tmp[i][j] = adj[perm[i]][perm[j]];
	for (i = 0; i < N; i++)
		for (j = 0; j < N; j++)
			adj[i][j] = tmp[i][j];
}

 void testperm() {
	 int sum;
	 for (int i = 0; i < N; ++i) {
		 sum = 0;
		 for (int j = 0; j < N; ++j)
			 if (perm[j] == i) ++sum;
		 if (sum != 1) {
			 cerr << "Wrong permutation!!" << endl;
			 exit(1);
		}
	}
}

 void write_matrix() {
	 for (int i = 0; i < N; i++) {
		 for (int j = 0; j < N; j++)
			 cout << adj[i][j] << " ";
		 cout << "\n";
	 }
 }

 void starting_point() {
	 startval = 0;
	 endval = N;
 }