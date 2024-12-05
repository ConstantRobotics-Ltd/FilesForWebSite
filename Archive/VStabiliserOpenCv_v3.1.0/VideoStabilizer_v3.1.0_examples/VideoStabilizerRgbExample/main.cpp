#include <iostream>
#include <opencv2/opencv.hpp>
#include <VideoStabilizer.h>


// Entry point.
int main(void)
{
    // Init video source.
    cv::VideoCapture cap;
    if (!cap.open("test.mp4"))
        return -1;

    // Get video frame size.
    int frameWidth = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int frameHeight = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);

    // Init images.
    cv::Mat opencvSrcBgrFrame(cv::Size(frameWidth, frameHeight), CV_8UC3);
    cv::Mat opencvDstBgrFrame(cv::Size(frameWidth, frameHeight), CV_8UC3);
    cv::Mat opencvSrcRgbFrame(cv::Size(frameWidth, frameHeight), CV_8UC3);
    cv::Mat opencvDstRgbFrame(cv::Size(frameWidth, frameHeight), CV_8UC3);
    cr::vsource::Frame srcRgbFrame(frameWidth, frameHeight, (uint32_t)cr::vsource::ValidFourccCodes::RGB24);
    cr::vsource::Frame dstRgbFrame(frameWidth, frameHeight, (uint32_t)cr::vsource::ValidFourccCodes::RGB24);

    // Create video stabilizer object.
    cr::vstab::VideoStabilizer videoStabilizer;

    // Set video stabilizer parameters.
    videoStabilizer.setProperty(cr::vstab::VideoStabilizerProperty::SCALE_FACTOR, 2);
    videoStabilizer.setProperty(cr::vstab::VideoStabilizerProperty::X_OFFSET_LIMIT, 150);
    videoStabilizer.setProperty(cr::vstab::VideoStabilizerProperty::Y_OFFSET_LIMIT, 150);

    // Main loop.
    while (true)
    {
        // Capture next video frame.
        cap >> opencvSrcBgrFrame;
        if (opencvSrcBgrFrame.empty())
        {
            // If we have video we can set initial video position.
            cap.set(cv::CAP_PROP_POS_FRAMES, 1);
            continue;
        }

        // Convert BGR pixel format to RGB.
        cv::cvtColor(opencvSrcBgrFrame, opencvSrcRgbFrame, cv::COLOR_BGR2RGB);

        // Copy video frame data from OpenCV image to image.
        memcpy(srcRgbFrame.data, opencvSrcRgbFrame.data, srcRgbFrame.size);

        // Calculate trasformation params.
        if (!videoStabilizer.calculateTransformationParams(srcRgbFrame))
            std::cout << "Transformation params not calculated" << std::endl;
        // Implement trasformation params.
        if (!videoStabilizer.implementTransformationParams(srcRgbFrame, dstRgbFrame))
            std::cout << "Transformation params not implemented" << std::endl;

        // Copy image to OpenCV image.
        memcpy(opencvDstRgbFrame.data, dstRgbFrame.data, dstRgbFrame.size);

        // Convert RGB pixel format to BGR.
        cv::cvtColor(opencvDstRgbFrame, opencvDstBgrFrame, cv::COLOR_BGR2RGB);

        // Show video.
        cv::imshow("SOURCE VIDEO", opencvSrcBgrFrame);
        cv::imshow("RESULT VIDEO", opencvDstBgrFrame);

        // Process keyboard events.
        switch (cv::waitKey(1))
        {
        case 27: // ESC - exit.
            exit(0);
        case 32: // SPACE - reset video stabilizer.
            videoStabilizer.executeCommand(cr::vstab::VideoStabilizerCommand::RESET);
            break;
        }
    }

    return 1;
}
