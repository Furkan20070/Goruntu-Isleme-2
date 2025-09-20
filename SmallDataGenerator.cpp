#include "SmallDataGenerator.h"


cv::Point SmallDataGenerator::getRandomCenter()
{
	int margin = SHAPE_SIZE / 2 + 5;
	int x = rng.uniform(margin, IMG_SIZE - margin);
	int y = rng.uniform(margin, IMG_SIZE - margin);
	return { x, y };
}

void SmallDataGenerator::drawCircle()
{
	cv::Point center = getRandomCenter();
	int radius = SHAPE_SIZE / 2;
	cv::circle(img, center, radius, cv::Scalar(255, 255, 255), LINE_THICKNESS, cv::LINE_AA);
}

void SmallDataGenerator::drawSquare()
{
	cv::Point center = getRandomCenter();
	int half = SHAPE_SIZE / 2;
	cv::Rect rect(center.x - half, center.y - half, SHAPE_SIZE, SHAPE_SIZE);
	cv::rectangle(img, rect, cv::Scalar(255, 255, 255), LINE_THICKNESS, cv::LINE_AA);
}

void SmallDataGenerator::drawTriangle()
{
	cv::Point center = getRandomCenter();
	float r = SHAPE_SIZE / 2;
	std::vector<cv::Point> pts;
	for (int i = 0; i < 3; ++i) {
		float angle = CV_PI * 2 * i / 3.0;
		pts.emplace_back(center.x + r * cos(angle), center.y + r * sin(angle));
	}
	cv::polylines(img, pts, true, cv::Scalar(255, 255, 255), LINE_THICKNESS, cv::LINE_AA);
}

void SmallDataGenerator::drawRotatedSquare()
{
	cv::Point center = getRandomCenter();
	float halfSize = SHAPE_SIZE / 2.0f;
	float angle = rng.uniform(0.0f, (float)(2 * CV_PI)); // random rotation in radians

	std::vector<cv::Point> pts;
	for (int i = 0; i < 4; ++i)
	{
		float theta = angle + i * CV_PI / 2.0f; // add 90° each time
		pts.emplace_back(
			center.x + halfSize * cos(theta),
			center.y + halfSize * sin(theta)
		);
	}

	cv::polylines(img, pts, true, cv::Scalar(255, 255, 255), LINE_THICKNESS, cv::LINE_AA);
}

void SmallDataGenerator::drawRoundedSquare()
{
	cv::Point center = getRandomCenter();
	int radius = SHAPE_SIZE / 5; // corner radius
	int side = SHAPE_SIZE;

	// Define outer rectangle
	int left = center.x - side / 2 + radius;
	int right = center.x + side / 2 - radius;
	int top = center.y - side / 2 + radius;
	int bottom = center.y + side / 2 - radius;

	// Straight sides
	cv::line(img, { left, center.y - side / 2 }, { right, center.y - side / 2 }, cv::Scalar(255, 255, 255), LINE_THICKNESS, cv::LINE_AA); // top
	cv::line(img, { left, center.y + side / 2 }, { right, center.y + side / 2 }, cv::Scalar(255, 255, 255), LINE_THICKNESS, cv::LINE_AA); // bottom
	cv::line(img, { center.x - side / 2, top }, { center.x - side / 2, bottom }, cv::Scalar(255, 255, 255), LINE_THICKNESS, cv::LINE_AA); // left
	cv::line(img, { center.x + side / 2, top }, { center.x + side / 2, bottom }, cv::Scalar(255, 255, 255), LINE_THICKNESS, cv::LINE_AA); // right

	// Arcs (drawn clockwise starting from top-left corner)
	cv::ellipse(img, { left, top }, { radius, radius }, 180, 0, 90, cv::Scalar(255, 255, 255), LINE_THICKNESS, cv::LINE_AA); // Top-left
	cv::ellipse(img, { right, top }, { radius, radius }, 270, 0, 90, cv::Scalar(255, 255, 255), LINE_THICKNESS, cv::LINE_AA); // Top-right
	cv::ellipse(img, { right, bottom }, { radius, radius }, 0, 0, 90, cv::Scalar(255, 255, 255), LINE_THICKNESS, cv::LINE_AA); // Bottom-right
	cv::ellipse(img, { left, bottom }, { radius, radius }, 90, 0, 90, cv::Scalar(255, 255, 255), LINE_THICKNESS, cv::LINE_AA); // Bottom-left
}

void SmallDataGenerator::drawRandomShape()
{
	int shapeType = rng.uniform(0, 6); // 0–4
	switch (shapeType) 
	{
	case 0: drawTriangle(); break;
	case 1: drawSquare(); break;
	case 2: drawRotatedSquare(); break;
	case 3: drawCircle(); break;
	case 4: drawRoundedSquare(); break;
	case 5: drawRotatedTriangle(); break;
	}
}

void SmallDataGenerator::drawRotatedTriangle() 
{
	cv::Point center = getRandomCenter();
	float r = SHAPE_SIZE / 2.0f;
	float angleOffset = rng.uniform(0.0f, (float)(2.0f * CV_PI)); // Random rotation angle in radians

	std::vector<cv::Point> pts;
	for (int i = 0; i < 3; ++i) 
	{
		float angle = angleOffset + 2 * CV_PI * i / 3.0f;
		pts.emplace_back(center.x + r * cos(angle), center.y + r * sin(angle));
	}

	cv::polylines(img, pts, true, cv::Scalar(255, 255, 255), LINE_THICKNESS, cv::LINE_AA);
}

void SmallDataGenerator::resetDrawingMat()
{
	img = cv::Mat::zeros(cv::Size(400, 400), CV_8UC1);
}

void SmallDataGenerator::debugShowImage()
{
	cv::imshow("Data Image", img);
	cv::waitKey(0);
}

cv::Mat& SmallDataGenerator::getImage()
{
	return img;
}
