#pragma once

#include <opencv2/opencv.hpp>
#include <Windows.h>

namespace DetectionLib {

    struct Target {
        POINT pos = { -1, -1 };
        bool valid = false;
    };

    cv::Mat CaptureFOVRegion(int fov, int& outOffsetX, int& outOffsetY);

    Target FindTargetInRegion(
        const cv::Mat& region,
        int fov,
        int offsetX,
        int offsetY,
        const cv::Scalar& hsvLower,
        const cv::Scalar& hsvUpper
    );

    void MoveMouseRelativeSmooth(int dx, int dy, int smoothing);
}
