//#include <opencv2/opencv.hpp>
//#include <fstream>
//#include "PatternDatabase.h"
//#include "SmallDataGenerator.h"
//#include "DiscreteEmbeddingScanLineLayer.h"
//
//using namespace std;
//using namespace cv;
//
//const float decayRate = 0.9f;
//const float impulseStrength = 1.0f;
//const float memoryThreshold = 0.2f;
//const int kernelSize = 2;
//const float clusterThreshold = 0.16f;
//
//// memory: vertical memory vector (decaying values)
//// y: position of the impulse (integer)
////void SaveImpulsePattern(int y, const vector<float>& memory, const string& filename = "patterns.txt") 
////{
////	std::ofstream outfile(filename, std::ios::app); // append mode
////	if (!outfile.is_open()) 
////	{
////		std::cerr << "Failed to open file for writing patterns.\n";
////		return;
////	}
////
////	// Extract kernel around y
////	for (int offset = -2; offset <= 2; ++offset) 
////	{
////		outfile << memory[y + offset];
////		if (offset != 2) outfile << " ";
////	}
////	outfile << "\n"; // new line after each vector
////
////	outfile.close();
////}
//
//void SaveImpulsePattern(int y, const vector<float>& memory, PatternDatabase& database)
//{
//	vector<float> pattern;
//	for (int offset = -2; offset <= 2; ++offset)
//	{
//		pattern.push_back(memory[y + offset]);
//	}
//
//	database.addPattern(pattern);
//}
//
//int RecognizeImpulsePattern(int y, const vector<float>& memory, PatternDatabase& database)
//{
//	vector<float> pattern;
//	for (int offset = -2; offset <= 2; ++offset)
//	{
//		pattern.push_back(memory[y + offset]);
//	}
//
//	return database.classify(pattern);
//}
//
//void DebugVisualizeMemoryVector(const vector<float>& memoryVector)
//{
//	Mat memoryVectorMat(400, 1, CV_32FC1, Scalar(0.0f));
//	for (int i = 0; i < memoryVector.size(); i++)
//	{
//		memoryVectorMat.at<float>(i, 0) = (float)memoryVector[i];
//	}
//
//	resize(memoryVectorMat, memoryVectorMat, Size(25, 400));
//	imshow("memoryVector", memoryVectorMat);
//	waitKey(0);
//}
//
//int main() 
//{
//	SmallDataGenerator generator;
//	PatternDatabase database(clusterThreshold);
//	database.loadFromFile("clusters.txt");
//	DiscreteEmbeddingScanLineLayer layer2(400, 0.9f, clusterThreshold, true, kernelSize);
//	layer2.loadClusters("clustersl2.txt");
//
//	for (int dataIteration = 0; dataIteration < 10; dataIteration++)
//	{
//		generator.drawRandomShape();
//
//		vector<float> memoryVector(400, 0.0f);
//		queue<int> saveEventLocations;
//
//		Mat& image = generator.getImage();
//		Mat debug = image.clone();
//		int recognizedClusterAmount = 0;
//
//		for (int i = 0; i < image.cols; i++)
//		{
//			std::vector<int> clusterIds(400, -1);
//			for (int j = 0; j < image.rows; j++)
//			{
//				memoryVector[j] *= decayRate;
//				if (memoryVector[j] < memoryThreshold)
//				{
//					memoryVector[j] = 0.0f;
//				}
//			}
//
//			while (!saveEventLocations.empty())
//			{
//				if (saveEventLocations.front() - kernelSize > 0 && saveEventLocations.front() + kernelSize < 399)
//				{
//					int recognitionResult = RecognizeImpulsePattern(saveEventLocations.front(), memoryVector, database);
//					if (recognitionResult != -1)
//					{
//						clusterIds[saveEventLocations.front()] = recognitionResult;
//						//cout << "Pattern recognized! Cluster ID: " << recognitionResult << " Clusters: " << recognizedClusterAmount << endl;
//						recognizedClusterAmount++;
//						debug.at<uchar>(saveEventLocations.front(), i) = recognitionResult * 10;
//					}
//				}
//				saveEventLocations.pop();
//			}
//
//			for (int j = 0; j < image.rows; j++)
//			{
//				if (image.at<uchar>(j, i) > 0)
//				{
//					memoryVector[j] = impulseStrength;
//					saveEventLocations.push(j);
//				}
//			}
//
//			layer2.step(clusterIds, database);
//		}
//
//		imshow("test", debug);
//		waitKey(0);
//		generator.resetDrawingMat();
//	}
//
//	layer2.saveClusters("clustersl2.txt");
//
//	return 0;
//
//	/*SmallDataGenerator generator;
//	PatternDatabase database(clusterThreshold);
//
//	for (int dataIteration = 0; dataIteration < 10000; dataIteration++)
//	{
//		generator.drawRandomShape();
//
//		vector<float> memoryVector(400, 0.0f);
//		queue<int> saveEventLocations;
//
//		Mat& image = generator.getImage();
//
//		cout << "Data Iteration " << dataIteration << endl;
//
//		for (int i = 0; i < image.cols; i++)
//		{
//			for (int j = 0; j < image.rows; j++)
//			{
//				memoryVector[j] *= decayRate;
//				if (memoryVector[j] < memoryThreshold)
//				{
//					memoryVector[j] = 0.0f;
//				}
//			}
//
//			while (!saveEventLocations.empty())
//			{
//				if (saveEventLocations.front() - kernelSize > 0 && saveEventLocations.front() + kernelSize < 399)
//				{
//					SaveImpulsePattern(saveEventLocations.front(), memoryVector, database);
//				}
//				saveEventLocations.pop();
//			}
//
//			for (int j = 0; j < image.rows; j++)
//			{
//				if (image.at<uchar>(j, i) > 0)
//				{
//					memoryVector[j] = impulseStrength;
//					saveEventLocations.push(j);
//				}
//			}
//		}
//
//		generator.resetDrawingMat();
//	}
//
//	database.saveToFile("clusters.txt");
//
//	return 0;*/
//}
