#include <stdio.h>
#include <opencv2/opencv.hpp>
using namespace cv;
int main(int argc, char** argv )
{
        if ( argc != 2 ){
                printf("usage: DisplayImage.out <Image_Path>\n");
                return -1;
        }
        Mat image,dst;
        image = imread( argv[1], 1 );
        if ( !image.data ){
                printf("No image data \n");
                return -1;
        }

	cvtColor(image, dst,COLOR_BGR2GRAY);
        //namedWindow("Display Image" );
        imshow("Display Image", dst);
        waitKey(0);
        return 0;
}
