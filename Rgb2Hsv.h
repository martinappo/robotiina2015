#pragma once
#include "types.h"
class Rgb2Hsv :
	public cv::ParallelLoopBody
{
public:
private:
	cv::Mat img;
	cv::Mat& retVal;

public:
	Rgb2Hsv(cv::Mat inputImgage, cv::Mat& outImage)
		: img(inputImgage), retVal(outImage){}

	virtual void operator()(const cv::Range& range) const
	{
		for (int i = range.start; i < range.end; i++)
		{
			float b = img.data[i];
			float g = img.data[i + 1];
			float r = img.data[i + 2];

			b /= 255;
			g /= 255;
			r /= 255;

			float h = b;
			float s = g;
			float v = r;

			float K = 0.f;
			float tmp;
			if (g < b)
			{
				//std::swap(g, b);
				tmp = g; g = b; b = tmp;
				K = -1.f;
			}
			float min_gb = b;
			if (r < g)
			{
				//std::swap(r, g);
				tmp = r; r = g; g = tmp;
				K = -2.f / 6.f - K;
				float min_gb = g < b ? g : b;
			}

			float chroma = r - min_gb;
			h = /*fabs*/(K + (g - b) / (6.f * chroma + 1e-20f));
			if (h < 0) h = -h;
			s = chroma / (r + 1e-20f);
			v = r;


			retVal.data[i] = h;
			retVal.data[i + 1] = s;
			retVal.data[i + 2] = v;

		}
	}
};

void rgb2Hsv(cv::Mat img, cv::Mat& retVal) {
	Rgb2Hsv tmp(img, retVal);
	cv::parallel_for_(cv::Range(0, img.total()), tmp);
}