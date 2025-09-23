#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "json.hpp"

namespace fs = std::filesystem;
using namespace cv;
using namespace std;
using json = nlohmann::json;

int shapeSafetySize = 140;
int impulseSize = 1;
float impulseStrength = 0.5f;
int dataAmount = 5;
int shapeCount = 5;
int minimumShapeSize = 25;
int maximumShapeSize = 80;
string dataFolder = "impulseStream";
string labelFolder = "labelStream";

Point GetRandomPoint(RNG& rng, int width, int height, Mat& areaMemory)
{
	int x = rng.uniform(20, width);   // uniform integer in [0, width)
	int y = rng.uniform(20, height);  // uniform integer in [0, height)

	if (areaMemory.at<uchar>(y, x) != 0)
	{
		return Point(-1, -1);
	}
	else
	{
		return Point(x, y);
	}
}

vector<string> LabelsToStringVector(string className, int xUpperLeft, int xlowerRight, int yUpperLeft, int yLowerRight)
{
	vector<string> res;
	res.push_back(className);
	res.push_back(to_string(xUpperLeft));
	res.push_back(to_string(xlowerRight));
	res.push_back(to_string(yUpperLeft));
	res.push_back(to_string(yLowerRight));
	return res;
}

vector<string> DrawRectangle(Point& center, int size, Mat& image, Mat& memory)
{
	int halfSize = size / 2;
	Point upperLeft(center.x - halfSize, center.y - halfSize);
	Point lowerRight(center.x + halfSize, center.y + halfSize);
	int shapeSafetyHalfSize = shapeSafetySize / 2;
	Point upperLeftMemory(center.x - shapeSafetyHalfSize, center.y - shapeSafetyHalfSize);
	Point lowerRightMemory(center.x + shapeSafetyHalfSize, center.y + shapeSafetyHalfSize);
	rectangle(image, upperLeft, lowerRight, Scalar(255), 1);
	rectangle(memory, upperLeftMemory, lowerRightMemory, Scalar(255), -1);
	vector<string> outputs;
	return LabelsToStringVector("square", max(0, upperLeft.x), min(199, lowerRight.x), max(0, upperLeft.y), min(199, lowerRight.y));
}

vector<string> DrawCircle(Point& center, int size, Mat& image, Mat& memory)
{
	size /= 2;
	circle(image, center, size, Scalar(255), 1);
	circle(memory, center, shapeSafetySize / 2, Scalar(255), -1);
	return LabelsToStringVector("circle", max(0, center.x - size), min(199, center.x + size), max(0, center.y - size), min(199, center.y + size));
}

vector<string> DrawTriangle(Point& center, int size, float rotation, Mat& image, Mat& memory, bool forMemory)
{
	float height = size * sqrt(3.0f) / 2.0f;
	float angleRad = rotation * CV_PI / 180.0f;

	vector<Point> points(3);
	points[0] = Point(0, -2.0f / 3.0f * height);
	points[1] = Point(-size / 2, height / 3.0f);
	points[2] = Point(size / 2, height / 3.0f);

	for (Point& p : points)
	{
		float x = p.x * cos(angleRad) - p.y * sin(angleRad);
		float y = p.x * sin(angleRad) + p.y * cos(angleRad);
		p = Point(static_cast<int>(x + center.x), static_cast<int>(y + center.y));
	}

	vector<vector<Point>> contour = { points };

	int xUpperLeft = points[0].x;
	int xLowerRight = points[0].x;
	int yUpperLeft = points[0].y;
	int yLowerRight = points[0].y;
	for (int i = 1; i < points.size(); i++)
	{
		if (points[i].x < xUpperLeft)
		{
			xUpperLeft = points[i].x;
		}

		if (points[i].x > xLowerRight)
		{
			xLowerRight = points[i].x;
		}

		if (points[i].y < yUpperLeft)
		{
			yUpperLeft = points[i].y;
		}

		if (points[i].y > yLowerRight)
		{
			yLowerRight = points[i].y;
		}
	}

	if (!forMemory)
	{
		polylines(image, contour, true, Scalar(255), 1);
		DrawTriangle(center, shapeSafetySize, rotation, image, memory, true);
		return LabelsToStringVector("triangle", max(0, xUpperLeft), min(199, xLowerRight), max(0, yUpperLeft), min(199, yLowerRight));
	}
	else
	{
		fillPoly(memory, contour, Scalar(255));
		return vector<string>();
	}
}

void ImpulseAt(vector<float>& impulseVector, int index)
{
	impulseVector[index] += impulseStrength;

	int leftPointer = index - 1;
	int rightPointer = index + 1;

	for (int i = 1; i < impulseSize; i++)
	{
		float spreadImpulse = impulseStrength / (float)(i + 1);;

		if (leftPointer >= 0)
		{
			impulseVector[leftPointer] += spreadImpulse;
			leftPointer--;
		}

		if (rightPointer < impulseVector.size())
		{
			impulseVector[rightPointer] += spreadImpulse;
			rightPointer++;
		}
	}
}

//void ImpulseAt(vector<float>& impulseVector, int index)
//{
//	impulseVector[index] += impulseStrength;
//
//	int leftPointer = index - 1;
//	int rightPointer = index + 1;
//
//	for (int i = 1; i < impulseSize; i++)
//	{
//		float spreadImpulse = impulseStrength / (float)(i + 1);;
//
//		if (leftPointer >= 0)
//		{
//			impulseVector[leftPointer] += spreadImpulse;
//			leftPointer--;
//		}
//
//		if (rightPointer < impulseVector.size())
//		{
//			impulseVector[rightPointer] += spreadImpulse;
//			rightPointer++;
//		}
//	}
//}

template <typename T>
bool WriteVectorToFile(const vector<vector<T>>& data, const string& filename)
{
	ofstream outFile(filename);

	if (!outFile)
	{
		cerr << "Failed to open file!" << endl;
		return false;
	}

	for (const auto& row : data)
	{
		for (size_t i = 0; i < row.size(); i++)
		{
			outFile << row[i];
			if (i < row.size() - 1)
			{
				outFile << " ";
			}
		}

		outFile << "\n";
	}

	return true;
}

int main()
{
	RNG rng(getTickCount());

	if (!fs::exists(dataFolder))
	{
		fs::create_directory(dataFolder);
	}

	if (!fs::exists(labelFolder))
	{
		fs::create_directory(labelFolder);
	}

	for (int datasetEntry = 0; datasetEntry < dataAmount; datasetEntry++)
	{
		Mat image = Mat::zeros(200, 200, CV_8UC1);
		Mat areaMemory = Mat::zeros(200, 200, CV_8UC1);
		json jsonFile;
		jsonFile["impulse_stream"] = json::array();
		jsonFile["objects"] = json::array();

		for (int i = 0; i < shapeCount; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				Point p = GetRandomPoint(rng, 181, 181, areaMemory);
				if (p.x != -1)
				{
					int shapeSelect = rng.uniform(0, 3);
					int shapeSize = rng.uniform(minimumShapeSize, maximumShapeSize + 1);
					int randomRotation = rng.uniform(0, 361);
					vector<string> labels;
					switch (shapeSelect)
					{
					case 0:
						labels = DrawCircle(p, shapeSize, image, areaMemory);
						break;
					case 1:
						labels = DrawRectangle(p, shapeSize, image, areaMemory);
						break;
					case 2:
						labels = DrawTriangle(p, shapeSize, randomRotation, image, areaMemory, false);
						break;
					default:
						break;
					}
					if (labels.empty())
					{
						cout << "AAAAAAAAAAAAAAAAA" << endl;
					}
					jsonFile["objects"].push_back({ { "class", labels[0] }, { "xMin", labels[1] }, { "xMax", labels[2] }, { "yMin", labels[3] }, { "yMax", labels[4] } });
					break;
				}
			}
		}

		vector<vector<int>> impulses;
		//vector<vector<int>> classIds;
		for (int i = 0; i < 200; i++)
		{
			impulses.push_back(vector<int>());
			//classIds.push_back(vector<int>(200, 0));
			for (int j = 0; j < 200; ++j)
			{
				if (image.at<uchar>(j, i))
				{
					impulses.back().push_back(j);
					//ImpulseAt(impulses.back(), j);
					//classIds.back()[j] = image.at<uchar>(j, i);
				}
			}
		}

		for (const vector<int>& vec : impulses)
		{
			jsonFile["impulse_stream"].push_back(vec);
		}

		string labelStreamName = labelFolder + "/" + to_string(datasetEntry) + ".json";
		ofstream out(labelStreamName);
		out << jsonFile.dump(4); 
		out.close();

		imwrite(to_string(datasetEntry) + ".png", image);

		//string impulseStreamName = dataFolder + "/" + to_string(datasetEntry) + ".txt";
		//string labelStreamName = labelFolder + "/" + to_string(datasetEntry) + ".txt";

		//WriteVectorToFile(impulses, impulseStreamName);
		//WriteVectorToFile(classIds, labelStreamName);
	}
}
