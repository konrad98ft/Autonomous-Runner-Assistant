#include <opencv2/highgui/highgui.hpp> 
#include "opencv2/imgproc/imgproc.hpp" 
#include <string> 
#include <iostream> 
#include <algorithm>

using namespace cv;
using namespace std;

VideoCapture video;
int debug = 1; //0-any windows, 1-orginal+steeing,2-additonal debug windows,3-all windows
int prevSteering = 0;
int resize_x = 600; // window size
int resize_y = 400;

vector< vector<double> > change_lines_notation(vector< vector<double> > vect, vector <Vec4i> lines)
{
	//add all lines to vector and sort it
	for (size_t i = 0; i < lines.size(); i++)
	{
		int x1 = lines[i].val[0];
		int y1 = lines[i].val[1];
		int x2 = lines[i].val[2];
		int y2 = lines[i].val[3];
		//y=ax+b
		double a = ((double)y2 - (double)y1) / ((double)x2 - (double)x1);	//a=(x2-x1)/(y2-y1)
		double b = (double)y1 - a * (double)x1;	//b=y-ax

		vect[i][0] = a;
		vect[i][1] = b;
		//cout << "x1:" << x1 << " y1:" << y1 << " x2:" << x2 << " y2:" << y2 << "   a:" << a << "    b:" << b << endl;
	}
	sort(vect.begin(), vect.end());

	return vect;
}
vector< vector<double> > delete_wrong_lines(vector< vector<double> > vect)
{
	int current_size = vect.size();
	for (int i = 0; i < vect.size(); i++)
	{
		if (vect[i][0] > -0.07 && vect[i][0] < 0.07) //too low slope --> delete line
		{
			current_size -= 1;
			vect[i][0] = 0;
			vect[i][1] = 0;
		}
	}

	typedef vector< std::vector<double> > matrix;
	matrix blue_lines(current_size, std::vector<double>(2));

	int k = 0; //to count number of blue lines
	for (int i = 0; i < vect.size(); i++)
	{
		if (vect[i][0] != 0 && vect[i][1] != 0) //achieve only proper lines
		{
			blue_lines[k][0] = vect[i][0];
			blue_lines[k][1] = vect[i][1];
			k++;
		}
	}


	return blue_lines;
}
vector< vector<double> > generate_blue_lines(vector< vector<double> > vect)
{
	//count how many blue lines (lines connected with both trck lines edges) we have and define matrix for them
	int blue_lines_number = 1;	//how many lines we have at the beegening = 1 because if we have any lines that means there is minimum 1 line 	
	for (size_t i = 0; i < vect.size() - 1; i++)	//count how many lines have simillar slope (+- 0.1 it can be modified )
	{
		if (!(((vect[i + 1][0] + 0.02) > vect[i][0]) && ((vect[i + 1][0] - 0.020) < vect[i][0]))) blue_lines_number++;	
	
	}
	typedef vector< std::vector<double> > matrix;
	matrix blue_lines(blue_lines_number, std::vector<double>(2));

	//add slopes of similar lines
	vector <int> particular_line_numbers(blue_lines_number); //how many short lines belongs to blue line
	blue_lines[blue_lines_number - 1][0] += vect[0][0];	// in next loop 1st line distract from the 2nd so 1 must be assigned to cointaner before loop
	blue_lines[blue_lines_number - 1][1] += vect[0][1];
	particular_line_numbers[blue_lines_number - 1]++;
	for (size_t i = 0; i < vect.size() - 1; i++)	
	{
		if (!(((vect[i + 1][0] + 0.02) > vect[i][0]) && ((vect[i + 1][0] - 0.02) < vect[i][0])))
		{	//when new main line is detected
			blue_lines_number--;
			blue_lines[blue_lines_number - 1][0] += vect[i + 1][0];
			blue_lines[blue_lines_number - 1][1] += vect[i + 1][1];
			particular_line_numbers[blue_lines_number - 1]++;
		}
		else
		{	//when it is still the same main line
			particular_line_numbers[blue_lines_number - 1]++;
			blue_lines[blue_lines_number - 1][0] += vect[i + 1][0];
			blue_lines[blue_lines_number - 1][1] += vect[i + 1][1];
		}
	}

	//final blue_lines proccesing: get average slope of each blue line by dividing slopes sum by its number
	for (int i = 0; i < particular_line_numbers.size(); i++)	
	{
		blue_lines[i][0] /= (double)particular_line_numbers[i];
		blue_lines[i][1] /= (double)particular_line_numbers[i];
		//cout << "line " << i << " a:" << blue_lines[i][0] << " b:" << blue_lines[i][1] << endl;
	}


	return blue_lines;
}
vector< vector<double> > generate_lines_to_draw(vector< vector<double> > blue_lines,bool sign )
{
	typedef vector< std::vector<double> > matrix;

	sort(blue_lines.begin(), blue_lines.end());
	int plus_number = 0, minus_number = 0;
	for (int i = 0; i < blue_lines.size(); i++)
	{
		if (blue_lines[i][0] >= 0) plus_number++;
	}
	minus_number = blue_lines.size() - plus_number;

	if (sign == 1)//plus_lines
	{
		if (plus_number > 4) 
		{
			plus_number = 4;
		}
		else
		{
			if (plus_number % 2 != 0 && plus_number>1) plus_number -= 1;
			
		}
			
		matrix main_plus_blue_lines(plus_number, vector<double>(2));
	
		for (int i = 0;i< plus_number;i++)
		{
			main_plus_blue_lines[i] = blue_lines[blue_lines.size() - 1 - i];
		}
		if (plus_number > 1)
		{
			matrix main_plus_track_lines(plus_number / 2, vector<double>(2));
			for (int i = 0; i < plus_number / 2; i++)
			{
				main_plus_track_lines[i][0] = (main_plus_blue_lines[2 * i][0] + main_plus_blue_lines[2 * i + 1][0]) / 2;
				main_plus_track_lines[i][1] = (main_plus_blue_lines[2 * i][1] + main_plus_blue_lines[2 * i + 1][1]) / 2;
			}
			return main_plus_track_lines;
		}
		else
		{
			return main_plus_blue_lines;
		}

	}
	else
	{	//minus_lines
		
		if (minus_number > 4)
		{
			minus_number = 4;
		}
		else
		{
			if (minus_number % 2 != 0 && minus_number > 1) minus_number -= 1;

		}

		matrix main_minus_blue_lines(minus_number , vector<double>(2));

		for (int i = 0; i < minus_number; i++)
		{
			main_minus_blue_lines[i] = blue_lines[i];
		}
		matrix main_minus_track_lines(minus_number / 2, vector<double>(2));
		if(minus_number>1)
		for (int i = 0; i < minus_number / 2; i++)
		{
			main_minus_track_lines[i][0] = (main_minus_blue_lines[2 * i][0] + main_minus_blue_lines[2 * i + 1][0]) / 2;
			main_minus_track_lines[i][1] = (main_minus_blue_lines[2 * i][1] + main_minus_blue_lines[2 * i + 1][1]) / 2;
			return main_minus_track_lines;
		}
		else
		{
			return main_minus_blue_lines;
		}


		
	}

}
Mat prepareImage(Mat img_orginal)
{
	//creating ROI mask
	int img_X = img_orginal.cols;
	int img_Y = img_orginal.rows;
	Mat mask(img_Y, img_X, CV_8UC3, Scalar(0, 0, 0));
	Point points[6] = { Point(0,img_Y),
						Point(img_X, img_Y),
						Point(img_X, img_Y * 4 / 10),
						Point(img_X * 2 / 3, img_Y * 3 / 9),
						Point(img_X * 1 / 3, img_Y * 3 / 9),
						Point(0, img_Y * 4 / 10) };
	fillConvexPoly(mask, points, 6, Scalar(255, 255, 255), 8, 0);
	if (debug >= 3) namedWindow("Mask", 0);							
	if (debug >= 3) imshow("Mask", mask); resizeWindow("Mask", resize_x, resize_y);
	Mat img_masked;
	img_orginal.copyTo(img_masked, mask);
	if (debug >= 3) namedWindow("Masked_Orginal", 0);							
	if (debug >= 3) imshow("Masked_Orginal", img_masked); resizeWindow("Masked_Orginal", resize_x, resize_y);
	mask.release();

	//convert to gayscale
	Mat img_grayscale;
	cvtColor(img_masked, img_grayscale, COLOR_BGR2GRAY);
	img_masked.release();
	if (debug >= 2) namedWindow("IMG Grayscale", 0);							
	if (debug >= 2) imshow("IMG Grayscale", img_grayscale); resizeWindow("IMG Grayscale", resize_x, resize_y);

	//binary treshold
	threshold(img_grayscale, img_grayscale, 185, 255, THRESH_BINARY);
	if (debug >= 2) namedWindow("Treshold", 0);
	if (debug >= 2) imshow("Treshold", img_grayscale); resizeWindow("Treshold", resize_x, resize_y);
	blur(img_grayscale, img_grayscale, Size(10, 10));
	if (debug >= 3) namedWindow("Blured_Gray", 0);
	if (debug >= 3) imshow("Blured_Gray", img_grayscale); resizeWindow("Blured_Gray", resize_x, resize_y);

	//blur and treshold again
	blur(img_grayscale, img_grayscale, Size(10, 10));
	threshold(img_grayscale, img_grayscale, 40, 255, THRESH_BINARY);
	if (debug >= 2) namedWindow("2nd treshold", 0);
	if (debug >= 2) imshow("2nd treshold", img_grayscale); resizeWindow("2nd treshold", resize_x, resize_y);

	//detect edges
	Mat img_egdges;
	Canny(img_grayscale, img_egdges, 150, 200);
	if (debug >= 2) namedWindow("Image Canny", 0);
	if (debug >= 2) imshow("Image Canny", img_egdges); resizeWindow("Image Canny", resize_x, resize_y);

	return img_egdges; //return image after Canny
}
vector <Vec4i> detectLines(Mat img_egdges,Mat img_orginal)
{
	//find lines
	vector <Vec4i> lines;
	HoughLinesP(img_egdges, lines, 1, CV_PI / 180, 70, 50, 100);
	img_egdges.release();

	//draw detected lines
	Mat img_complex;
	Mat img_complex2;
	img_orginal.copyTo(img_complex);
	img_orginal.copyTo(img_complex2);
	for (int i = 0; i < lines.size(); i++)
	{
		Vec4i l = lines[i];
		line(img_complex, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255, 0, 0), 3, LINE_AA);
	}
	if (debug >= 2) namedWindow("Track_Complex", 0);
	if (debug >= 2) imshow("Track_Complex", img_complex); resizeWindow("Track_Complex", resize_x, resize_y);
	//------------------------delete wrong lines from the middle-----------------------------//
	for (int i = 0; i < lines.size(); i++)
	{	
		int x1 = lines[i].val[0];
		int y1 = lines[i].val[1];
		int x2 = lines[i].val[2];
		int y2 = lines[i].val[3];
		double a = ((double)y2 - (double)y1) / ((double)x2 - (double)x1);	//a=(x2-x1)/(y2-y1)
		double b = (double)y1 - a * (double)x1;	//b=y-ax

		int x = img_orginal.cols;
		int y = img_orginal.rows;
		if (a > -0.8 && a < 0.8) //if line slope is low
		{
			if ((x1 > x / 6 && x1 < 5 * x / 6) && (x2 > x / 6 && x2 < 5 * x / 6) && (y1 > y / 3) && (y2 > y / 3))//if this line is int hte middle
			{
				lines[i].val[0] = 0;
				lines[i].val[1] = 0;
				lines[i].val[2] = 0;
				lines[i].val[3] = 0;
			}
		}	
	}
	vector <Vec4i> lines2;
	for (int i = 0; i < lines.size(); i++)
	{
		if (lines[i].val[0] != 0 && lines[i].val[1] != 0 && lines[i].val[2] != 0 && lines[i].val[3] != 0)
		{
			lines2.push_back(lines[i]);
		}
	}
	
	for (int i = 0; i < lines2.size(); i++)
	{
		Vec4i l = lines2[i];
		line(img_complex2, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255, 0, 0), 3, LINE_AA);
	}
	if (debug >= 2) namedWindow("Track_Complex2", 0);
	if (debug >= 2) imshow("Track_Complex2", img_complex2); resizeWindow("Track_Complex2", resize_x, resize_y);


	return lines2;
}
int giveSteering(Mat &img_orginal, vector <Vec4i> lines)
{
	int yl1 = img_orginal.rows;			// y coordinates of lines to draw start and end
	int yl2 = yl1 /3;
	
	if (lines.size() <= 0) return -9999;
	typedef vector< std::vector<double> > matrix;

	matrix short_lines(lines.size(), vector<double>(2));
	short_lines = change_lines_notation(short_lines, lines);	// change notation from 2 points (x,y) to y=ax+b
	if (short_lines.size() <= 0)return -9999;

	matrix merged_lines;
	merged_lines = generate_blue_lines(short_lines);			//merge a lot of short lines with similar a into 1
	if (merged_lines.size() <= 0)return -9999;
	
	Mat img_complex;
	img_orginal.copyTo(img_complex);
	for (int i = 0; i < merged_lines.size(); i++)	//draw blue lines
	{
		int xl1 = ((yl1 - merged_lines[i][1]) / merged_lines[i][0]);
		int xl2 = ((yl2 - merged_lines[i][1]) / merged_lines[i][0]);
		line(img_complex, Point(xl1, yl1), Point(xl2, yl2), Scalar(255, 0, 0), 5);
	}
	if (debug >= 2) namedWindow("Track_Complex3", 0);
	if (debug >= 2) imshow("Track_Complex3", img_complex); resizeWindow("Track_Complex3", resize_x, resize_y);

	matrix blue_lines;
	blue_lines = delete_wrong_lines(merged_lines);		//delete lines if slope is too low
	if (blue_lines.size() <= 0)return -9999;

	Mat img_complex2;
	img_orginal.copyTo(img_complex2);
	for (int i = 0; i < blue_lines.size(); i++)	
	{
		int xl1 = ((yl1 - blue_lines[i][1]) / blue_lines[i][0]);
		int xl2 = ((yl2 - blue_lines[i][1]) / blue_lines[i][0]);
		line(img_complex2, Point(xl1, yl1), Point(xl2, yl2), Scalar(255, 0, 0), 5);
	}

	if (debug >= 2) namedWindow("Track_Complex4", 0);
	if (debug >= 2) imshow("Track_Complex4", img_complex2); resizeWindow("Track_Complex4", resize_x, resize_y);


	matrix main_minus_track_lines;
	matrix main_plus_track_lines;

	main_plus_track_lines = generate_lines_to_draw(blue_lines, 1);
	main_minus_track_lines = generate_lines_to_draw(blue_lines, 0);

	if (main_minus_track_lines.size() <= 0)return -9999;
	if (main_plus_track_lines.size() <= 0)return -9999;

	matrix main_track_lines(main_plus_track_lines.size() + main_minus_track_lines.size(), vector<double>(2));
	if (main_track_lines.size() <= 0)return -9999;

	int i = 0;
	for (i; i < main_plus_track_lines.size(); i++)
	{
		main_track_lines[i][0] = main_plus_track_lines[main_plus_track_lines.size() - i - 1][0];
		main_track_lines[i][1] = main_plus_track_lines[main_plus_track_lines.size() - i - 1][1];
	}
	for (int j = 0; j < main_minus_track_lines.size(); j++)
	{
		main_track_lines[i + j][0] = main_minus_track_lines[j][0];
		main_track_lines[i + j][1] = main_minus_track_lines[j][1];
	}

	for (int i = 0; i < main_track_lines.size(); i++)	//draw blue lines
	{
		int xl1 = ((yl1 - main_track_lines[i][1]) / main_track_lines[i][0]);
		int xl2 = ((yl2 - main_track_lines[i][1]) / main_track_lines[i][0]);
		line(img_orginal, Point(xl1, yl1), Point(xl2, yl2), Scalar(255, 0, 0), 5);
	}

	if (debug >= 2) namedWindow("Lines", 0);	
	if (debug >= 2) imshow("Lines", img_orginal); resizeWindow("Lines", resize_x, resize_y);
	line(img_orginal, Point((img_orginal.cols / 2), yl1), Point((img_orginal.cols / 2), 0), Scalar(0, 255, 255), 2);	//draw middle line

	int stering_x = 0;
	double max_a = 0;
	for (int i = 0; i < main_track_lines.size() - 1; i++)		//draw steering lines
	{
		//x1 = [ ((y1 - b1) / a1) + ((y1 - b2) / a2) ] / 2
		int xl1 = (((yl1 - main_track_lines[i][1]) / main_track_lines[i][0]) + ((yl1 - main_track_lines[i + 1][1]) / main_track_lines[i + 1][0])) / 2;
		int xl2 = (((yl2 - main_track_lines[i][1]) / main_track_lines[i][0]) + ((yl2 - main_track_lines[i + 1][1]) / main_track_lines[i + 1][0])) / 2;
		line(img_orginal, Point(xl1, yl1), Point(xl2, yl2), Scalar(0, 0, 255 - i * 50), 5);
		if (max_a < main_track_lines[i][0])
		{
			stering_x = xl1;
		}
	}
	
	int steering = -1 * (img_orginal.cols / 2 - stering_x);
	


	return steering;
}
int screenTest()
{
		Mat img_orginal=imread("real_good_quality_track_6.jpg");	//CHANGE FILE IF NEEDED
		if(debug>=1) namedWindow("Orginal", 0);						
		if (debug >= 1) imshow("Orginal", img_orginal); resizeWindow("Orginal", resize_x, resize_y);

		Mat img_egdges = prepareImage(img_orginal);	//prepare image to lines searching
		vector <Vec4i> lines = detectLines(img_egdges, img_orginal); //find straight lines on the image
		int steer = giveSteering(img_orginal, lines);	
		if (steer == -9999) steer = prevSteering; //if there was any error don't apply new steering
		prevSteering = steer;//save last steering for next iteration
		if (debug >= 1) cout << "Steeing: " << steer << endl;
		if (debug >= 1) namedWindow("Steering", 0);							//show orginal
		if (debug >= 1) imshow("Steering", img_orginal); resizeWindow("Steering", resize_x, resize_y);
		waitKey();

	return 0;
}
int videoTest()
{
	// early and middle stage video are up to date becasue of fixed camera positin 
	video.open("video_late_test_1.mp4");	//CHANG FILE IF NEEDED
	if (!video.isOpened())
	{
		cout << "Nie znaleznione video";
		return -1;
	}

	while (video.get(CAP_PROP_POS_FRAMES) < video.get(CAP_PROP_FRAME_COUNT) - 1)
	{
		Mat img_orginal;
		video.read(img_orginal);
		if (debug >= 1) namedWindow("Orginal", 0);
		if (debug >= 1) imshow("Orginal", img_orginal); resizeWindow("Orginal", resize_x, resize_y);

		Mat img_egdges = prepareImage(img_orginal);	//prepare image to lines searching
		vector <Vec4i> lines = detectLines(img_egdges, img_orginal); //find straight lines on the image
		int steer = giveSteering(img_orginal, lines);
		if (steer == -9999) steer = prevSteering; //if there was any error don't apply new steering
		prevSteering = steer;//save last steering for next iteration
		if (debug >= 1) cout << "Steeing: " << steer << endl;
		if (debug >= 1) namedWindow("Steering", 0);							//show orginal
		if (debug >= 1) imshow("Steering", img_orginal); resizeWindow("Steering", resize_x, resize_y);
		waitKey(10);
	}
		return 0;
	
}
Mat prepareImgForLineDetection(Mat img_orginal)
{
	int cannyAdjust[2] = { 200,150 };
	int tresholdOneAdjust[2] = { 150         ,255 };
	int tresholdTwoAdjust[2] = { 90,255 };
	int blurOneAdjust[2] = { 5,10 };
	int blurTwoAdjust[2] = { 3,10 };
	float perspectiveAdjust[2] = { 0.23, 0.6 }; //X multiplier, Y multiplier

	//PERSPECTIVE TRANSFORMATION
	int img_X = img_orginal.cols;
	int img_Y = img_orginal.rows;
	//source points used to transformation change to adjust percepctive view          
	//  upperLeft(1/5X , 5/9Y) -> upperRight(4/5X , 5/9Y) V
	//  bottomLeft(0 , Y)	   <- bottomRight(X , Y)     <- 
	Point2f sourcePoints[4] = { Point2f(img_X * perspectiveAdjust[0], img_Y * perspectiveAdjust[1]),
								Point2f(img_X * (1 - 1.22*perspectiveAdjust[0]), img_Y *  perspectiveAdjust[1]),
								Point2f(img_X,img_Y),
								Point2f(0,img_Y) };
	Point2f dstPoints[4] =	  { Point2f(0,0),
								Point(img_X,0), 
								Point2f(img_X,img_Y), 
								Point2f(0,img_Y) };
	Mat transformMatrix = getPerspectiveTransform(sourcePoints, dstPoints);
	Mat img_perspective;
	warpPerspective(img_orginal, img_perspective, transformMatrix, Size(img_X, img_Y));
	imshow("Perspective", img_perspective);

	//CONVERT TO GRAYSCALE
	Mat img_grayscale;
	cvtColor(img_perspective, img_grayscale, COLOR_BGR2GRAY);
	//imshow("Grayscale", img_grayscale);

	//BINARY TRESHOLD
	//blured and tresholded twice to get better effect
	threshold(img_grayscale, img_grayscale, tresholdOneAdjust[0], tresholdOneAdjust[1], THRESH_BINARY);
	blur(img_grayscale, img_grayscale, Size(blurOneAdjust[0], blurOneAdjust[1]));
	blur(img_grayscale, img_grayscale, Size(blurTwoAdjust[0], blurTwoAdjust[1]));
	threshold(img_grayscale, img_grayscale, tresholdTwoAdjust[0], tresholdTwoAdjust[1], THRESH_BINARY);
	imshow("Tresholded", img_grayscale);

	//DETECT EDGES
	Mat img_egdges;
	Canny(img_grayscale, img_egdges, cannyAdjust[0], cannyAdjust[1]);
	//imshow("Canny", img_egdges);

	img_perspective.release();
	img_grayscale.release();

	return img_egdges;
}
int ex11()
{
	video.open("no_lines.mp4");
	//video.open("lines.mp4");
	//video.open("middle_track.mp4");
	//video.open("track_test007.mp4");

	if (!video.isOpened())
	{
		cout << "Nie znaleznione video";
		return -1;
	}

	//while (video.get(CAP_PROP_POS_FRAMES) < video.get(CAP_PROP_FRAME_COUNT) - 1)
	//{
	

		Mat img_orginal = imread("curved_lines.png", IMREAD_UNCHANGED);
		//video.read(img_orginal);					
		imshow("Orginal", img_orginal);
	
		Mat img_prepared = prepareImgForLineDetection(img_orginal);
	

}

int main()
{
	//Chose what are You going to test single image or video
	videoTest();
	//screenTest();

	waitKey(0);

	return 1;
}