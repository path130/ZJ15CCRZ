

//#include <omp.h>
#include <math.h>
#include <vector>
#include <fstream>

using namespace std;

typedef struct square_box
{
	int row;
	int col;
	int size;
	int height;
	int width;
	float score;
	int neighbors;
}square_box;

class NPDScan_Face_Det
{
public:
	~NPDScan_Face_Det();
	void model_loader(const char* name_model);
	vector<square_box> DetectFace(int height, int width, const unsigned char* I, int n_threads = 0, int minF = 20, int maxF = 4000, float overlappingThr=0.5);

private:

	vector<square_box> NPDScan(int height, int width, const unsigned char* I, int minF , int maxF, int n_threads );
	int Find(vector<int>& parent, int x);//return root index
	int Partition(vector<int>&output_label, vector<vector<int> >& Ajacent_matrix);//return nGroups
	float Logistic(float X);

	int objSize;
	int numStages ;
	int numLeafNodes;
	int numBranchNodes ;
	const float *pStageThreshold ;
	const int *pTreeRoot ;

	int numScales ;
	int ** ppPoints1;
	int ** ppPoints2;

	const unsigned char* ppCutpoint[2];

	const int *pLeftChild ;
	const int *pRightChild ;
	const float *pFit;

	unsigned char * ppNpdTable[256];

	//double scaleFactor = mxGetScalar(mxGetField(pModel, 0, "scaleFactor"));
	const int *pWinSize ;
};
