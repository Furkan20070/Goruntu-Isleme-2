// opencv_contexts.cpp
// Minimal working example of the event-driven impulse/context system
// Compile: g++ opencv_contexts.cpp -std=c++17 `pkg-config --cflags --libs opencv4` -O2 -o contexts
// Run: ./contexts <edge_image.png>

#include <opencv2/opencv.hpp>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <cstdint>
#include <iostream>

using namespace cv;
using namespace std;

// this is basically memory approach on steroids
// running this script you can observe how our visual neurons are actually evolving and learning !!
// NOT COMPLETE!! lots of things to do. i might scrap this and move on to the more supervised approach. but it still gave good insight about potentials
// of memory approach.

// utility: pack (x,y) into 64-bit key
inline int64_t keyXY(int x, int y) 
{
    return (int64_t(x) << 32) | (uint32_t)y;
}

// Simple impulse event
struct Impulse 
{
    int x, y;
    float strength; // e.g. edge intensity
    Impulse(int _x, int _y, float s = 1.0f) : x(_x), y(_y), strength(s) {}
};

struct TinyMLP
{
    vector<Mat> weights;
    vector<Mat> biases;
    float learningRate = 0.0001f;

    TinyMLP(const vector<int>& layers)
    {
        for (size_t i = 1; i < layers.size(); ++i)
        {
            weights.push_back(Mat(layers[i], layers[i - 1], CV_32F));
            biases.push_back(Mat(layers[i], 1, CV_32F));
            randu(weights.back(), -0.5f, 0.5f);
            randu(biases.back(), -0.5f, 0.5f);
        }
    }

    // Forward pass with activation cache (for training)
    float forward(const Mat& in, vector<Mat>& activations, vector<Mat>& zs) const
    {
        Mat a = in.clone();
        activations.clear();
        zs.clear();
        activations.push_back(a);

        for (size_t i = 0; i < weights.size(); ++i)
        {
            Mat z = weights[i] * a + biases[i];
            zs.push_back(z.clone());
            if (i < weights.size() - 1)
            {
                max(z, 0, a); // ReLU
            }
            else
            {
                Mat negZ;
                multiply(z, Scalar(-1), negZ);
                Mat expZ;
                exp(negZ, expZ);
                a = 1.0f / (1.0f + expZ); // sigmoid
                //a = 2.0 / (1.0 + expZ) - 1.0; // tanh approx
            }
            activations.push_back(a.clone());
        }
        return a.at<float>(0);
    }

    // Forward-only evaluation
    float predict(float x, float y) const
    {
        Mat in = (Mat_<float>(2, 1) << x, y);
        vector<Mat> acts, zs;
        return forward(in, acts, zs);
    }

    // Online learning step (Hebbian-like update)
    void trainStep(float x, float y, float target)
    {
        Mat in = (Mat_<float>(2, 1) << x, y);
        vector<Mat> acts, zs;
        float output = forward(in, acts, zs);

        float error = target - output;
        float delta = error * output * (1 - output); // sigmoid derivative

        Mat d = (Mat_<float>(1, 1) << delta);
        for (int i = (int)weights.size() - 1; i >= 0; --i)
        {
            Mat gradW = d * acts[i].t();
            weights[i] += learningRate * gradW;
            biases[i] += learningRate * d;

            if (i > 0)
            {
                Mat newD = weights[i].t() * d;
                Mat reluMask = zs[i - 1] > 0;
                Mat reluMaskF;
                reluMask.convertTo(reluMaskF, CV_32F);
                newD = newD.mul(reluMaskF);
                d = newD;
            }
        }
    }
};

struct Context 
{
    Point2i center; // absolute center in the image where this context is placed
    float threshold = 0.85f; // firing threshold 
    TinyMLP listenerField;
    float activeListeners = 0.0f;
    float activation = 0.0f;
    float radius = 13.0f; // area around center for impulses

    Context(Point2f c) : center(c), listenerField({ 2, 16, 16, 1 }) // MLP architecture 
    {

    }

    // Process impulse at given location (dx, dy relative to center)
    void onImpulse(const Impulse& impulse)
    {
        float dx = impulse.x - center.x;
        float dy = impulse.y - center.y;

        float distance = sqrt(dx * dx + dy * dy);
        if (distance > radius)
            return; // ignore far impulses

        float response = listenerField.predict(dx / radius, dy / radius);
        activeListeners += response;

        // Simple local adaptation
        float xNorm = dx / radius;
        float yNorm = dy / radius;
        float lineThickness = 1.0f; // idk

        // horizontal line target
        float target = (exp(-(yNorm * yNorm) / (2 * lineThickness * lineThickness)) * 2.0f - 1.0f) * impulse.strength;
        listenerField.trainStep(xNorm, yNorm, target);
    }

    // Visualization
    Mat VisualizeField(int size = 100) const
    {
        Mat img(size, size, CV_32F);
        for (int y = 0; y < size; ++y)
        {
            for (int x = 0; x < size; ++x)
            {
                float nx = (x - size / 2) / float(size / 2);
                float ny = (y - size / 2) / float(size / 2);
                float val = listenerField.predict(nx, ny);
                img.at<float>(y, x) = val;
            }
        }
        Mat out;
        img.convertTo(out, CV_8U, 255.0);
        applyColorMap(out, out, COLORMAP_JET);
        return out;
    }

    bool fired() const { return activation >= threshold; }
    //bool fired() const { return activeListeners / (float)listeners.size() >= threshold; }
};

int main() 
{
    /*if (argc < 2) {
        return 1;
    }*/

    Mat edge = imread("edge.png", IMREAD_GRAYSCALE);
    if (edge.empty()) { cerr << "Failed to open image\n"; return 1; }

    // threshold to binary impulses (white = edge)
    Mat bin;
    threshold(edge, bin, 128, 255, THRESH_BINARY);

    vector<Context> contexts;
    contexts.emplace_back(Point2f(14, 141));

    Mat vis; cvtColor(edge, vis, COLOR_GRAY2BGR);

    Mat field1 = contexts[0].VisualizeField();
    imshow("field", field1);
    waitKey(0);
    for (int i = 0; i < bin.cols; i++)
    {
        for (int j = 0; j < bin.rows; j++)
        {
            for (auto& ctx : contexts)
            {
                double dist = cv::norm(ctx.center - Point2i(i, j));
                if (dist > ctx.radius)
                    continue;
                //Impulse imp(i, j, (ctx.radius - (float)dist) / ctx.radius);
                Impulse imp(i, j, bin.at<uchar>(j, i) ? 1.0f : 0.0f);
                ctx.onImpulse(imp);
                cout << "Impulse fired at: " << i << " " << j << endl;

                vis = Scalar(0, 0, 0);
                cvtColor(bin, vis, COLOR_GRAY2BGR);
                vis.at<Vec3b>(j, i) = Vec3b(0, 0, 255);
                if (i > 26) // just for seeing how our context neurons evolve over time (controlled with keypress)
                {
                    Mat field = contexts[0].VisualizeField();
                    imshow("field", field);
                    imshow("test", vis);
                    waitKey(0);
                    i = 0;
                }
            }
        }
    }
}
