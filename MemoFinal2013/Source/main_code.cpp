// ----------------------------------------------------------------------------------------
//    Functional Software Reference for MEMOCODE 2013 Design Contest 
//
//    This code was taken from nghiaho.com/?page_id=1366
//
//    It has been modified for MEMOCODE 2013 design contest. 
//    It reads in data cost from vdata_in.txt, and writes resulting 
//    depth labels to output_labels.txt
//
// ----------------------------------------------------------------------------------------

#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <omp.h>
#include <string>

using namespace std;

enum DIRECTION {LEFT, RIGHT, UP, DOWN, DATA};

typedef unsigned int TYPE;

const unsigned int  MAX_NUM = -1;

const int BP_ITERATIONS = 30;
const int LABELS = 16;
const int LAMBDA = 8;
const int SMOOTHNESS_TRUNC = 2;
const int BORDER_SZ = 18;
const int LEVELS = 2;
const int LSCALE = 1;
const int wdiv = 1;

struct Vertex {
	// Each vertex has 4 messages from its 
	// right/left/up/down edges and a data cost. 
	TYPE msg[5][LABELS];
};

struct MRF2D {
	std::vector <Vertex> grid;
};

int width, height;
int width2, height2;

vector<TYPE> reg;
vector<TYPE> best_as;

void InitGraph(const std::string &vdata_file, MRF2D &mrf); 
void WriteResults(const std::string &edata_file); 
TYPE SmoothnessCost(int i, int j);
void BP(MRF2D &mrf, DIRECTION direction, int ITER);
void SendMsg(MRF2D &mrf, int x, int y, DIRECTION direction);
TYPE MAP(MRF2D &mrf);

int main(int argv, char** argc) {
	if(argv <= 2){
		cout << "Usage: ./bin [input.txt] [output.txt] "<<endl;
			exit(-1);
	}
	string input_file_name = argc[1];
	string output_file_name = argc[2];
	//	LABELS = atoi(argc[1]);
	//	unsigned int a = 0;
	MRF2D mrf;
	//PYRAMID* pyramid = new PYRAMID();

	InitGraph(input_file_name, mrf);

	// ------------------------------------------------------
	// FOR THE CONTEST, START IMPLEMENTATION HERE 
	// This loop is where LBP is performed.
	// Runtime measurement starts here 
	struct timeval t1,t2;
	gettimeofday(&t1, NULL);

#pragma omp parallel for	
	for (int j=0; j < height; j++){
		for (int k=0; k < width; k++){
			for (int l=0; l < LABELS; l++){
				mrf.grid[(j/LSCALE)*(width/LSCALE) + k/LSCALE].msg[DATA][l] += reg[(j*width + k)*LABELS + l]/wdiv; 
			}
		}

	}
	__builtin_prefetch(&mrf);
	for(int j=0; j <  BP_ITERATIONS; j++) {
		BP(mrf, RIGHT, j);
		BP(mrf, DOWN, j);
	}

gettimeofday(&t2,NULL);
cout<< (((t2.tv_sec -t1.tv_sec)*1000.0)+((t2.tv_usec - t1.tv_usec)/1000.0))<<" ms"<<endl;
// Runtime measurement ends here 

// FOR THE CONTEST, END IMPLEMENTATION HERE 
// ------------------------------------------------------

// Assign labels 
TYPE energy = MAP(mrf);
cout <<  "energy = " << energy << endl;

WriteResults(output_file_name);
return 0;
}

//void InitGraph(const std::string &vdata_file, PYRAMID* pyramid) {
void InitGraph(const std::string &vdata_file, MRF2D &mrf) {
	FILE *fp;
	//int width, height;

	// Open File
	if( ( fp = fopen( vdata_file.c_str(), "r" ) ) == NULL ) {
		printf("Can't open file %s\n", vdata_file.c_str());
		assert(0);
	} 

	fscanf(fp, "%d", &width);
	fscanf(fp, "%d", &height);
	int gridsize = width * height;
	//width of Compressed layer
	width2 = width/LSCALE + width%LSCALE;
	//height of Compressed layer
	height2 = height/LSCALE + height%LSCALE;

	mrf.grid.resize(width2 * height2);

	for(int i=0; i < mrf.grid.size(); i++) {
		for(int j=0; j < 5; j++) {
			for(int k=0; k < LABELS; k++) {
				mrf.grid[i].msg[j][k] = 0;
			}
		}
	}

	reg.resize(gridsize * LABELS);

	// Initialize vertex data (Data Cost) from given file (layers[0])
	for(int vid=0; vid < gridsize ; vid++) {
		for(int l=0; l < LABELS; l++) {
			fscanf(fp, "%d", &reg[vid*LABELS + l]);
		}
	}


	fclose(fp);

}

void WriteResults(const std::string &edata_file) {
	FILE *fp;

	// Open File
	if( ( fp = fopen( edata_file.c_str(), "w" ) ) == NULL ) {
		printf("Can't open file %s\n", edata_file.c_str());
		assert(0);
	} 

	// First line, write number of entries in file
	fprintf(fp, "%d\n", (width-(BORDER_SZ*2))*(height-(BORDER_SZ*2)));

	// Write label assignments 
	for(int y=BORDER_SZ; y < height-BORDER_SZ; y++) {
		for(int x=BORDER_SZ; x < width-BORDER_SZ; x++) { 
			fprintf(fp, "%d\n", best_as[y*width+x]);
		}
	}

	fclose(fp);

	}


	TYPE SmoothnessCost(int i, int j)
	{
		int d = i - j;

		return LAMBDA*std::min(abs(d), SMOOTHNESS_TRUNC);
	}

	void SendMsgU(MRF2D &mrf, int x, int y, DIRECTION direction)
	{
		TYPE new_msgs;
		TYPE mp[LABELS]={0};

		int width = width2;
		const int pos = y*width2+x;

		TYPE min_h = MAX_NUM;

		TYPE p =  mrf.grid[pos].msg[DATA][0];
		p += mrf.grid[pos].msg[LEFT][0];
		p += mrf.grid[pos].msg[RIGHT][0];
		p += mrf.grid[pos].msg[DOWN][0];

		mp[0] = p;
		min_h = std::min(p,min_h);
		TYPE temps = p;
		TYPE mp_temp = 0;
#pragma unroll
		for(int i=1; i < LABELS; i++) {

			TYPE p =  mrf.grid[pos].msg[DATA][i]; 
			p += mrf.grid[pos].msg[LEFT][i];
			p += mrf.grid[pos].msg[RIGHT][i];
			p += mrf.grid[pos].msg[DOWN][i];

			min_h = std::min(p,min_h);
			mp_temp = std::min(p,temps+LAMBDA);
			mp[i] = mp_temp;
			temps =p;
		}

		new_msgs = std::min(mp_temp , min_h + 2*LAMBDA);

		mrf.grid[(y-1)*width + x].msg[DOWN][LABELS-1] = new_msgs;

#pragma unroll
		for(int i=LABELS-2 ; i >=0 ; i--) {
			TYPE q = mp[i];
			q = std::min(q,mp_temp+LAMBDA);
			new_msgs = std::min(q,min_h + 2*LAMBDA);
			mrf.grid[(y-1)*width + x].msg[DOWN][i] = new_msgs;
			mp_temp = q;
		}
	}

	void SendMsgD(MRF2D &mrf, int x, int y, DIRECTION direction)
	{
		TYPE new_msgs;
		TYPE mp[LABELS]={0};

		const int pos = y*width2+x;

		int width = width2;

		TYPE min_h = MAX_NUM;

		TYPE p =  mrf.grid[pos].msg[DATA][0];
		p += mrf.grid[pos].msg[LEFT][0];
		p += mrf.grid[pos].msg[RIGHT][0];
		p += mrf.grid[pos].msg[UP][0];

		mp[0] = p;
		min_h = std::min(p,min_h);
		TYPE temps = p;
		TYPE mp_temp = 0;
#pragma unroll
		for(int i=1; i < LABELS; i++) {

			TYPE p =  mrf.grid[pos].msg[DATA][i];
			p += mrf.grid[pos].msg[LEFT][i];
			p += mrf.grid[pos].msg[RIGHT][i];
			p += mrf.grid[pos].msg[UP][i];

			min_h = std::min(p,min_h);
			mp_temp = std::min(p,temps+LAMBDA);
			mp[i] = mp_temp;
			temps =p;
		}//initialization

		new_msgs = std::min(mp_temp,min_h + 2*LAMBDA);

		mrf.grid[(y+1)*width + x].msg[UP][LABELS-1] = new_msgs;

#pragma unroll
		for(int i=LABELS-2 ; i >=0 ; i--) {
			TYPE q = mp[i];
			q = std::min(q,mp_temp+LAMBDA);
			new_msgs = std::min(q,min_h + 2*LAMBDA);
			mrf.grid[(y+1)*width + x].msg[UP][i] = new_msgs;
			mp_temp = q;
		}
	}

	void SendMsgL(MRF2D &mrf, int x, int y, DIRECTION direction)
	{
		TYPE new_msgs;
		TYPE mp[LABELS]={0};

		const int pos = y*width2+x;

		int width = width2;

		TYPE min_h = MAX_NUM;

		TYPE p =  mrf.grid[pos].msg[DATA][0]; // Data Cost
		p += mrf.grid[pos].msg[UP][0];
		p += mrf.grid[pos].msg[RIGHT][0];
		p += mrf.grid[pos].msg[DOWN][0];

		mp[0] = p;
		min_h = std::min(p,min_h);
		TYPE temps = p;
#pragma unroll
		for(int i=1; i < LABELS; i++) {

			TYPE p =  mrf.grid[pos].msg[DATA][i]; // Data Cost
			p += mrf.grid[pos].msg[UP][i];
			p += mrf.grid[pos].msg[RIGHT][i];
			p += mrf.grid[pos].msg[DOWN][i];

			min_h = std::min(p,min_h);
			mp[i] = std::min(p,temps+LAMBDA);
			temps =p;
		}//initialization

		new_msgs = std::min(mp[LABELS-1],min_h + 2*LAMBDA);

		mrf.grid[y*width + x-1].msg[RIGHT][LABELS-1] = new_msgs;

#pragma unroll
		for(int i=LABELS-2 ; i >=0 ; i--) {
			mp[i] = std::min(mp[i],mp[i+1]+LAMBDA);
			new_msgs = std::min(mp[i],min_h + 2*LAMBDA);
			mrf.grid[y*width + x-1].msg[RIGHT][i] = new_msgs;
		}
	}

	void SendMsgR(MRF2D &mrf, int x, int y, DIRECTION direction)
	{
		TYPE new_msgs;
		TYPE mp[LABELS]={0};
	
		const int pos = y*width2+x;
		int width = width2;
		TYPE min_h = MAX_NUM;

		TYPE p =  mrf.grid[pos].msg[DATA][0]; // Data Cost
		p += mrf.grid[pos].msg[LEFT][0];
		p += mrf.grid[pos].msg[UP][0];
		p += mrf.grid[pos].msg[DOWN][0];

		mp[0] = p;
		min_h = std::min(p,min_h);
		TYPE temps = p;
#pragma unroll
		for(int i=1; i < LABELS; i++) {
			
			TYPE p =  mrf.grid[pos].msg[DATA][i]; // Data Cost
			p += mrf.grid[pos].msg[LEFT][i];
			p += mrf.grid[pos].msg[UP][i];
			p += mrf.grid[pos].msg[DOWN][i];

			min_h = std::min(p,min_h);
			mp[i] = std::min(p,temps+LAMBDA);
			temps =p;

		}//initialization

		new_msgs = std::min(mp[LABELS-1],min_h + 2*LAMBDA);

		mrf.grid[y*width + x+1].msg[LEFT][LABELS-1] = new_msgs;
#pragma unroll
		for(int i=LABELS-2 ; i >=0 ; i--) {
			mp[i] = std::min(mp[i],mp[i+1]+LAMBDA);
			new_msgs = std::min(mp[i],min_h + 2*LAMBDA);
			mrf.grid[y*width + x+1].msg[LEFT][i] = new_msgs;
		}
	}

	void BP(MRF2D &mrf, DIRECTION direction, int ITER) 
	{
		int width = width2;
		int height = height2;

		switch(direction) {
			case RIGHT:
#pragma omp parallel for
				for(int y=0; y < height; y++) {
					for(int x=(ITER%2); x < width-1; x+=2) {
						int xbar = width - x - 1;
						SendMsgR(mrf, x, y, direction);
						SendMsgL(mrf, xbar, y, LEFT);
					}
				}
				break;

			case DOWN:
#pragma omp parallel for
				for(int x=0; x < width; x++) {
					for(int y=(ITER%2); y < height-1; y+=2) {
						int ybar = height - y - 1;
						SendMsgD(mrf, x, y, direction);
						SendMsgU(mrf, x, ybar, direction);
					}
				}
				break;

		}
	}

	TYPE MAP(MRF2D &mrf)
	{
		// Finds the MAP assignment as well as calculating the energy

		// MAP assignment
		best_as.resize(width*height);
		for(size_t i=0; i < height; i++){
			for(size_t j=0; j < width; j++) {
				TYPE best = MAX_NUM; 
				for(int k=0; k < LABELS; k++) {
					TYPE cost = 0;

					cost += mrf.grid[((i/LSCALE)*(width/LSCALE)) + j/LSCALE].msg[LEFT][k];
					cost += mrf.grid[((i/LSCALE)*(width/LSCALE)) + j/LSCALE].msg[RIGHT][k];
					cost += mrf.grid[((i/LSCALE)*(width/LSCALE)) + j/LSCALE].msg[UP][k];
					cost += mrf.grid[((i/LSCALE)*(width/LSCALE)) + j/LSCALE].msg[DOWN][k];

					cost += (reg[((i*width) + j)*LABELS+k]);

					if(cost < best) {
						best = cost;
						best_as[i*width + j] = k;
					}
				}
			}
		}

		// Energy
		TYPE energy = 0;

		for(int y=0; y < height; y++) {
			for(int x=0; x < width; x++) {

				int cur_label = best_as[y*width+x];

				// Data cost

				energy += (reg[(y * width + x) * LABELS + cur_label]);


				if(x-1 >= 0)     energy += SmoothnessCost(cur_label, best_as[y*width+x-1]);
				if(x+1 < width)  energy += SmoothnessCost(cur_label, best_as[y*width+x+1]);
				if(y-1 >= 0)     energy += SmoothnessCost(cur_label, best_as[(y-1)*width+x]);
				if(y+1 < height) energy += SmoothnessCost(cur_label, best_as[(y+1)*width+x]);
			}
		}

		return energy;
		}
