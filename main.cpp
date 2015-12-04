#include <stdio.h>
#include <time.h>

//#define SHOW_CONSOLE
#include "include\graphics.h"

#include "pre_define.h"
#include "cuda_connector.h"

using namespace ege;

// status of all cells
char cells[CELL_X + 2][CELL_Y + 2] = { 0 };
// next status of all cells
char cellsNext[CELL_X + 2][CELL_Y + 2] = { 0 };

// status of all cells backup used for CUDA
char cellsTemp[CELL_X + 2][CELL_Y + 2] = { 0 };


// Get the next status of all cells
void updateStatus()
{
	char cellsCount;
	for (int i = 1; i <= CELL_X; i++)
	for (int j = 1; j <= CELL_Y; j++)
	{
		cellsCount = cells[i - 1][j - 1] + cells[i - 1][j] + cells[i - 1][j + 1] +
			cells[i][j - 1] + cells[i][j + 1] +
			cells[i + 1][j - 1] + cells[i + 1][j] + cells[i + 1][j + 1];
		if (cellsCount == 3)			// 3 neighbours -> alive
			cellsNext[i][j] = 1;
		else if (cellsCount == 2)		// 2 neighbours -> remain
			cellsNext[i][j] = cells[i][j];
		else							// others -> die
			cellsNext[i][j] = 0;
	}
	memcpy(cells, cellsNext, (CELL_X + 2) * (CELL_Y + 2));		// Copy next status to current for update
}


void updateScene(bool status = true, bool use_gpu = false)
{
	cleardevice();
	if (status)
	{
		if (use_gpu)
			//CUDAUpdate(cells, 1);
			anotherCUDAUpdate(cells, 1);
		else
			updateStatus();
	}

	// Draw cells
	if (CELL_SIZE == 1)
	{
		for (int i = 1; i <= CELL_X; i++)
		for (int j = 1; j <= CELL_Y; j++)
		if (cells[i][j])
			putpixel(i - 1, j - 1, CELL_COLOR);
	}
	else
	{
		for (int i = 1; i <= CELL_X; i++)
		for (int j = 1; j <= CELL_Y; j++)
		if (cells[i][j])
			bar((i - 1) * CELL_SIZE, (j - 1) * CELL_SIZE, i * CELL_SIZE, j * CELL_SIZE);
	}
}


// Randomly generate cells
void generate(bool update = true, int probability = 25)
{
	for (int i = 1; i <= CELL_X; i++)
	for (int j = 1; j <= CELL_Y; j++)
	if (rand() % 100 + 1 < probability)
		cells[i][j] = 1;

	if (update)
		updateScene();
}


// Draw a gosper glider gun
void draw_gun(int x, int y, bool update = true)
{
	if (x < 1 || y - 4 < 1 || x + 35 > CELL_X || y + 4 > CELL_Y)
		return;

	cells[x][y] = 1;
	cells[x + 1][y] = 1;
	cells[x][y + 1] = 1;
	cells[x + 1][y + 1] = 1;

	cells[x + 10][y] = 1;
	cells[x + 10][y + 1] = 1;
	cells[x + 10][y + 2] = 1;
	cells[x + 11][y - 1] = 1;
	cells[x + 11][y + 3] = 1;
	cells[x + 12][y - 2] = 1;
	cells[x + 12][y + 4] = 1;
	cells[x + 13][y - 2] = 1;
	cells[x + 13][y + 4] = 1;
	cells[x + 14][y + 1] = 1;
	cells[x + 15][y - 1] = 1;
	cells[x + 15][y + 3] = 1;
	cells[x + 16][y] = 1;
	cells[x + 16][y + 1] = 1;
	cells[x + 16][y + 2] = 1;
	cells[x + 17][y + 1] = 1;

	cells[x + 20][y] = 1;
	cells[x + 20][y - 1] = 1;
	cells[x + 20][y - 2] = 1;
	cells[x + 21][y] = 1;
	cells[x + 21][y - 1] = 1;
	cells[x + 21][y - 2] = 1;
	cells[x + 22][y + 1] = 1;
	cells[x + 22][y - 3] = 1;

	cells[x + 24][y + 1] = 1;
	cells[x + 24][y + 2] = 1;
	cells[x + 24][y - 3] = 1;
	cells[x + 24][y - 4] = 1;

	cells[x + 34][y - 1] = 1;
	cells[x + 34][y - 2] = 1;
	cells[x + 35][y - 1] = 1;
	cells[x + 35][y - 2] = 1;

	if (update)
		updateScene(false);
}


// Make all cells die
void clear(bool update = true)
{
	memset(cells, 0, (CELL_X + 2) * (CELL_Y + 2));

	if (update)
		updateScene();
}


// Mainloop of GUI
void mainloop()
{
	bool pause = false;
	int fps = 5;

	generate();

	for (; is_run(); delay_fps(fps))
	{
		bool live = false;
		bool die = false;

		// keyboard message
		while (kbhit())
		{
			int key = getch();
			switch (key)
			{
			case 'p':	// pause
				pause = pause ? false : true;
				break;
			case 'n':	// next frame when pause
				if (pause)
					updateScene();
				break;
			case 'r':	// regenerate
				generate();
				break;
			case 'w':	// fps++
				if (fps < 120)
					fps++;
				break;
			case 's':	// fps--
				if (fps > 1)
					fps--;
				break;
			case ' ':	// clear
				clear();
				break;
			case 'z':
				live = true;
				break;
			case 'x':
				die = true;
				break;
			default:
				break;
			}
		}

		// mouse message
		while (mousemsg())
		{
			mouse_msg mouse = getmouse();

			if (mouse.is_move())
			{
				// Press z or x and move mouse to change the status of a cell
				if (live && !cells[mouse.x / CELL_SIZE + 1][mouse.y / CELL_SIZE + 1])
				{
					cells[mouse.x / CELL_SIZE + 1][mouse.y / CELL_SIZE + 1] = 1;
					updateScene(false);
				}
				else if (die && cells[mouse.x / CELL_SIZE + 1][mouse.y / CELL_SIZE + 1])
				{
					cells[mouse.x / CELL_SIZE + 1][mouse.y / CELL_SIZE + 1] = 0;
					updateScene(false);
				}
			}

			// Click mid button to draw a gosper glider gun
			if (mouse.is_up() && mouse.is_mid())
			{
				draw_gun(mouse.x / CELL_SIZE + 1, mouse.y / CELL_SIZE + 1);
			}
		}

		if (pause)
			continue;

		updateScene();
	}
}


// GUI version
void GUI()
{
	setinitmode(INIT_DEFAULT);
	initgraph(CELL_X * CELL_SIZE, CELL_Y * CELL_SIZE);
	setcaption("Conway's Game of Life");
	setrendermode(RENDER_MANUAL);
	setfillcolor(CELL_COLOR);

	mainloop();

	closegraph();
}


// Console version with CPU, return execute time
double CPUConsole(int iterateTime)
{
	clear(false);
	generate(false);
	memcpy(cellsTemp, cells, (CELL_X + 2) * (CELL_Y + 2));

	clock_t begin, end;
	double time;
	begin = clock();
	for (int iterator = 0; iterator < iterateTime; iterator++)
	{
		updateStatus();
	}
	end = clock();
	time = double(end - begin) * 1000 / CLOCKS_PER_SEC;
	printf("CPU time used %fms.\n", time);

	return time;
}


// Console version with CUDA, return execute time
double GPUConsole(int iterateTime)
{
	//clear(false);
	//generate(false);

	clock_t begin, end;
	double time;
	begin = clock();

	//CUDAUpdate(cellsTemp, iterateTime);
	anotherCUDAUpdate(cellsTemp, iterateTime);

	end = clock();
	time = double(end - begin) * 1000 / CLOCKS_PER_SEC;
	printf("GPU time used %fms.\n", time);

	return time;
}


// Check if the results of CPU and CUDA are same
bool checkResult()
{
	for (int i = 1; i <= CELL_X; i++)
	for (int j = 1; j <= CELL_Y; j++)
	if (cellsTemp[i][j] != cells[i][j])
		return false;
	return true;
}


// Console version, run CPU and CUDA version, compare execute time
void Console()
{
	printf("Conway's Game of Life\n");
	printf("%d x %d cells, ", CELL_X, CELL_Y);
	printf("iterate for %d times.\n\n", ITERATE_TIME);

	double cpu_time = CPUConsole(ITERATE_TIME);
	double gpu_time = GPUConsole(ITERATE_TIME);

	printf("\nChecking result...\n");
	if (checkResult())
		printf("Right Answer.\n");
	else
		printf("Wrong Answer.\n");

	printf("\nSpeed up: %fx\n", cpu_time / gpu_time);
	system("pause");
}


int main()
{
	srand((unsigned)time(NULL));
	// if run GUI(), the Console() won't execute
	GUI();
	//Console();

	return 0;
}