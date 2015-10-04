#pragma once
#include "BallPosition.h"

//http://stackoverflow.com/a/1402369/1299264
//!!! 	There appears to be a bug in kdNode2D constructor as provided by LiraNuna above : for (int i = 0; i<2; ++i) sons[i] = new kdNode2D(pointList + (i*halfLength), halfLength, depth + 1); The above code will not work for pointList arrays or vectors of odd length.It will ignore the last element

class kdNode2D
{
public:
	kdNode2D(BallPosition* pointList, int pointLength, int depth = 0);

	~kdNode2D()
	{
		for (int i = 0; i<2; ++i)
			delete sons[i];
	}

	/* Leave depth alone for outside code! */
	std::pair<unsigned, BallPosition*> nearest(const cv::Point &point, int depth = 0);

	union {
		struct {
			kdNode2D* left;
			kdNode2D* right;
		};

		kdNode2D* sons[2];
	};

	BallPosition* p;

};
