// ----------------------------------------------------------------------------------------
//    Functional Software Reference for MEMOCODE 2013 Design Contest 
//
//    This code was taken from nghiaho.com/?page_id=1366
//
//    It has been modified for MEMOCODE 2013 design contest. 
//    It reads in data cost from vdata_in.txt, and writes resulting 
//    depth labels to output_labels.txt
//
//    License from the original code is included below. 
// ----------------------------------------------------------------------------------------

/* 

Code is released under Simplified BSD License.

-------------------------------------------------------------------------------
Copyright 2012 Nghia Ho. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY NGHIA HO ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL NGHIA HO OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Nghia Ho.

*/ 

#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>

using namespace std;

enum DIRECTION {LEFT, RIGHT, UP, DOWN, DATA};

typedef unsigned int TYPE;

#define MAX_NUM -1 

const int BP_ITERATIONS = 40;
const int LABELS = 16;
const int LAMBDA = 16;
const int SMOOTHNESS_TRUNC = 2;
const int BORDER_SZ = 18;

struct Vertex {
   // Each vertex has 4 messages from its 
   // right/left/up/down edges and a data cost. 
   TYPE msg[5][LABELS];

   int best_assignment;
};

struct MRF2D {
    std::vector <Vertex> grid;
    int width, height;
};


void InitGraph(const std::string &vdata_file, MRF2D &mrf); 
void WriteResults(const std::string &edata_file, MRF2D &mrf); 
TYPE SmoothnessCost(int i, int j);
void BP(MRF2D &mrf, DIRECTION direction);
void SendMsg(MRF2D &mrf, int x, int y, DIRECTION direction);
TYPE MAP(MRF2D &mrf);


int main() {
    MRF2D mrf;

    InitGraph("vdata_in.txt", mrf);

    // ------------------------------------------------------
    // FOR THE CONTEST, START IMPLEMENTATION HERE 

    // This loop is where LBP is performed.
    // Runtime measurement starts here 
	struct timeval t1, t2;
	gettimeofday(&t1,NULL);

    for(int i=0; i < BP_ITERATIONS; i++) {
        cout << "Iteration: " << i << endl;

        BP(mrf, RIGHT);
        BP(mrf, LEFT);
        BP(mrf, UP);
        BP(mrf, DOWN);
    }
    // Runtime measurement ends here 

	gettimeofday(&t2,NULL);
    // FOR THE CONTEST, END IMPLEMENTATION HERE 
    // ------------------------------------------------------

    // Assign labels 
    TYPE energy = MAP(mrf);
    cout <<  "energy = " << energy << endl;

    cout<< (((t2.tv_sec -t1.tv_sec)*1000.0)+((t2.tv_usec - t1.tv_usec)/1000.0))/1000.0<<endl;
	WriteResults("output_labels.txt", mrf);

    return 0;
}

void InitGraph(const std::string &vdata_file, MRF2D &mrf) {
    FILE *fp;
    int width, height;

    // Open File
    if( ( fp = fopen( vdata_file.c_str(), "r" ) ) == NULL ) {
        printf("Can't open file %s\n", vdata_file.c_str());
        assert(0);
    } 

    fscanf(fp, "%d", &width);
    fscanf(fp, "%d", &height);

    mrf.width  = width; 
    mrf.height = height; 

    // Allocate grid of size width x height
    mrf.grid.resize(mrf.width * mrf.height);

    // Initialise edge data (messages) to zero
    for(int i=0; i < mrf.grid.size(); i++) {
        for(int j=0; j < 5; j++) {
            for(int k=0; k < LABELS; k++) {
                mrf.grid[i].msg[j][k] = 0;
            }
        }
    }

    // Initialize vertex data (Data Cost) from given file
    for(int vid=0; vid < mrf.grid.size(); vid++) {
       for(int l=0; l < LABELS; l++) {
          fscanf(fp, "%d", &mrf.grid[vid].msg[DATA][l]); 
       }
    }

    fclose(fp);
}

void WriteResults(const std::string &edata_file, MRF2D &mrf) {
    FILE *fp;

    // Open File
    if( ( fp = fopen( edata_file.c_str(), "w" ) ) == NULL ) {
        printf("Can't open file %s\n", edata_file.c_str());
        assert(0);
    } 

    // First line, write number of entries in file
    fprintf(fp, "%d\n", (mrf.width-(BORDER_SZ*2))*(mrf.height-(BORDER_SZ*2)));

     // Write label assignments 
    for(int y=BORDER_SZ; y < mrf.height-BORDER_SZ; y++) {
        for(int x=BORDER_SZ; x < mrf.width-BORDER_SZ; x++) { 
            fprintf(fp, "%d\n", mrf.grid[y*mrf.width+x].best_assignment);
        }
    }

    fclose(fp);
}


TYPE SmoothnessCost(int i, int j)
{
    int d = i - j;

    return LAMBDA*std::min(abs(d), SMOOTHNESS_TRUNC);
}

void SendMsg(MRF2D &mrf, int x, int y, DIRECTION direction)
{
    TYPE new_msg[LABELS];

    int width = mrf.width;

    for(int i=0; i < LABELS; i++) {
        TYPE min_val = MAX_NUM;

        for(int j=0; j < LABELS; j++) {
            TYPE p = 0;

            p += SmoothnessCost(i,j);              // Smoothness Cost
            p += mrf.grid[y*width+x].msg[DATA][j]; // Data Cost

            // Exclude the incoming message direction that we are sending to
            if(direction != LEFT) p += mrf.grid[y*width+x].msg[LEFT][j];
            if(direction != RIGHT) p += mrf.grid[y*width+x].msg[RIGHT][j];
            if(direction != UP) p += mrf.grid[y*width+x].msg[UP][j];
            if(direction != DOWN) p += mrf.grid[y*width+x].msg[DOWN][j];

            min_val = std::min(min_val, p);
        }

        new_msg[i] = min_val;
    }

    for(int i=0; i < LABELS; i++) {

        switch(direction) {
            case LEFT:
            mrf.grid[y*width + x-1].msg[RIGHT][i] = new_msg[i];
            break;

            case RIGHT:
            mrf.grid[y*width + x+1].msg[LEFT][i] = new_msg[i];
            break;

            case UP:
            mrf.grid[(y-1)*width + x].msg[DOWN][i] = new_msg[i];
            break;

            case DOWN:
            mrf.grid[(y+1)*width + x].msg[UP][i] = new_msg[i];
            break;

            default:
            assert(0);
            break;
        }
    }
}

void BP(MRF2D &mrf, DIRECTION direction) 
{
    int width = mrf.width;
    int height = mrf.height;

    switch(direction) {
        case RIGHT:
        for(int y=0; y < height; y++) {
            for(int x=0; x < width-1; x++) {
                SendMsg(mrf, x, y, direction);
            }
        }
        break;

        case LEFT:
        for(int y=0; y < height; y++) {
            for(int x=width-1; x >= 1; x--) {
                SendMsg(mrf, x, y, direction);
            }
        }
        break;

        case DOWN:
        for(int x=0; x < width; x++) {
            for(int y=0; y < height-1; y++) {
                SendMsg(mrf, x, y, direction);
            }
        }
        break;

        case UP:
        for(int x=0; x < width; x++) {
            for(int y=height-1; y >= 1; y--) {
                SendMsg(mrf, x, y, direction);
            }
        }
        break;

        case DATA:
        assert(0);
        break;
    }
}

TYPE MAP(MRF2D &mrf)
{
    // Finds the MAP assignment as well as calculating the energy

    // MAP assignment
    for(size_t i=0; i < mrf.grid.size(); i++) {
        TYPE best = MAX_NUM; 
        for(int j=0; j < LABELS; j++) {
            TYPE cost = 0;

            cost += mrf.grid[i].msg[LEFT][j];
            cost += mrf.grid[i].msg[RIGHT][j];
            cost += mrf.grid[i].msg[UP][j];
            cost += mrf.grid[i].msg[DOWN][j];
            cost += mrf.grid[i].msg[DATA][j];

            if(cost < best) {
                best = cost;
                mrf.grid[i].best_assignment = j;
            }
        }
    }

    int width = mrf.width;
    int height = mrf.height;

    // Energy
    TYPE energy = 0;

    for(int y=0; y < mrf.height; y++) {
        for(int x=0; x < mrf.width; x++) {
            int cur_label = mrf.grid[y*width+x].best_assignment;

            // Data cost
            energy += mrf.grid[y*width+x].msg[DATA][cur_label];

            if(x-1 >= 0)     energy += SmoothnessCost(cur_label, mrf.grid[y*width+x-1].best_assignment);
            if(x+1 < width)  energy += SmoothnessCost(cur_label, mrf.grid[y*width+x+1].best_assignment);
            if(y-1 >= 0)     energy += SmoothnessCost(cur_label, mrf.grid[(y-1)*width+x].best_assignment);
            if(y+1 < height) energy += SmoothnessCost(cur_label, mrf.grid[(y+1)*width+x].best_assignment);
        }
    }

    return energy;
}

