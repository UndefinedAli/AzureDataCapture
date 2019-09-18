#pragma comment(lib, "k4a.lib")
#include <stdio.h>
#include <k4a/k4a.h>
#include <k4arecord/record.h>
#include <k4arecord/playback.h>
#include <iostream>
#include <stdlib.h>
#include <opencv2/core/core.hpp>  
#include <opencv2\opencv.hpp>
#include <opencv2/highgui/highgui.hpp>  

using namespace cv;
using namespace std;

void writeImages(vector<Mat>, char*);


int main()
{
	k4a_capture_t capture = NULL;

	const uint32_t device_count = k4a_device_get_installed_count();

	if (device_count == 0)
	{
		printf("No K4A devices found\n");
		return 0;
	}

	k4a_device_t device = NULL;
	//k4a_device_close(device);
	if (K4A_RESULT_SUCCEEDED != k4a_device_open(K4A_DEVICE_DEFAULT, &device))
	{
		printf("Failed to open device\n");
		return 0;
	}


	k4a_device_configuration_t config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
	config.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
	config.color_resolution = K4A_COLOR_RESOLUTION_1080P;
	config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
	config.camera_fps = K4A_FRAMES_PER_SECOND_30;
	//If the synchronization identity is not added here, the multi-frame collection will have a problem
	config.synchronized_images_only = true;

	
	if (K4A_RESULT_SUCCEEDED != k4a_device_start_cameras(device, &config))
	{
		printf("Failed to start device\n");
		return 0;
	}

	k4a_image_t irImage;
	k4a_image_t rgbImage;
	k4a_image_t depthImage;


	Mat color_frame;
	Mat depth_frame;
	Mat ir_frame;

	vector<Mat> imageBuffer;
	vector<Mat> IRBuffer;
	vector<Mat> RGBBuffer;
	int count = 0;
	const int32_t TIMEOUT_IN_MS = 1000;
	while (true)
	{
		
		// Get a depth frame
		switch (k4a_device_get_capture(device, &capture, TIMEOUT_IN_MS))
		{
		case K4A_WAIT_RESULT_SUCCEEDED:
			count++;
			printf("Capturing! NO.%d\n",count);
			break;
		case K4A_WAIT_RESULT_TIMEOUT:
			printf("Timed out waiting for a capture\n");
			continue;
			break;
		case K4A_WAIT_RESULT_FAILED:
			printf("Failed to read a capture\n");
                        cv::destroyAllWindows();
			k4a_device_close(device);
			std::system("pause");
			return 0;
		}



		rgbImage = k4a_capture_get_color_image(capture);
		depthImage = k4a_capture_get_depth_image(capture);
		irImage = k4a_capture_get_ir_image(capture);
		
		color_frame = cv::Mat(k4a_image_get_height_pixels(rgbImage), k4a_image_get_width_pixels(rgbImage), CV_8UC4, k4a_image_get_buffer(rgbImage));
		depth_frame = cv::Mat(k4a_image_get_height_pixels(depthImage), k4a_image_get_width_pixels(depthImage), CV_16U, k4a_image_get_buffer(depthImage));
		ir_frame = cv::Mat(k4a_image_get_height_pixels(irImage), k4a_image_get_width_pixels(irImage), CV_16U, k4a_image_get_buffer(irImage));

		cv::imshow("color", color_frame);
		cv::imshow("depth", depth_frame);
		cv::imshow("ir", ir_frame);

		k4a_image_release(rgbImage);
		k4a_image_release(depthImage);
		k4a_image_release(irImage);

		color_frame.release();
		depth_frame.release();
		ir_frame.release();
		
		//Must release capture here!
		k4a_capture_release(capture);

		if (waitKey(1) == 27)
			break;

	}
	cv::destroyAllWindows();
	k4a_device_close(device);
	std::system("pause");
	return 0;
}



void writeImages(vector<Mat> imageData,char* imageCat)
{
	printf("Write Images to disk...\n");
	printf("Num of Image:%d\n", imageData.size());
	for (int i = 0; i < imageData.size(); i++)
	{
		char fileName[128];
		sprintf(fileName, "%s_%d.png", imageCat, i);
		cv::imwrite(fileName, imageData[i]);
	}
	printf("Finish\n");
}
