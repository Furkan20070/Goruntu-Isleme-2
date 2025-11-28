#include <opencv2/opencv.hpp>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <iostream>
#include <cmath>
#include <cassert>

using namespace std;
using namespace cv;

// algorithm for effectively detecting edges and their orientations in an image fast. basically a simulation of retina and 
// v1 parts of human visual cortex.
// this uses a very cool attribute of gaussian blur. when you go from high blur level to low blur level (at small decrements), you can observe 
// at pixel level blur is shifting towards the edge (the points of interest can also be said). by utilizing this, we can 
// extract edges AND ALSO their orientations at real time. just like retina and v1 parts of the brain does. what this algorithm does as the 
// slightly less blurred versions of the image comes in, it basically creates a temporary small memory and keeps tracks of where it "remembered" that
// memory. this returns an image with clear edges and their orientations. and since the core of the algorithm is gaussian blur,
// it ignores / eliminates noise on the fly! (unless you go with very low subsequent blur levels). as a result the outputs of algorithm is very 
// predictible and easy to work with.
// 
// since the subsequent blurs close in on details of the image, i decided to call this algorithm "autofocus".
// 
// 
// this file contains some other experiments regarding the mechanism described above. mainly for utilizing it.
// i might have messed up the calculations a little bit while experimenting. but the algorithm is solid can be re-implemeted easily
//

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

// energy accumulation experiment
void applyImpulseField(
    Mat& field,
    const Point2f& center,
    const Point2f& dir, // normalized
    float radius,
    float strength, // how much energy can be moved
    float travelDistance, 
    bool falloff = true
) {
    int minX = std::max(0, int(center.x - radius));
    int maxX = std::min(field.cols - 1, int(center.x + radius));
    int minY = std::max(0, int(center.y - radius));
    int maxY = std::min(field.rows - 1, int(center.y + radius));

    float r2 = radius * radius;

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {

            float dx = x - center.x;
            float dy = y - center.y;
            float dist2 = dx * dx + dy * dy;

            if (dist2 > r2) continue;

            float dist = sqrt(dist2);

            float scale = 1.0f;
            if (falloff) {
                scale = 1.0f - dist / radius;
                if (scale < 0.0f) scale = 0.0f;
            }

            float& pixel = field.at<float>(y, x);

            float movedEnergy = std::min(pixel, strength * scale);

            if (movedEnergy <= 0.0f)
                continue;

            // Remove energy
            pixel -= movedEnergy;

            // Fixed travel distance
            float targetX = x + dir.y * travelDistance;
            float targetY = y + dir.x * travelDistance;

            int tx = int(round(targetX));
            int ty = int(round(targetY));

            if (tx >= 0 && tx < field.cols &&
                ty >= 0 && ty < field.rows) {
                field.at<float>(ty, tx) += movedEnergy;
            }
        }
    }
}

inline float similarityL1(const Vec3b& a, const Vec3b& b)
{
    int diff =
        abs(a[0] - b[0]) +
        abs(a[1] - b[1]) +
        abs(a[2] - b[2]);

    return 1.0f - (diff / 765.0f);
}

inline Point2f normalize(const Point2f& v)
{
    float length = sqrt(v.x * v.x + v.y * v.y);

    if (length < 0.01f)
        return Point2f(0.0f, 0.0f);

    return Point2f(v.x / length, v.y / length);
}

inline float mapFloatSafe(float value, float inMin, float inMax, float outMin, float outMax)
{
    if (inMax - inMin == 0.0f)
        return outMin; 

    return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
}

// energy accumulation experiment 2
void propagateIfOppositeMulti(
    const Point& point,
    const Point2f& dir, // must be normalized
    const Mat& directionField, // CV_32FC2
    Mat& gray, // CV_32F or CV_8U
    int steps = 4, // how many pixels to sample
    float valueToAdd = 1.0f,
    float oppositeThreshold = -0.7f
) {
    CV_Assert(directionField.type() == CV_32FC2);
    CV_Assert(gray.channels() == 1);

    Vec2f avgDir(0.f, 0.f);
    int validCount = 0;

    // sample several pixels forward
    for (int i = 1; i <= steps; i++)
    {
        int nx = point.x + (int)round(dir.x * i);
        int ny = point.y + (int)round(dir.y * i);

        if (nx < 0 || ny < 0 || nx >= directionField.cols || ny >= directionField.rows)
            break;

        Vec2f sample = directionField.at<Vec2f>(ny, nx);

        float len = sqrt(sample[0] * sample[0] + sample[1] * sample[1]);
        if (len > 0.0001f)
        {
            avgDir += sample / len; // ensure normalized
            validCount++;
        }
    }

    // If no valid neighbors found
    if (validCount == 0)
        return;

    avgDir /= (float)validCount;

    float avgLen = sqrt(avgDir[0] * avgDir[0] + avgDir[1] * avgDir[1]);
    if (avgLen > 0.0001f)
        avgDir /= avgLen;

    int closestX = point.x + (int)round(dir.x);
    int closestY = point.y + (int)round(dir.y);

    if (closestX < 0 || closestY < 0 ||
        closestX >= directionField.cols || closestY >= directionField.rows)
        return;

    // dot product (are they opposite?)
    float dot = dir.x * avgDir[0] + dir.y * avgDir[1];

    if (dot <= oppositeThreshold)
    {
        if (gray.type() == CV_32F)
        {
            gray.at<float>(closestY, closestX) += valueToAdd;
        }
        else if (gray.type() == CV_8U)
        {
            int newVal = gray.at<uchar>(closestY, closestX) + valueToAdd;
            gray.at<uchar>(closestY, closestX) = saturate_cast<uchar>(newVal);
        }
    }
}

void GetStimuli(Mat& prev, Mat& curr, Mat& out, Mat& field)
{
    for (int i = 1; i < prev.cols - 1; i++)
    {
        for (int j = 1; j < prev.rows - 1; j++)
        {
            Point2f dir(0.0f, 0.0f);
            Vec3b& ownValue = prev.at<Vec3b>(j, i);
            for (int k = 0; k < 8; k++)
            {
                Vec3b& testValue = curr.at<Vec3b>(j + directions[k].y, i + directions[k].x);
                float similarity = similarityL1(ownValue, testValue);
                dir += directionsF[k] * similarity;
            }
            dir = normalize(dir);
            out.at<Vec3b>(j, i) = Vec3b((int)mapFloatSafe(dir.x, -1.0f, 1.0f, 0.0f, 255.0f), (int)mapFloatSafe(dir.y, -1.0f, 1.0f, 0.0f, 255.0f), 0);
            //out.at<Vec2f>(j, i) = Vec2f(dir.x, dir.y);
            //if (dir.x != 0.0f || dir.y != 0.0f)
            //{
            //    //applyImpulseField(field, Point2f(j, i), dir, 2.0f, 2.0f, 2.0f, true);
            //    Point propPoint(i, j);
            //    propagateIfOppositeMulti(propPoint, dir, out, field, 2, 30.0f);
            //}   
        }
    }
}

int main() 
{
    Mat previous = imread("autofocustest1/8.png");
    Mat out(previous.rows, previous.cols, CV_8UC3, Scalar(0, 0, 0));
    Mat field(400, 400, CV_8UC1);
    field.setTo(0);
    for (int i = 7; i >= 0; i--)
    {
        Mat current = imread("autofocustest1/" + to_string(i) + ".png");
        GetStimuli(previous, current, out, field);
        Mat display;
        imshow("out", out);
        //field.convertTo(display, CV_8U);
        //imshow("Field", display);
        waitKey(0);
        previous = current;
    }

    /*Point2f dir(0.8f, -0.8f);
    float len = sqrt(dir.x * dir.x + dir.y * dir.y);
    dir /= len;

    for (int i = 0; i < 1000; i++)
    {
        applyImpulseField(field, Point2f(300 - i, 200), dir, 5.0f, 20.0f, 5.0f, true);
        Mat display;
        field.convertTo(display, CV_8U);
        imshow("Field", display);
        waitKey(0);
    }*/
}
