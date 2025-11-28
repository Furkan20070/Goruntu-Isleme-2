#pragma once

// image processing element inspired by the v2 elements of human visual cortex.
// what it should do -- classify the edges and corners in an image in a noise-tolerant way. and also cheaply.
// what it shouldn't do -- classify an object or anything higher level than an edge or corners
// how it should do it? 
// event driven system design is the key - to avoid multiple passes of the image and redundant stacking.
// a strong and fast edge detection algorithm runs through the image first (see autofocus.cpp)
// a vertical scan column runs through the output, it classifies points of interest (more on this later) in the output as impulses.
// an impulse is the most basic event element and it spawns a v2 with following initializations:
// *bool direction - does the v2 have a direction? initially false, a single impulse cannot mean a direction.
// *Point2f direction - actual direction, decided after some impulses from the initial spawning impulse since a single impulse cannot give direction meaning.
// the hard part is to make this system work with noise. i think the best approach is to wait for some impulse data to appear (not just immediately decide)
// for determining the direction of v2.
// *head - initial spawn impulse location. 
// *tail - last impulse which have fallen to this v2's area of responsibility. the area of responsibility is determined as a small circular field 
// around the v2 at initial impulse, and as a small point area (or maybe longer) along direction vector of the tail after v2's direction is determined. this area of
// responsibility dynamic is very powerful against noise while keeping the system computationally efficent and event-driven. 
// when an impulse occurs, it can just query the spatial hash to see if it excites an area of responsibility for a v2. (it's VERY IMPORTANT to implement a spatial hash
// that is noise tolerant, i tried this with neural networks before (see opencv_contexts.cpp) but it seemed like overkill).
// tail and head parts of v2s also serve as places to quickly check for and spawn if needed for new v2s which brings us to:
// *corner points and impulses - head and tail attributes serves as a way to quickly highlight corner points.
// *max length  - maximum length this v2 can grow (max distance between head and tail). 
// remainder - a v2 should have a maximum length it can grow to. this will cause a problem though,
// most of the time, length of an edge wont be divided into defined maximum length perfectly, so we add this attribute to inform 
// the next layer of detection (v4) about varying lengths of edges. it's simple and effective, in fact it's so effective exact same 
// information mechanism is detected in human brain as well. this is also useful when the edge is smaller than the maximum length.
// 
class v2
{
};

