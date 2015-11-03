#include <stdio.h>
#include <time.h>

#define SHOW_CONSOLE
#include "include\graphics.h"

#include "pre_define.h"
#include "cuda_connector.h"

using namespace ege;


char cells[CELL_X + 2][CELL_Y + 2] = { 0 };
char cellsNext[CELL_X + 2][CELL_Y + 2] = { 0 };

char cellsTemp[CELL_X + 2][CELL_Y + 2] = { 0 };

void updateStatus()
{
	char cellsCount;
	for (int i = 1; i <= CELL_X; i++)
	for (int j = 1; j <= CELL_Y; j++)
	{
		cellsCount = cells[i - 1][j - 1] + cells[i - 1][j] + cells[i - 1][j + 1] +
			cells[i][j - 1] + cells[i][j + 1] +
			cells[i + 1][j - 1] + cells[i + 1][j] + cells[i + 1][j + 1];
		if (cellsCount == 3)
			cellsNext[i][j] = 1;
		else if (cellsCount == 2)
			cellsNext[i][j] = cells[i][j];
		else
			cellsNext[i][j] = 0;
	}
	memcpy(cells, cellsNext, (CELL_X + 2) * (CELL_Y + 2));
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


void generate(bool update = true, int probability = 25)
{
	for (int i = 1; i <= CELL_X; i++)
	for (int j = 1; j <= CELL_Y; j++)
	if (rand() % 100 + 1 < probability)
		cells[i][j] = 1;

	if (update)
		updateScene();
}


void clear(bool update = true)
{
	memset(cells, 0, (CELL_X + 2) * (CELL_Y + 2));

	if (update)
		updateScene();
}


void mainloop()
{
	bool pause = false;
	int fps = 5;

	generate();

	for (; is_run(); delay_fps(fps))
	{
		// keyboard message
		while (kbhit())
		{
			int key = getch();
			putchar(key);

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
			default:
				break;
			}
		}

		// mouse message
		while (mousemsg())
		{
			putchar('m');
			mouse_msg mouse = getmouse();
			if (pause && mouse.is_up())
			{
				if (cells[mouse.x / CELL_SIZE + 1][mouse.y / CELL_SIZE + 1])
					cells[mouse.x / CELL_SIZE + 1][mouse.y / CELL_SIZE + 1] = 0;
				else
					cells[mouse.x / CELL_SIZE + 1][mouse.y / CELL_SIZE + 1] = 1;
				updateScene(false);
			}
		}

		if (pause)
			continue;

		updateScene();
	}
}


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


float CPUConsole(int iterateTime)
{
	clear(false);
	generate(false);
	memcpy(cellsTemp, cells, (CELL_X + 2) * (CELL_Y + 2));

	clock_t begin, end;
	float time;
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


float GPUConsole(int iterateTime)
{
	//clear(false);
	//generate(false);

	clock_t begin, end;
	float time;
	begin = clock();

	//CUDAUpdate(cellsTemp, iterateTime);
	anotherCUDAUpdate(cellsTemp, iterateTime);

	end = clock();
	time = double(end - begin) * 1000 / CLOCKS_PER_SEC;
	printf("GPU time used %fms.\n", time);

	return time;
}


bool checkResult()
{
	for (int i = 1; i <= CELL_X; i++)
	for (int j = 1; j <= CELL_Y; j++)
	if (cellsTemp[i][j] != cells[i][j])
		return false;
	return true;
}


void Console()
{
	printf("Conway's Game of Life\n");
	printf("%d x %d cells, ", CELL_X, CELL_Y);
	printf("iterate for %d times.\n\n", ITERATE_TIME);

	float cpu_time = CPUConsole(ITERATE_TIME);
	float gpu_time = GPUConsole(ITERATE_TIME);

	printf("\nChecking result...\n");
	if (checkResult())
		printf("Right Answer.\n");
	else
		printf("Wrong Answer.\n");

	printf("\nSpeed up: %fx\n", cpu_time / gpu_time);
}


int main()
{
	//GUI();
	Console();

	system("pause");
	return 0;
}