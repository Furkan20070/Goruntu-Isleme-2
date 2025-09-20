#pragma once

#include <opencv2/opencv.hpp>
#include <random>
#include <vector>
#include <cmath>

class SmallDataGenerator
{
private:
	const int IMG_SIZE = 400;
	const int SHAPE_SIZE = 80; // Fixed size for shape bounding box
	const int LINE_THICKNESS = 1;

	cv::RNG rng; // Random generator
	cv::Mat img;
	cv::Point getRandomCenter();
	void drawTriangle();
	void drawSquare();
	void drawRotatedSquare();
	void drawCircle();
	void drawRoundedSquare();
	void drawRotatedTriangle();
public:
	SmallDataGenerator()
	{
		rng = cv::RNG(cv::getTickCount());
		img = cv::Mat::zeros(cv::Size(400, 400), CV_8UC1);
	}

	void drawRandomShape();
	void debugShowImage();
	void resetDrawingMat();
	cv::Mat& getImage();
};






