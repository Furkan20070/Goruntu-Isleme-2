#include <opencv2/opencv.hpp>
#include <iostream>
#include <utility>

using namespace std;
using namespace cv;

// initial autofocus.cpp file was dirty, and potentially, not working due to experimentation with the algorithm.
// here is a clean version of the autofıcus algorithm, there are certainly some room for improvement, but i think 
// it's fine for now.

vector<Point> directions = { {-1, 0}, {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1} };
vector<Point2f> directionsF = 
{
    { -1.0f,  0.0f }, // left
    { -0.7071f, -0.7071f }, // up-left
    {  0.0f, -1.0f }, // up
    {  0.7071f, -0.7071f }, // up-right
    {  1.0f,  0.0f }, // right
    {  0.7071f,  0.7071f }, // down-right
    {  0.0f,  1.0f }, // down
    { -0.7071f,  0.7071f } // down-left
};

inline float similarityL1(const Vec3b& a, const Vec3b& b) // l1 similarity for memory 
{
    int diff =
        abs(a[0] - b[0]) +
        abs(a[1] - b[1]) +
        abs(a[2] - b[2]);

    return 1.0f - (diff / 765.0f);
}

inline Point2f normalize(const Point2f& v) // simple normalizing function, also prunes weak signals
{
    float length = sqrt(v.x * v.x + v.y * v.y);

    if (length < 0.1f)
        return Point2f(0.0f, 0.0f);

    return Point2f(v.x / length, v.y / length);
}

inline float mapFloatSafe(float value, float inMin, float inMax, float outMin, float outMax)
{
    if (inMax - inMin == 0.0f)
        return outMin; 

    return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
}

void GetStimuli(Mat& prev, Mat& curr, Mat& out) // core algorithm of autofocus concept
{
    for (int i = 1; i < prev.cols - 1; i++)
    {
        for (int j = 1; j < prev.rows - 1; j++)
        {
            Point2f dir(0.0f, 0.0f);
            Vec3b& ownValue = prev.at<Vec3b>(j, i); // save previous value at pixel
            for (int k = 0; k < 8; k++) // compare to current values of neighbors to detect motion
            {
                Vec3b& testValue = curr.at<Vec3b>(j + directions[k].y, i + directions[k].x);
                float similarity = similarityL1(ownValue, testValue);
                dir += directionsF[k] * similarity; // similar neighbor means point of interest shifted towards here.
            }
            dir = normalize(dir);
            out.at<Vec3b>(j, i) = Vec3b((int)mapFloatSafe(dir.x, -1.0f, 1.0f, 0.0f, 255.0f), (int)mapFloatSafe(dir.y, -1.0f, 1.0f, 0.0f, 255.0f), 127);
        }
    }
}

int main()
{
    Mat previous = imread("autofocustest1/8.png"); // read blurred versions of image -- in production, gaussian blurs can be calculated at real time
    Mat out(previous.rows, previous.cols, CV_8UC3, Scalar(0, 0, 0));
    for (int i = 7; i >= 1; i--)
    {
        Mat current = imread("autofocustest1/" + to_string(i) + ".png");
        GetStimuli(previous, current, out);
        imshow("out", out);
        waitKey(0); // visual debug
        previous = current;
    }

    imshow("Direction Map (GB)", out);
    waitKey(0);

    imwrite("autofocusresult.png", out);

    return 0;
}