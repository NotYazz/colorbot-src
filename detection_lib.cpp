#include "detection_lib.h"
#include <cmath>
#include <thread>

namespace DetectionLib {

    cv::Mat CaptureFOVRegion(int fov, int& outOffsetX, int& outOffsetY) {
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        int regionSize = fov * 2;
        int centerX = screenWidth / 2;
        int centerY = screenHeight / 2;

        int left = centerX - fov;
        int top = centerY - fov;

        if (left < 0) left = 0;
        if (top < 0) top = 0;
        if (left + regionSize > screenWidth) regionSize = screenWidth - left;
        if (top + regionSize > screenHeight) regionSize = screenHeight - top;

        outOffsetX = left;
        outOffsetY = top;

        HDC hScreenDC = GetDC(NULL);
        HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = regionSize;
        bmi.bmiHeader.biHeight = -regionSize;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* pBits = nullptr;
        HBITMAP hBitmap = CreateDIBSection(hScreenDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
        HGDIOBJ oldBitmap = SelectObject(hMemoryDC, hBitmap);

        BitBlt(hMemoryDC, 0, 0, regionSize, regionSize, hScreenDC, left, top, SRCCOPY);

        cv::Mat mat(regionSize, regionSize, CV_8UC4, pBits);
        cv::Mat result = mat.clone();

        SelectObject(hMemoryDC, oldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);

        return result;
    }

    Target FindTargetInRegion(
        const cv::Mat& region,
        int fov,
        int offsetX,
        int offsetY,
        const cv::Scalar& hsvLower,
        const cv::Scalar& hsvUpper
    ) {
        Target out;
        if (region.empty()) return out;

        cv::Mat hsv;
        cv::cvtColor(region, hsv, cv::COLOR_BGR2HSV);

        cv::Mat mask;
        cv::inRange(hsv, hsvLower, hsvUpper, mask);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        int regionSize = region.cols;
        int centerX = regionSize / 2;
        int centerY = regionSize / 2;

        double closestDist = static_cast<double>(fov) + 1.0;
        cv::Point bestCentroid(-1, -1);

        for (const auto& contour : contours) {
            double area = cv::contourArea(contour);
            if (area > 100) {
                cv::Moments M = cv::moments(contour);
                if (M.m00 != 0) {
                    int cX = static_cast<int>(M.m10 / M.m00);
                    int cY = static_cast<int>(M.m01 / M.m00);
                    double dist = std::sqrt((cX - centerX) * (cX - centerX) + (cY - centerY) * (cY - centerY));
                    if (dist <= fov && dist < closestDist) {
                        closestDist = dist;
                        bestCentroid = cv::Point(cX, cY);
                    }
                }
            }
        }

        if (closestDist <= fov && bestCentroid.x != -1) {
            out.pos.x = bestCentroid.x + offsetX;
            out.pos.y = bestCentroid.y + offsetY;
            out.valid = true;
        }
        return out;
    }

    void MoveMouseRelativeSmooth(int dx, int dy, int smoothing) {
        if (smoothing < 1) smoothing = 1;
        float stepX = static_cast<float>(dx) / smoothing;
        float stepY = static_cast<float>(dy) / smoothing;

        float accumX = 0, accumY = 0;
        for (int i = 0; i < smoothing; ++i) {
            accumX += stepX;
            accumY += stepY;
            int moveX = static_cast<int>(std::round(accumX));
            int moveY = static_cast<int>(std::round(accumY));
            if (moveX != 0 || moveY != 0) {
                INPUT input = { 0 };
                input.type = INPUT_MOUSE;
                input.mi.dx = moveX;
                input.mi.dy = moveY;
                input.mi.dwFlags = MOUSEEVENTF_MOVE;
                SendInput(1, &input, sizeof(INPUT));
                accumX -= moveX;
                accumY -= moveY;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
