#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <string>
#include <string.h>

using namespace cv;

// The recipe of GIMP/Artistic Filters/Photocopy:
// http://www.imagemagick.org/discourse-server/viewtopic.php?t=14441

int main( int argc, char** argv )
{
   if( argc != 2) {
      std::cout <<"USAGE: " << std::string(basename(argv[0])) << " <filename>" << std::endl;
      return -1;
   }

   Mat image;
   image = imread(argv[1], CV_LOAD_IMAGE_COLOR);

   if( !image.data ) {
      std::cout <<  "Cannot not open image" << std::endl;
      return -1;
   }

   namedWindow( "Display window", WINDOW_AUTOSIZE );
   imshow( "Display window", image );

   waitKey(0);
   return 0;
}
