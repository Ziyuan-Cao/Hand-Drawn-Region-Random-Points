#include <stdio.h>
#include <math.h>
#include <cmath>
#include <GL/glut.H>
#include<algorithm>
#include <iostream>
#include <vector>
#include <stack>
#include <ctime>
using namespace std;

///////////************************************//////////////
/**Notice
*	1. Region can only be drawn sequentially and overlaid.
*   2. Unable to delete existing region.
*	3. Only supports drawing solid regions.
*   4. Only use a simple random function to generate nutrient points.
*   5. When drawing, the end point needs to be very close to the start 
*      point to complete the area drawing.
*   6. ...
**/
///////////************************************//////////////

int MaxGrowPoint = 50;

int PixelSize = 3;
int Width = 1000;
int Height = 1000;
int width = 0;
int height = 0;

//The data of the point contains the markers of the xy coordinates and the region number
struct point
{
	int x = 0;
	int y = 0;
	int number = 0;

	point(int ix, int iy, int inumber) : x(ix), y(iy), number(inumber) {}

};

vector<vector<int>> Figure; //the final result map
vector<vector<point>> Region; //the edge of the region
vector<vector<point>> GrowPointGroup;//Nutrient points in each region

//process variable
int number = 0;
bool Isdrawing = false;
vector<point> Region_Buffer;


//Use random functions to generate nutrient points in a region
void GrowPointInsert(int Ix_min, int Ix_max, int Iy_min, int Iy_max,int Inumber)
{
	int count = 0;
	int maxiter = MaxGrowPoint*100;
	vector<point> growpoints;
	
	srand(time(0));

	while (count < MaxGrowPoint && maxiter > 0)
	{
		//Use bounding boxes to reduce the range of random point generation
		int Rnd1 = rand();
		int Rnd2 = rand();
		int Rx = Rnd1 % (Ix_max - Ix_min) + Ix_min;
		int Ry = Rnd2 % (Iy_max - Iy_min) + Iy_min;
		//Check if the generated point is inside this region
		if (Figure[Ry][Rx] == Inumber)
		{
			growpoints.push_back(point(Rx, Ry,Inumber));
			count++;
		}
		maxiter--;
	}
	GrowPointGroup.push_back(growpoints);
}

//Fill the entire region with depth traversal
void DeepFill(int Ix, int Iy,int Inumber)
{
	stack<point> fillstack;

	fillstack.push(point(Ix, Iy, Inumber));

	while (!fillstack.empty())
	{
		int y = fillstack.top().y;
		int x = fillstack.top().x;
		fillstack.pop();
		Figure[y][x] = Inumber;
		if (y + 1 < Height && Figure[y + 1][x] != Inumber)
		{
			fillstack.push(point(x, y + 1, Inumber));
		}
		if (y-1 >= 0 && Figure[y-1][x] != Inumber)
		{
			fillstack.push(point(x, y - 1, Inumber));
		}
		if (x + 1 < Width && Figure[y][x+1] != Inumber)
		{
			fillstack.push(point(x + 1, y, Inumber));
		}
		if (x-1 >= 0 && Figure[y][x-1] != Inumber)
		{
			fillstack.push(point(x - 1, y, Inumber));
		}
	}

}

//Refresh region buffer
void FigureRefresh()
{
	//clear the buffer
	for (int i = 0; i < Figure.size(); i++)
	{
		for (int j = 0; j < Figure[i].size(); j++)
		{
			Figure[i][j] = -1;
		}
	}

	//re-paint each Region
	for (int i = 0; i < Region.size(); i++)
	{
		//Build a bounding box to find the center point 
		//to fill the entire area
		int x_max = 0;
		int x_min = Width;
		int y_max = 0;
		int y_min = Height;

		for (int j = 0; j < Region[i].size(); j++)
		{

			int x = Region[i][j].x;
			int y = Region[i][j].y;

			x_max = max(x_max, x);
			x_min = min(x_min, x);
			y_max = max(y_max, y);
			y_min = min(y_min, y);
			//To keep the loop closed at pixel accuracy,
			//Need to enclose edge points with PixelSize
			for (int k = 0; k < PixelSize; k++)
			{
				for (int l = 0; l < PixelSize; l++)
				{
					Figure[y+k][x+l] = Region[i][j].number;
				}
			}
		}
		//Fill the entire region with depth traversal
		//begin with the center point 
		DeepFill((x_max+x_min)/2, (y_max + y_min) / 2, Region[i][0].number);
		//Randomly generate nutrient points in the area
		//only one times
		if (number - 1 == Region[i][0].number)
		{
			GrowPointInsert(x_min, x_max, y_min, y_max, Region[i][0].number);
		}
	}

	
}

//Check if two points are close, 
//the range is PixelSize
bool IsClosePoint(point IP1,point IP2)
{
	bool pointcmp = true;

	pointcmp = (IP1.x - PixelSize <= IP2.x) && (IP1.x + PixelSize >= IP2.x);
	pointcmp &= (IP1.y - PixelSize <= IP2.y) && (IP1.y + PixelSize >= IP2.y);
	return pointcmp;
}

//Check if the last point of route is closed with the first point
bool IsReigonComplete(vector<point>& Iregionbuffer)
{
	return IsClosePoint(Iregionbuffer.front(), Iregionbuffer.back());
}


void display(void)
{

	float w = 2.0 / (Height - 1);
	float h = 2.0 / (Width - 1);
	
	
	if (Isdrawing)
	{
		//the process of drawing the mouse path with red points
		glPointSize(PixelSize);
		glColor3f(1.0, 0, 0);
		glBegin(GL_POINTS);
		for (int i = 0; i < Region_Buffer.size(); i++)
		{
			glVertex3f(Region_Buffer[i].x * w - 1, (Height - Region_Buffer[i].y) * h - 1, 0);
		}
		glEnd();
	}
	else
	{
		//Paint corel regions
		glClear(GL_COLOR_BUFFER_BIT);
		glLoadIdentity();


		glPointSize(PixelSize);
		glBegin(GL_POINTS);
		for (int i = 0; i < Height; i++)
		{
			for (int j = 0; j < Width; j++)
			{
				if (Figure[i][j] >-1)
				{
					glColor3f((0.03 * (Figure[i][j] + 1)), (0.06 * (Figure[i][j] + 1)), (0.09 * (Figure[i][j] + 1)));
					glVertex3f(j * w - 1, (Height - i) * h - 1, 0);
				}
			}
		}
		glEnd();

		//Plot nutrient points for each region
		glPointSize(PixelSize*3);
		glBegin(GL_POINTS);
		for (int i = 0; i < GrowPointGroup.size(); i++)
		{
			for (int j = 0; j < GrowPointGroup[i].size(); j++)
			{
				int x = GrowPointGroup[i][j].x;
				int y = GrowPointGroup[i][j].y;
				if (GrowPointGroup[i][j].number == Figure[y][x])
				{
					glColor3f(0, 1.0 - (0.25 * ((Figure[i][j] + 1) % 5)), 0.5);
					glVertex3f(x * w - 1, (Height - y) * h - 1, 0);
				}
			}
		}
		glEnd();


		////Debug
		//glPointSize(PixelSize);
		//glBegin(GL_POINTS);
		//for (int i = 0; i < Region.size(); i++)
		//{
		//	for (int j = 0; j < Region[i].size(); j++)
		//	{
		//		glColor3f(0.0, 1.0, 0.0);
		//		glVertex3f(Region[i][j].x * w - 1, (Height - Region[i][j].y) * h - 1, 0);
		//	}
		//}
		//glEnd();
	}

	glFlush(); 
}

//Pixel-based two-point line interpolation function
void connect(point IP1, point IP2, vector<point>& Iregionbuffer)
{
	//printf("(%d,%d) --- (%d,%d) , %d\n", IP1.x, IP1.y, IP2.x, IP2.y, number);
	if (IP1.x == IP2.x && IP1.y == IP2.y)
	{
		Iregionbuffer.push_back(IP1);
	}
	if(IP1.x == IP2.x)
	{
		if(IP1.y < IP2.y)
		for (int y = IP1.y; y < IP2.y; y++)
		{
			Iregionbuffer.push_back(point(IP1.x,y,number));
		}
		if (IP2.y < IP1.y)
		for (int y = IP2.y; y < IP1.y; y++)
		{
			Iregionbuffer.push_back(point(IP1.x, y, number));
		}
		return;
	}
	if (IP1.y == IP2.y)
	{
		if (IP1.x < IP2.x)
		for (int x = IP1.x; x < IP2.x; x++)
		{
			Iregionbuffer.push_back(point(x, IP1.y, number));
		}
		if (IP2.x < IP1.x)
		for (int x = IP2.x; x < IP1.x; x++)
		{
			Iregionbuffer.push_back(point(x, IP1.y, number));
		}
		return;
	}
	
	if (fabs(IP2.x - IP1.x) < fabs(IP2.y - IP1.y))
	{
		if (IP1.y > IP2.y)
		{
			for (int y = IP2.y; y < IP1.y; y++)
			{
				int x = ((float)y - IP1.y) / (IP2.y - IP1.y) * (IP2.x - IP1.x) + IP1.x;
				point linepoint(x, y, number);
				Iregionbuffer.push_back(linepoint);
			}
		}
		else
		{
			for (int y = IP2.y; y >= IP1.y; y--)
			{
				int x = ((float)y - IP1.y) / (IP2.y - IP1.y) * (IP2.x - IP1.x) + IP1.x;
				point linepoint(x, y, number);
				Iregionbuffer.push_back(linepoint);
			}
		}
	}
	else
	{
		if (IP1.x > IP2.x)
		{
			for (int x = IP2.x; x < IP1.x; x++)
			{
				int y = ((float)x - IP1.x) / (IP2.x - IP1.x) * (IP2.y - IP1.y) + IP1.y;
				point linepoint(x, y, number);
				Iregionbuffer.push_back(linepoint);
			}
		}
		else
		{
			for (int x = IP2.x; x >= IP1.x; x--)
			{
				int y = ((float)x - IP1.x) / (IP2.x - IP1.x) * (IP2.y - IP1.y) + IP1.y;
				point linepoint(x, y, number);
				Iregionbuffer.push_back(linepoint);
			}
		}
	}

}

void RegionDraw(int Ix, int Iy)
{

	Region_Buffer.push_back(point(Ix, Iy, number));

	display();
}



void mouse(int btn, int state, int mx, int my) 
{
	//Record the mouse track while holding down the left mouse button
	if (state == GLUT_DOWN)
	{

			Isdrawing = true;
			glutMotionFunc(RegionDraw);
	}
	else if(state == GLUT_UP)
	{
		//Detect whether the track is closed loop when the left button is released
		// Ture for new region of coral
		// False for nothing
		Isdrawing = false;
		if (IsReigonComplete(Region_Buffer))
		{
			//Due to the intermittent call of the callback function, 
			//the record points of the mouse path are not continuous, 
			//and the connection function needs to be used to connect 
			//the path into a loop.
			vector<point> Newregion;
			for(int i = 0; i < Region_Buffer.size()-1;i++)
			{
				connect(Region_Buffer[i], Region_Buffer[i+1], Newregion);
			}
			connect(Region_Buffer[0], Region_Buffer.back(), Newregion);

			//Useless
			//sort(
			//	Newregion.begin(),
			//	Newregion.begin()+ Newregion.size(),
			//	[](point Ipoint1, point Ipoint2)-> bool 
			//	{
			//		if (Ipoint1.y > Ipoint2.y)
			//		{
			//			return true;
			//		}
			//		else if (Ipoint1.y == Ipoint2.y)
			//		{
			//			return Ipoint1.x < Ipoint2.x;
			//		}
			//		else
			//		{
			//			return false;
			//		}
			//	});

			Region.push_back(Newregion);
			number++;

			FigureRefresh();
			Region_Buffer.clear();
		}
		else
		{
			Region_Buffer.clear();
		}
	}

	display();
	return;
}


void keyboard(unsigned char key, int mx, int my) 
{

	display();
}

void myReshape(int w, int h)
{
	width = w;
	height = h;

	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(-1, 1, -1, 1, -1, 1);

	glMatrixMode(GL_MODELVIEW);
}

void myinit()
{
	Figure.assign(Width, vector<int>(Height, -1));

	glClearColor(1, 1, 1, 1);
}

int main(int argc, char** argv)
{
  glutInit(&argc, argv); 
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  glutInitWindowSize(Width, Height);
  glutCreateWindow("SAN GO");
  
  myinit();

  glutReshapeFunc(myReshape);

  glutMouseFunc(mouse); 
  
  glutKeyboardFunc(keyboard);

  glutDisplayFunc(display); 
  glutMainLoop();
  
  return 0;
}

