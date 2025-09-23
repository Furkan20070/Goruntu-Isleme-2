#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace std;
using namespace cv;

class MemoryNode
{
private:
	Rect2f hitbox;
	string name;
public:
	MemoryNode(int x, int y, string nodeName = "") : name(nodeName)
	{
		hitbox.x = x - 1;
		hitbox.y = y - 1;
		hitbox.width = 2.0f;
		hitbox.height = 2.0f;
	}

	Rect2f& GetHitbox()
	{
		return hitbox;
	}

	void DebugPrintName()
	{
		cout << "Name of called MemoryNode: " << name << endl;
	}
};

class MemoryHandler
{
private:
	vector<MemoryNode*> activeMemoryNodes;

	inline bool overlapY(const cv::Rect2f& a, const cv::Rect2f& b) 
	{
		return (a.y < b.y + b.height) && (b.y < a.y + a.height);
	}

	vector<MemoryNode*> GetCollidingWithExisting(const Rect2f& hb)
	{
		std::vector<MemoryNode*> colliding;
		float minX = hb.x;
		float maxX = hb.x + hb.width;

		float minY = hb.y;
		float maxY = hb.y + hb.height;

		for (auto it = activeMemoryNodes.rbegin() + 1; it != activeMemoryNodes.rend(); ++it)
		{
			const auto& other = (*it)->GetHitbox();

			// X cutoff (no more possible overlaps)
			if (other.x + other.width < minX) break;

			// Quick vertical range skip
			if (other.y > maxY || other.y + other.height < minY)
				continue; // No Y overlap possible

			// Full overlap check
			if (overlapY(hb, other))
			{
				colliding.push_back(*it);
			}
		}
		return colliding;
	}
public:
	void AddMemoryNode(int x, int y, String name = "")
	{
		activeMemoryNodes.push_back(new MemoryNode(x, y, name));

		vector<MemoryNode*> currentColliding = GetCollidingWithExisting(activeMemoryNodes.back()->GetHitbox());
		
		for (int i = 0; i < currentColliding.size(); i++)
		{
			currentColliding[i]->DebugPrintName();
		}
	}
};

int main()
{
	Mat image = imread("edge.png", IMREAD_GRAYSCALE);
	Mat debug(image.rows, image.cols, CV_8UC1, Scalar(0));

	/*for (int i = 0; i < image.cols; i++)
	{
		for (int j = 0; j < image.rows; j++)
		{
			
		}
	}*/

	MemoryHandler memoryHandler;
	memoryHandler.AddMemoryNode(0, 0, "first");
	memoryHandler.AddMemoryNode(3, 0, "second");
}