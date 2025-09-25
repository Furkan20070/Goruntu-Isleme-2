#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <functional>
#include <utility>

using namespace std;
using namespace cv;

//instead of having a symbolic representation for every possible combination of impulses (see memorytest2.cpp), 
//we'll try to extract useful amount of, hey, here is a 30° line, here is a curve, here is a vertical line 
//(which is a crucial part of the shape square) etc. Aim here is to combine small info into symbolic and related and 
//hierarchial representations with managable size. after getting "impulses" (i really like this word) we can do quick 
//and smart object recognition (hopefully). 
// 
//currently, the symbolic representations only have single integer id's. this is a quite temporary problem, 
//feature vectors or other kinds of embeddings can be easily assigned once the system is properly set up.

struct SymbolicImpulse 
{
    int atX;
    int atY;
    int id;
};

struct SymbolicMemory
{
	int xDiff;
	int yDiff;
	//int xDiff and int yDiff - this is a very cheap method for encoding spatial dependencies. 
	//in the future, those should be changed into a format or data structure computer can actually learn from and manipulate.
	int fromId1;
	int fromId2;

	bool operator==(const SymbolicMemory& other) const 
	{
		return xDiff == other.xDiff && yDiff == other.yDiff && fromId1 == other.fromId1 && fromId2 == other.fromId2;
	}
};

struct SymbolicMemoryHasher
{
	size_t operator()(const SymbolicMemory& sm) const 
	{
		size_t h1 = hash<int>{}(sm.xDiff);
		size_t h2 = hash<int>{}(sm.yDiff);
		size_t h3 = hash<int>{}(sm.fromId1);
		size_t h4 = hash<int>{}(sm.fromId2);

		return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
	}
};

struct PairHasher //hashing opencv's point class might be messy, so we go with std::pair for lookups
{
	size_t operator()(const pair<int, int>& p) const 
	{
		return hash<int>{}(p.first) ^ (hash<int>{}(p.second) << 1);
	}
};

vector<Point> possibleNeighbors = { {0, -1}, {-1, -1}, {-1, 0}, {-1, 1} }; //scanline moves from left to right and scans from up

unordered_map<SymbolicMemory, int, SymbolicMemoryHasher> symbolicDatabase; //big memory - gonna need some saving/loading functionality for this
int currentMaxId = -1; //counter for assigning unique id's to symbolic memories

class ImpulseScanLine
{
private:
	ImpulseScanLine* next; //if any - this allows building bigger memory blocks from smaller ones. memory impulses occured here should be passed to this.
	unordered_map<pair<int, int>, int, PairHasher> impulseMemory;
public:
	void Impulse(SymbolicImpulse i)
	{
		for (const Point& p : possibleNeighbors)
		{
			Point neighbor(i.atX + p.x, i.atY + p.y);
			if (impulseMemory.find({ neighbor.x, neighbor.y }) != impulseMemory.end()) // check for nearby impulse
			{
				if (symbolicDatabase.find({ p.x, p.y, impulseMemory[{neighbor.x, neighbor.y}], i.id }) != symbolicDatabase.end())
				{
					// we found an existing combination. fire it up.
				}
				else
				{
					// no existing combination so we add it. 
					currentMaxId++;
					symbolicDatabase[{p.x, p.y, impulseMemory[{neighbor.x, neighbor.y}], i.id}] = currentMaxId;
				}
			}
		}

		//after we process nearby impulses, impulseMemory should be updated.
	}
};

int main()
{
	Mat image = imread("edge.png", IMREAD_GRAYSCALE);
	

}