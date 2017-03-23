/*example C/C++ program to process video file or camera capture using opencv
  and serve the data over http using libevent evhttp
  feel free to change this code
  me : hello@arifnurkhoirudin.com
*/

#include "manciksayang.h"

int main(int argc, char* argv[])
{
  int real = REAL_DISTANCEPOINT;

  if (argc < 3){
    printf("usage : ./main [file/device] [logname]\n");
    printf("example from camera: ./main 0 thislog\n");
    printf("example from file: ./main /test/vid.mp4 experiment3\n");
    return -1;
  }

  // capture from camera
  // VideoCapture cap(atoi(argv[1]));

  //capture from file
  VideoCapture cap(argv[1]);

  if ( !cap.isOpened() ) {
       cout << "Cannot open the video file" << endl;
       return -1;
  }

  thread http(http_serving);
  Mat bgr_image;
  Mat orig_image;
	Mat hsv_image;
	Mat lower_red_hue_range;
	Mat upper_red_hue_range;
	Mat red_hue_image;

  double fps = cap.get(CV_CAP_PROP_FPS); //get the frames per seconds of the video
  cout << "Frame per seconds : " << fps << endl;
  double count = cap.get(CV_CAP_PROP_FRAME_COUNT); //get the frame count
  cout << "Frame total : " << count << endl;

	namedWindow("Combined threshold images", WINDOW_AUTOSIZE);
	namedWindow("Detected red circles on the input image", WINDOW_AUTOSIZE);

  FILE *fp;
  char filename[128];
  time_t ti = time(NULL);
  struct tm tmi = *localtime(&ti);
  sprintf(filename, "log/%d-%d-%d_%d:%d:%d_%s",tmi.tm_year + 1900, tmi.tm_mon + 1, tmi.tm_mday,
                                        tmi.tm_hour, tmi.tm_min, tmi.tm_sec, argv[2]);
  // printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
  // tm.tm_hour, tm.tm_min, tm.tm_sec);
  fp = fopen(filename,"a");
  int i;
  while(1){
    bool bSuccess = cap.read(bgr_image); // read a new frame from video
    if (!bSuccess){
      cout << "Cannot read the frame from video file" << endl;
      break;
    }


    orig_image = bgr_image.clone();
    medianBlur(bgr_image, bgr_image, 3);
    cvtColor(bgr_image, hsv_image, COLOR_BGR2HSV);

    inRange(hsv_image, Scalar(0, 100, 100),
            Scalar(10, 255, 255), lower_red_hue_range);
    inRange(hsv_image, Scalar(160, 100, 100),
            Scalar(179, 255, 255), upper_red_hue_range);
    // inRange(hsv_image, Scalar(160, 100, 100), Scalar(179, 255, 255), red_hue_image);

    // Combine the above two images
    addWeighted(lower_red_hue_range, 1.0,
                upper_red_hue_range, 1.0, 0.0, red_hue_image);
    GaussianBlur(red_hue_image, red_hue_image, Size(9, 9), 2, 2);


    /*IF THERE ARE TOO MANY NOISE ENABLE THIS*/
    // //morphological opening (removes small objects from the foreground)
    // erode(red_hue_image, red_hue_image, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
    // dilate(red_hue_image, red_hue_image, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
    // // //morphological closing (removes small holes from the foreground)
    // dilate(red_hue_image, red_hue_image, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
    // erode(red_hue_image, red_hue_image, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

    vector<Vec3f> circles;
    vector<Point> pts;
    HoughCircles(red_hue_image, circles,
                 CV_HOUGH_GRADIENT, 1, red_hue_image.rows/4, 100, 20, 5, 0);

    int radius;

    //must have 3 circles
    if(circles.size() == 3) {
      radius = round(circles[0][2]);
      for(size_t current_circle = 0; current_circle < circles.size(); ++current_circle) {
      	Point center(round(circles[current_circle][0]), round(circles[current_circle][1]));
        pts.push_back(center);
      }

      // sort vector using myobject as comparator
      std::sort(pts.begin(), pts.end(), myobject);
      circle(orig_image, pts[1], radius, Scalar(0, 255, 0), 2);

      int ytemp;

      m.lock();
      //SET THE RATION ON 5th LOOP
      if (i == 5) setzero(pts[1].x, pts[1].y, pts[0].x, pts[2].x, real);
      xdata=static_cast<int>((abs(pts[1].x - xfirst) / magic));
      ytemp=static_cast<int>((abs(pts[1].y - yfirst) / magic));
      ydata=ytemp *-1;
      m.unlock();
      printf("x == %d px --> %d ||",pts[1].x, xdata);
      printf("y == %d px --> %d\n",pts[1].y, ydata);
      //time
      time_t t = time(NULL);
      struct tm tm = *localtime(&t);

      fprintf(fp, "%d-%d-%d;%d:%d:%d;%d;%d\n",tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                                              tm.tm_hour, tm.tm_min, tm.tm_sec,xdata,ydata);
      i++;
    }

    imshow("Combined threshold images", red_hue_image);
    imshow("Detected red circles on the input image", orig_image);

    if(waitKey(30) == 27) {
      cout << "esc key is pressed by user" << endl;
      break;
    }
  }
  fclose(fp);
  http.join();
  return 0;
}



void notfound (struct evhttp_request *req, void *params) {
  struct evbuffer *evb = NULL;

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  // sprintf(filename, "log/%d-%d-%d_%d:%d:%d_%s",tmi.tm_year + 1900, tmi.tm_mon + 1, tmi.tm_mday,
  //                                       tmi.tm_hour, tmi.tm_min, tmi.tm_sec, argv[2]);

  evb = evbuffer_new ();
  evbuffer_add_printf(evb, "{\"x\": %d,\"y\": %d,\"time\":\"%d:%d:%d\"}",xdata,ydata,tm.tm_hour, tm.tm_min, tm.tm_sec);
  evhttp_add_header (evhttp_request_get_output_headers (req), "Access-Control-Allow-Origin", "*");
  evhttp_add_header (evhttp_request_get_output_headers (req), "Content-Type", "application/json;");
  // evhttp_add_header (evhttp_request_get_output_headers (req), "Content-Type", "text/html;");
  evhttp_send_reply(req, 200, "OK", evb);
  if (evb) evbuffer_free (evb);
}

void http_serving() {
	struct event_base *ebase;
	struct evhttp *server;

	// Create a new event handler
	ebase = event_base_new ();;
	// Create a http server using that handler
	server = evhttp_new (ebase);
	// Limit serving GET requests
	evhttp_set_allowed_methods (server, EVHTTP_REQ_GET);
	// Set the callback for anything not recognized
	evhttp_set_gencb (server, notfound, 0);
	// Listen locally on port 6777
	if (evhttp_bind_socket (server, "0.0.0.0", PORT) != 0){
    printf("%s\n","bind error");
    return;
  }
	// Start processing queries
	event_base_dispatch(ebase);
	// Free up stuff
	evhttp_free (server);
	event_base_free (ebase);
}


void  setmagic(int x1,int x2,int rl) {
    printf("%s\n","set magic" );
    magic = (double)abs(x1 - x2) / (double)rl;
}

void setzero(int x, int y, int x1, int x2, int reals){
    printf("%s\n","set zero now");
    xfirst = x;
    yfirst = y;
    printf("%d -- %d\n", xfirst, yfirst);
    setmagic(x1, x2, reals);
}
