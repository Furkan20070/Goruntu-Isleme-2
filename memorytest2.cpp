#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int MAX_MEMORY_SIZE = 20;

struct MemoryConnection
{
	int x;
	int y;
	int fromId;
	int toId;
};

struct HierarchialHitbox
{
	Rect2f hitbox;
	int level;
	int memoryNodeId;

	HierarchialHitbox(int hitboxX, int hitboxY, int nodeId, int nodeLevel) : level(nodeLevel), memoryNodeId(nodeId)
	{
		hitbox.x = hitboxX - 1;
		hitbox.y = hitboxY - 1;
		hitbox.width = 1.0f;
		hitbox.height = 1.00001f;
	}
};

class MemoryNode
{
private:
	int id;
	int level;
	vector<MemoryConnection> connections;
public:
	MemoryNode(int nodeId, int nodeLevel) : id(nodeId), level(nodeLevel)
	{

	}

	int GetId()
	{
		return id;
	}

	int GetLevel()
	{
		return level;
	}

	void AddConnection(int atX, int atY, int withMemoryNode, int toMemoryNode)
	{
		connections.push_back({ atX, atY, withMemoryNode, toMemoryNode });
	}

	void AddHitbox(vector<HierarchialHitbox*>& to, int x, int y)
	{
		to.push_back(new HierarchialHitbox(x, y, id, level));
	}

	int BruteSearchEstablishedConnections(int xDiff, int yDiff, int nodeId)
	{
		for (int i = 0; i < connections.size(); i++)
		{
			if (connections[i].x == xDiff && connections[i].y == yDiff && connections[i].fromId == nodeId)
			{
				//cout << "Connection found! Returning memory node id: " << connections[i].toId << "." << endl << endl;
				return connections[i].toId;
			}
		}

		//cout << "No memory found. Notifying handler by returning -1. " << endl << endl;
		return -1;
	}

	void DebugPrintMemoryNode()
	{
		cout << "-- Memory Node Info -- " << endl;
		cout << "Memory Node Id: " << id << endl;
		cout << "Memory Node Level: " << level << endl;
		cout << "-- Memory Node Info End --" << endl << endl;
	}
};

class MemoryHandler
{
private:
	vector<MemoryNode*> memoryNodes;
	vector<vector<HierarchialHitbox*>> activeHitboxes;

	inline bool overlapY(const cv::Rect2f& a, const cv::Rect2f& b) 
	{
		return (a.y < b.y + b.height) && (b.y < a.y + a.height);
	}

	vector<HierarchialHitbox*> GetCollidingWithExisting(const HierarchialHitbox& hb, int level)
	{
		std::vector<HierarchialHitbox*> colliding;
		float minX = hb.hitbox.x;
		float maxX = hb.hitbox.x + hb.hitbox.width;

		float minY = hb.hitbox.y;
		float maxY = hb.hitbox.y + hb.hitbox.height;

		for (auto it = activeHitboxes[level].rbegin() + 1; it != activeHitboxes[level].rend(); ++it)
		{
			const auto& other = (*it)->hitbox;

			// X cutoff (no more possible overlaps)
			if (other.x + other.width < minX) break;

			// Quick vertical range skip
			if (other.y > maxY || other.y + other.height < minY)
				continue; // No Y overlap possible

			// Full overlap check
			if (overlapY(hb.hitbox, other) && hb.level == (*it)->level)
			{
				colliding.push_back(*it);
			}
		}
		return colliding;
	}
public:
	MemoryHandler()
	{
		for (int i = 0; i < MAX_MEMORY_SIZE; i++)
		{
			activeHitboxes.push_back(vector<HierarchialHitbox*>());
		}
	}

	bool AddMemoryNode(int x, int y, int level)
	{
		if (level >= MAX_MEMORY_SIZE)
		{
			//cout << "Max memory level reached." << endl;
			return false;
		}
		//cout << "Adding a new memory node to handler..." << endl << endl;
		memoryNodes.push_back(new MemoryNode(memoryNodes.size(), level));
		//memoryNodes.back()->DebugPrintMemoryNode();
		AddHitboxFromMemoryNode(x, y, memoryNodes.size() - 1);
		return true;
	}

	void HitboxCollideAndMemoryCheck(int level)
	{
		vector<HierarchialHitbox*> currentColliding = GetCollidingWithExisting(*activeHitboxes[level].back(), level);

		//cout << "Found " << currentColliding.size() << " hitbox interactions." << endl << endl;

		for (int i = 0; i < currentColliding.size(); i++)
		{
			int xDiff = activeHitboxes[level].back()->hitbox.x - currentColliding[i]->hitbox.x;
			int yDiff = activeHitboxes[level].back()->hitbox.y - currentColliding[i]->hitbox.y;
			int memoryCheck = memoryNodes[currentColliding[i]->memoryNodeId]->BruteSearchEstablishedConnections(xDiff, yDiff, activeHitboxes[level].back()->memoryNodeId);
			if (memoryCheck == -1)
			{
				//cout << "Creating a new memory node" << endl << endl;
				bool addSuccessful = AddMemoryNode(activeHitboxes[level].back()->hitbox.x + 1, activeHitboxes[level].back()->hitbox.y + 1, level + 1);
				if (addSuccessful)
				{
					memoryNodes[currentColliding[i]->memoryNodeId]->AddConnection(xDiff, yDiff, activeHitboxes[level].back()->memoryNodeId, memoryNodes.back()->GetId());
				}
			}
			else
			{
				AddHitboxFromMemoryNode(activeHitboxes[level].back()->hitbox.x + 1, activeHitboxes[level].back()->hitbox.y + 1, memoryCheck);
			}
		}
	}

	void AddHitboxFromMemoryNode(int x, int y, int memoryNodeId)
	{
		//cout << "Firing up memory node " << memoryNodeId << " at x: " << x << " y: " << y << endl << endl;
		memoryNodes[memoryNodeId]->AddHitbox(activeHitboxes[memoryNodes[memoryNodeId]->GetLevel()], x, y);
		HitboxCollideAndMemoryCheck(memoryNodes[memoryNodeId]->GetLevel());
	}

	vector<MemoryNode*>& GetMemoryNodes()
	{
		return memoryNodes;
	}

	vector<HierarchialHitbox*>& GetActiveHierarchialHitboxes(int level)
	{
		return activeHitboxes[level];
	}
};

int main()
{
	Mat image = imread("edge.png", IMREAD_GRAYSCALE);
	
	MemoryHandler handler;
	handler.AddMemoryNode(0, 0, 0);

	for (int i = 0; i < image.cols; i++)
	{
		for (int j = 0; j < image.rows; j++)
		{
			if (image.at<uchar>(i, j))
			{
				handler.AddHitboxFromMemoryNode(j, i, 0);
			}
		}
	}
}