//
//  main.cpp
//  PedestrianRemoval
//
//  Created by Shreyash Pandey on 10/23/17.
//  Copyright © 2017 Shreyash Pandey. All rights reserved.
//

#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include <vector>
#include <stdio.h>
#include "nms.h"
#include "opencv2/photo.hpp"
using namespace cv;
using namespace std;

Mat image, gray, cedge, edge, gray_edge, croppedImage, croppedImage_copy;
int edgeThresh = 1;

void onMouse(int evt, int x, int y, int flags, void* param) {
    
    if(evt == CV_EVENT_LBUTTONDOWN) {
        std::vector<cv::Point>* ptPtr = (std::vector<cv::Point>*)param;
        ptPtr->push_back(cv::Point(x,y));
    }
}
static void onTrackbar(int, void*)
{
    blur(gray_edge, edge, Size(3,3));
    // Run the edge detector on grayscale
    Canny(edge, edge, edgeThresh, edgeThresh*3, 3);
    cedge = Scalar::all(0);
    croppedImage.copyTo(cedge, edge);
    imshow("Edge map", cedge);
}
static void help()
{
    printf("\nThis sample demonstrates Pedestrian detection\n"
           "Call:\n"
           "    /.edge [image_name -- Default is ../data/fruits.jpg]\n\n");
}
const char* keys =
{
    "{help h||}{@image |/Users/shreyashpandey/Downloads/pedestrians.jpg|input image name}"
};
int main( int argc, const char** argv )
{
    CascadeClassifier detectorBody;
    CascadeClassifier detectorLower;
    CascadeClassifier detectorUpper;
    
    string fullCascadeName = "/Users/shreyashpandey/acads/PedestrianRemoval/PedestrianRemoval/full_cascade.xml";
    string upperCascadeName = "/Users/shreyashpandey/acads/PedestrianRemoval/PedestrianRemoval/upper_cascade.xml";
    string lowerCascadeName = "/Users/shreyashpandey/acads/PedestrianRemoval/PedestrianRemoval/lower_cascade.xml";
    
    detectorBody.load(fullCascadeName);
    detectorUpper.load(upperCascadeName);
    detectorLower.load(lowerCascadeName);
    
    
    vector<Rect> human;
    vector<Rect> upperBody;
    vector<Rect> lowerBody;
    CommandLineParser parser(argc, argv, keys);
    if (parser.has("help"))
    {
        help();
        return 0;
    }
    string filename = parser.get<string>(0);
    image = imread(filename, 1);
    if(image.empty())
    {
        printf("Cannot read image file: %s\n", filename.c_str());
        help();
        return -1;
    }
   
    cvtColor(image, gray, COLOR_BGR2GRAY);
    // Create a window
    namedWindow("Detections", 1);
    
    detectorBody.detectMultiScale(gray, human, 1.1, 1, 0 | 1, Size(1,1), Size(1000, 3000));
    detectorUpper.detectMultiScale(gray, upperBody, 1.1, 1, 0 | 1, Size(1, 1), Size(1000, 3000));
    detectorLower.detectMultiScale(gray, lowerBody, 1.1, 1, 0 | 1, Size(1, 1), Size(1000, 3000));
    Mat inpaintMask = Mat::zeros(gray.size(), CV_8U);
    Mat inpaintedImage;
    Mat original_img = image.clone();
    
    std::vector<cv::Rect> srcRects;
    std::vector<cv::Rect> resRects;
    
    if (human.size() > 0) {
        for (int gg = 0; gg < human.size(); gg++) {
            srcRects.push_back(cv::Rect(human[gg].tl(), human[gg].br()));
        }
    }
    
    if (upperBody.size() > 0) {
        for (int gg = 0; gg < upperBody.size(); gg++) {
            srcRects.push_back(cv::Rect(upperBody[gg].tl(), upperBody[gg].br()));
        }
    }
    if (lowerBody.size() > 0) {
        for (int gg = 0; gg < lowerBody.size(); gg++) {
            srcRects.push_back(cv::Rect(lowerBody[gg].tl(), lowerBody[gg].br()));
        }
    }
    /*for (auto r : srcRects)
    {
        cv::rectangle(image, r, cv::Scalar(0, 255, 255), 2);
    }*/
    nms(srcRects, resRects, 0.3f, 1);
    for (auto r : resRects)
    {
        //printf("Accessing rectangle: %d %d\n",r.x,r.y);
        cv::rectangle(image, r, cv::Scalar(0, 255, 0), 2);
    }
    std::vector<cv::Point> points;
    cv::namedWindow("Output Window");
    cv::imshow("Output Window", image);
    printf("Enter 1 for automatic removal, 2 for single click removal, 3 for two click removal.\n");
    int ch;
    scanf("%d",&ch);
    cv::setMouseCallback("Output Window", onMouse, (void*)&points);
    printf("Press Enter (on the output window) to proceed...\n");
    waitKey(0);
    
   
    switch(ch) {
        case 1  :
            //automatically remove a person from the image
            printf("Randomly removes a person from the image.\n");
            for (auto r : resRects)
            {
                cv::Rect myROI(r.tl(), r.br());
                croppedImage = image(myROI);
                rectangle(inpaintMask, r.tl(), r.br(), Scalar(255,255,255), CV_FILLED, 8, 0);
                
                inpaint(original_img, inpaintMask, inpaintedImage, 2, INPAINT_TELEA);
                
                
                GaussianBlur(croppedImage, croppedImage, Size(15, 15), 0, 0 );//applying Gaussian filter
                inpaintedImage(myROI) = croppedImage;
                imshow("Result", inpaintedImage);
                printf("Press Enter (on the output window) to end.\n");
                waitKey(0);
                
                break;
            }
           
            break;
        case 2  :
            //remove a person with a single click
            printf("Please click at the person you want to remove.\n");
            if (points.size() == 1) //we have 1 point
            {
                printf("%d %d",points[0].x,points[0].y);
            }
            else {
                printf("Error! Please click at one point.\n");
                break;
            }
            for (auto r : resRects)
            {
                if (r.tl().x < points[0].x && r.tl().y < points[0].y && r.br().x > points[0].x && r.br().y > points[0].y){
                    cv::Rect myROI(r.tl(), r.br());
                    croppedImage = image(myROI);
                    rectangle(inpaintMask, r.tl(), r.br(), Scalar(255,255,255), CV_FILLED, 8, 0);
                    
                    inpaint(original_img, inpaintMask, inpaintedImage, 2, INPAINT_TELEA);
                    
                    
                    GaussianBlur(croppedImage, croppedImage, Size(15, 15), 0, 0 );//applying Gaussian filter
                    inpaintedImage(myROI) = croppedImage;
                    imshow("Result", inpaintedImage);
                    printf("Press Enter (on the output window) to end.\n");
                    waitKey(0);
                    
                    break;
                }
                
            }
            
          
            break;
          
        case 3 : //remove a person with two clicks
            printf("Please click at the top left and bottom right corners of the person you want to remove.\n");
            if (points.size() == 2) //we have 2 points
            {
                for (auto it = points.begin(); it != points.end(); ++it)
                {
                    printf("%d %d",(*it).x,(*it).y);
                }
                
            }
            else{
                printf("Error! Please click at two points.\n");
                break;
            }
            cv::Rect myROI(points[0], points[1]);
            croppedImage = image(myROI);
            rectangle(inpaintMask, points[0], points[1], Scalar(255,255,255), CV_FILLED, 8, 0);
            
            inpaint(original_img, inpaintMask, inpaintedImage, 1, INPAINT_TELEA);
            
            
            GaussianBlur(croppedImage, croppedImage, Size(15, 15), 0, 0 );//applying Gaussian filter
            inpaintedImage(myROI) = croppedImage;
            imshow("Result", inpaintedImage);
            printf("Press Enter (on the output window) to end.\n");
            waitKey(0);
           
    }
    
    //Experiments that didn't work
    /*bilateralFilter(croppedImage_copy, croppedImage, 2, 3.0, 4.0);
    cedge.create(croppedImage.size(), croppedImage.type());
    cvtColor(croppedImage, gray_edge, COLOR_BGR2GRAY);
    // Create a window
    namedWindow("Edge map", 1);
    // create a toolbar
    createTrackbar("Canny threshold", "Edge map", &edgeThresh, 100, onTrackbar);
    // Show the image
    onTrackbar(0, 0);
    waitKey(0);
    
    imshow("Edges:", edge);
    waitKey(0);
    //Inpaint Now
    cv::copyMakeBorder(edge, edge, 1, 1, 1, 1, cv::BORDER_REPLICATE);
    
    //Fill mask with value 255
    cv::Point seed((points[1].x-points[0].x)/2,(points[1].y-points[0].y)/2);
    uchar fillValue = 255;
    cv::floodFill(croppedImage, edge, seed, cv::Scalar(255) ,0, cv::Scalar(), cv::Scalar(), 4| cv::FLOODFILL_FIXED_RANGE | (fillValue << 8));
    imshow("Mask:", edge);
    waitKey(0);
     */
    
    return 0;
}

