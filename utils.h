#pragma once
#include "image.h"
#include "descriptor.h"
#include "pyramid.h"
#include "descriptorfactory.h"
using namespace std;


pair<float, float> rotate(float x0, float y0, float x1, float y1, float angle);

int myProc(int a, int b);

shared_ptr<Image> Sobel(const Image& _im, EdgeMode _mode);

pair<int, int> findClosestPair(Descriptor _p, vector<Descriptor> _vec);

vector<pair<KeyPoint, KeyPoint>> FindMatches(vector<Descriptor> descs1, vector<Descriptor> descs2);

void DrawMatches(const Image &_im1, const Image &_im2, vector<pair<KeyPoint, KeyPoint>> _matches, const QString &_fileName);

void DrawBlobs(const Image& _im, const vector<KeyPoint>& _blobs, const QString &_fileName);

vector<Descriptor> findBlobs(const Image& _im, float T,  const QString &_fileName);
