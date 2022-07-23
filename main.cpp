#include <iostream>
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <cmath>
#include <GL/glut.H>
#include<algorithm>
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
*   6. border of Window cannot be drawn.
*   7. ...
**/
///////////************************************//////////////

int MaxGrowPoint = 50;
int OutlineThick = 3;
int borderlandGrowPoint = 200;

int PixelSize = 3;
int Width = 1000;
int Height = 1000;
int Length = 1000;
int width = 0;
int height = 0;

//The data of the point contains the markers of the xy coordinates and the region ID
struct point
{
	int X = 0;
	int Y = 0;
	int Z = 0;
	int ID = 0;

	point(int ix, int iy, int iz, int iid) : X(ix), Y(iy), Z(iz), ID(iid) {}


	void Normalize(float& ox, float& oy, float& oz)
	{
		float w = 1.0 / (Height - 1);
		float h = 1.0 / (Width - 1);
		float l = 1.0 / (Length - 1);

		ox = X * w;
		oy = (Height - Y) * h;
		oz = Z * l;
	}
};

struct color
{
	float R = 0;
	float G = 0;
	float B = 0;
};

struct Region
{
	int X_max = 0;
	int X_min = Width;
	int Y_max = 0;
	int Y_min = Height;
	int Center_X = 0;
	int Center_Y = 0;
	int ID = 0;
	int MaxGrowPointNumber = MaxGrowPoint;

	vector<point> EdgePoints; //the edge of the region
	vector<point> GrowPoints; //Nutrient points in each region

	color RegionColor;
	color GrowPointColor;
};

struct borderLand : Region
{
	int Thick = 0;
	void Initialize(int IID)
	{
		MaxGrowPointNumber = borderlandGrowPoint;
		Thick = OutlineThick;
		X_max = Width - 1;
		X_min = 1;
		Y_max = Height - 1;
		Y_min = 1;
		Center_X = 0;
		Center_Y = 0;
		ID = IID;

		EdgePoints.clear();
		GrowPoints.clear();

		RegionColor.B = 0;
		RegionColor.G = 1;
		RegionColor.R = 1;

		GrowPointColor.B = 1;
		GrowPointColor.G = 0;
		GrowPointColor.R = 0;
	}

};

struct Creator
{
	Region CoralRegion;
	borderLand Borderland;
};



vector<vector<int>> Figure; //the final result map
vector<Creator> CoralGroup;


//process variable

int currentid = 0;
bool Isdrawing = false;
vector<point> Region_Buffer;

void ComfirmRegionGrowPointNumber()
{
	cout << "Max Grow Points number in this region: ";
	cin >> MaxGrowPoint;

	cout << "thick of outline: ";
	cin >> OutlineThick;

	cout << "Outline's Grow Points number: ";
	cin >> borderlandGrowPoint;
}

//Use random functions to generate nutrient points in a region
void GrowPointInsert(Region& IORegion)
{
	IORegion.GrowPoints.clear();

	int count = 0;
	int maxiter = IORegion.MaxGrowPointNumber * 100;

	srand(time(0));

	while (count < IORegion.MaxGrowPointNumber && maxiter > 0)
	{
		//Use bounding boxes to reduce the range of random point generation
		int Rnd1 = rand();
		int Rnd2 = rand();
		int Rx = Rnd1 % (IORegion.X_max - IORegion.X_min) + IORegion.X_min;
		int Ry = Rnd2 % (IORegion.Y_max - IORegion.Y_min) + IORegion.Y_min;
		//Check if the generated point is inside this region
		if (Figure[Ry][Rx] == IORegion.ID)
		{
			IORegion.GrowPoints.push_back(point(Rx, Ry, 0, IORegion.ID));
			count++;
		}
		maxiter--;
	}
}

//Fill the entire region with depth traversal Depth-First-Search
void DeepFill(int Ix, int Iy, int Iid)
{
	stack<point> fillstack;

	fillstack.push(point(Ix, Iy, 0, Iid));

	while (!fillstack.empty())
	{
		int y = fillstack.top().Y;
		int x = fillstack.top().X;
		fillstack.pop();
		Figure[y][x] = Iid;
		if (y + 1 < Height && Figure[y + 1][x] != Iid)
		{
			fillstack.push(point(x, y + 1, 0, Iid));
		}
		if (y - 1 >= 0 && Figure[y - 1][x] != Iid)
		{
			fillstack.push(point(x, y - 1, 0, Iid));
		}
		if (x + 1 < Width && Figure[y][x + 1] != Iid)
		{
			fillstack.push(point(x + 1, y, 0, Iid));
		}
		if (x - 1 >= 0 && Figure[y][x - 1] != Iid)
		{
			fillstack.push(point(x - 1, y, 0, Iid));
		}
	}

}

//Pixel-based two-point line interpolation function
void connect(point IP1, point IP2, vector<point>& Iregionbuffer)
{
	//printf("(%d,%d) --- (%d,%d) , %d\n", IP1.x, IP1.y, IP2.x, IP2.y, currentid);
	if (IP1.X == IP2.X && IP1.Y == IP2.Y)
	{
		Iregionbuffer.push_back(IP1);
	}
	if (IP1.X == IP2.X)
	{
		if (IP1.Y < IP2.Y)
			for (int y = IP1.Y; y < IP2.Y; y++)
			{
				Iregionbuffer.push_back(point(IP1.X, y, 0, currentid));
			}
		if (IP2.Y < IP1.Y)
			for (int y = IP2.Y; y < IP1.Y; y++)
			{
				Iregionbuffer.push_back(point(IP1.X, y, 0, currentid));
			}
		return;
	}
	if (IP1.Y == IP2.Y)
	{
		if (IP1.X < IP2.X)
			for (int x = IP1.X; x < IP2.X; x++)
			{
				Iregionbuffer.push_back(point(x, IP1.Y, 0, currentid));
			}
		if (IP2.X < IP1.X)
			for (int x = IP2.X; x < IP1.X; x++)
			{
				Iregionbuffer.push_back(point(x, IP1.Y, 0, currentid));
			}
		return;
	}

	if (fabs(IP2.X - IP1.X) < fabs(IP2.Y - IP1.Y))
	{
		if (IP1.Y > IP2.Y)
		{
			for (int y = IP2.Y; y < IP1.Y; y++)
			{
				int x = ((float)y - IP1.Y) / (IP2.Y - IP1.Y) * (IP2.X - IP1.X) + IP1.X;
				point linepoint(x, y, 0, currentid);
				Iregionbuffer.push_back(linepoint);
			}
		}
		else
		{
			for (int y = IP2.Y; y >= IP1.Y; y--)
			{
				int x = ((float)y - IP1.Y) / (IP2.Y - IP1.Y) * (IP2.X - IP1.X) + IP1.X;
				point linepoint(x, y, 0, currentid);
				Iregionbuffer.push_back(linepoint);
			}
		}
	}
	else
	{
		if (IP1.X > IP2.X)
		{
			for (int x = IP2.X; x < IP1.X; x++)
			{
				int y = ((float)x - IP1.X) / (IP2.X - IP1.X) * (IP2.Y - IP1.Y) + IP1.Y;
				point linepoint(x, y, 0, currentid);
				Iregionbuffer.push_back(linepoint);
			}
		}
		else
		{
			for (int x = IP2.X; x >= IP1.X; x--)
			{
				int y = ((float)x - IP1.X) / (IP2.X - IP1.X) * (IP2.Y - IP1.Y) + IP1.Y;
				point linepoint(x, y, 0, currentid);
				Iregionbuffer.push_back(linepoint);
			}
		}
	}

}


void CreateRegion(vector<point>& IRegion_Buffer)
{
	//Due to the intermittent call of the callback function, 
	//the record points of the mouse path are not continuous, 
	//and the connection function needs to be used to connect 
	//the path into a loop.
	Region Newregion;
	Creator NewCoral;
	Newregion.MaxGrowPointNumber = MaxGrowPoint;
	Newregion.ID = currentid;
	for (int i = 0; i < IRegion_Buffer.size() - 1; i++)
	{
		connect(IRegion_Buffer[i], IRegion_Buffer[i + 1], Newregion.EdgePoints);
	}
	connect(IRegion_Buffer[0], IRegion_Buffer.back(), Newregion.EdgePoints);

	//Build a bounding box to find the center point 
	int x_max = 0;
	int x_min = Width;
	int y_max = 0;
	int y_min = Height;
	for (int i = 0; i < Newregion.EdgePoints.size(); i++)
	{
		int x = Newregion.EdgePoints[i].X;
		int y = Newregion.EdgePoints[i].Y;

		x_max = max(x_max, x);
		x_min = min(x_min, x);
		y_max = max(y_max, y);
		y_min = min(y_min, y);
	}
	Newregion.X_max = x_max;
	Newregion.X_min = x_min;
	Newregion.Y_max = y_max;
	Newregion.Y_min = y_min;
	Newregion.Center_X = (x_max + x_min) / 2;
	Newregion.Center_Y = (y_max + y_min) / 2;

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

	//color

	srand(time(0));
	Newregion.RegionColor.R = (float)(rand() % 30) / 30;
	Newregion.RegionColor.G = (float)(rand() % 30) / 30;
	Newregion.RegionColor.B = (float)(rand() % 30) / 30;
	Newregion.GrowPointColor.R = 1.0 - Newregion.RegionColor.R;
	Newregion.GrowPointColor.G = 1.0 - Newregion.RegionColor.G;
	Newregion.GrowPointColor.B = 1.0 - Newregion.RegionColor.B;

	NewCoral.CoralRegion = move(Newregion);
	NewCoral.Borderland.Initialize(currentid+1);
	CoralGroup.push_back(NewCoral);

}

void Createborderland(Creator& ICreator)
{
	Region& IRegion = ICreator.CoralRegion;
	for (int j = 0; j < IRegion.EdgePoints.size(); j++)
	{

		int x = IRegion.EdgePoints[j].X;
		int y = IRegion.EdgePoints[j].Y;


		//To keep the loop closed at pixel accuracy,
		//Need to enclose edge points with PixelSize
		for (int k = 0; k < ICreator.Borderland.Thick; k++)
		{
			for (int l = 0; l < ICreator.Borderland.Thick; l++)
			{
				Figure[y + k][x + l] = ICreator.Borderland.ID;

			}
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
	for (int i = 0; i < CoralGroup.size(); i++)
	{
		Region& IRegion = CoralGroup[i].CoralRegion;

		for (int j = 0; j < IRegion.EdgePoints.size(); j++)
		{

			int x = IRegion.EdgePoints[j].X;
			int y = IRegion.EdgePoints[j].Y;


			//To keep the loop closed at pixel accuracy,
			//Need to enclose edge points with PixelSize
			for (int k = 0; k < PixelSize; k++)
			{
				for (int l = 0; l < PixelSize; l++)
				{
					Figure[y + k][x + l] = IRegion.ID;

				}
			}
		}


		//Fill the entire region with depth traversal
		//begin with the center point 
		DeepFill(IRegion.Center_X, IRegion.Center_Y, IRegion.ID);



		Createborderland(CoralGroup[i]);



	}

	//Refresh grow points to avoid mixing
	for (int i = 0; i < CoralGroup.size(); i++)
	{
		Region& IRegion = CoralGroup[i].CoralRegion;
		vector<point> NewGrowPoints;
		for (int j = 0; j < IRegion.GrowPoints.size(); j++)
		{
			int x = IRegion.GrowPoints[j].X;
			int y = IRegion.GrowPoints[j].Y;
			if (IRegion.ID == Figure[y][x])
			{
				NewGrowPoints.push_back(IRegion.GrowPoints[j]);
			}

		}
		IRegion.GrowPoints.clear();
		IRegion.GrowPoints = NewGrowPoints;
	}

}

//Check if two points are close, 
//the range is PixelSize
bool IsClosePoint(point IP1, point IP2)
{
	bool pointcmp = true;

	pointcmp = (IP1.X - PixelSize <= IP2.X) && (IP1.X + PixelSize >= IP2.X);
	pointcmp &= (IP1.Y - PixelSize <= IP2.Y) && (IP1.Y + PixelSize >= IP2.Y);
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
			float x = 0;
			float y = 0;
			float z = 0;
			Region_Buffer[i].Normalize(x, y, z);
			glVertex3f(x * 2 - 1, y * 2 - 1, z * 2 - 1);
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
				if (Figure[i][j] > -1)
				{
					if (Figure[i][j] % 2 == 0)
					{
						Region& IRegion = CoralGroup[Figure[i][j]/2].CoralRegion;
						float r = IRegion.RegionColor.R;
						float g = IRegion.RegionColor.G;
						float b = IRegion.RegionColor.B;

						point ijpoint(j, i, 0, -1);
						float x = 0;
						float y = 0;
						float z = 0;
						ijpoint.Normalize(x, y, z);
						glColor3f(r, g, b);
						glVertex3f(x * 2 - 1, y * 2 - 1, z * 2 - 1);
					}
					else
					{
						Region& IRegion = CoralGroup[(Figure[i][j]-1)/2].Borderland;
						float r = IRegion.RegionColor.R;
						float g = IRegion.RegionColor.G;
						float b = IRegion.RegionColor.B;

						point ijpoint(j, i, 0, -1);
						float x = 0;
						float y = 0;
						float z = 0;
						ijpoint.Normalize(x, y, z);
						glColor3f(r, g, b);
						glVertex3f(x * 2 - 1, y * 2 - 1, z * 2 - 1);
					}
				}
			}
		}
		glEnd();

		//Plot nutrient points for each region
		glPointSize(PixelSize * 3);
		glBegin(GL_POINTS);
		for (int i = 0; i < CoralGroup.size(); i++)
		{
			Region& IRegion = CoralGroup[i].CoralRegion;
			for (int j = 0; j < IRegion.GrowPoints.size(); j++)
			{

				int x = IRegion.GrowPoints[j].X;
				int y = IRegion.GrowPoints[j].Y;
				if (IRegion.ID == Figure[y][x])
				{
					float r = IRegion.GrowPointColor.R;
					float g = IRegion.GrowPointColor.G;
					float b = IRegion.GrowPointColor.B;

					float x = 0;
					float y = 0;
					float z = 0;
					IRegion.GrowPoints[j].Normalize(x, y, z);

					glColor3f(r, g, b);
					glVertex3f(x * 2 - 1, y * 2 - 1, z * 2 - 1);
				}
			}

			borderLand IBorderLand = CoralGroup[i].Borderland;
			for (int j = 0; j < IBorderLand.GrowPoints.size(); j++)
			{

				int x = IBorderLand.GrowPoints[j].X;
				int y = IBorderLand.GrowPoints[j].Y;
				if (IBorderLand.ID == Figure[y][x])
				{
					float r = IBorderLand.GrowPointColor.R;
					float g = IBorderLand.GrowPointColor.G;
					float b = IBorderLand.GrowPointColor.B;

					float x = 0;
					float y = 0;
					float z = 0;
					IBorderLand.GrowPoints[j].Normalize(x, y, z);

					glColor3f(r, g, b);
					glVertex3f(x * 2 - 1, y * 2 - 1, z * 2 - 1);
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


void RegionDraw(int Ix, int Iy)
{

	Region_Buffer.push_back(point(Ix, Iy, 0, currentid));

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
	else if (state == GLUT_UP)
	{
		//Detect whether the track is closed loop when the left button is released
		// Ture for new region of coral
		// False for nothing
		Isdrawing = false;
		if (IsReigonComplete(Region_Buffer))
		{
			ComfirmRegionGrowPointNumber();

			CreateRegion(Region_Buffer);
			currentid += 2;

			FigureRefresh();

			//Randomly generate nutrient points in the area
			//only one times
			GrowPointInsert(CoralGroup.back().CoralRegion);

			GrowPointInsert(CoralGroup.back().Borderland);

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

void CSVOutput(vector<Creator>& ICoralGroup)
{

	string csvPath = "GrowPoint.csv";
	ofstream csvfile;
	

	struct Brodpoint
	{
		int ID = -1;
		float x = 0;
		float y = 0;
		float z = 0;

	};
	vector<Brodpoint> BGP;
	for (int i = 0; i < ICoralGroup.size(); i++)
	{
		auto& GP = ICoralGroup[i].Borderland.GrowPoints;
		for (int j = 0; j < GP.size(); j++)
		{
			Brodpoint gp;
			gp.ID = 0;
			GP[j].Normalize(gp.x, gp.y, gp.z);
			BGP.push_back(gp);
		}
	}

	csvfile.open(csvPath, ios::out);

	for(int i = 0; i < BGP.size();i++)
	{
		csvfile << BGP[i].ID << ",";//ID
		csvfile << BGP[i].x << ",";//X
		csvfile << BGP[i].y << ",";//Y
		csvfile << BGP[i].z;//Z
		csvfile << "\n";
	}

	for (int i = 0; i < ICoralGroup.size(); i++)
	{
		Region& IRegion = ICoralGroup[i].CoralRegion;
		for (int j = 0; j < IRegion.GrowPoints.size(); j++)
		{

			float x = 0;
			float y = 0;
			float z = 0;
			IRegion.GrowPoints[j].Normalize(x, y, z);

			csvfile << i+1 << ",";//ID
			csvfile << x << ",";//X
			csvfile << y << ",";//Y
			csvfile << z;//Z
			csvfile << "\n";
		}
	}
	csvfile.close();

	return;
}

void keyboard(unsigned char key, int mx, int my)
{
	switch (key)
	{
	case 'c':
		CSVOutput(CoralGroup);
	}
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
	//CoralGroup.push_back(borderLand());
	//borderland = (borderLand*)&CoralGroup[0];
	//borderland->Initialize();


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

