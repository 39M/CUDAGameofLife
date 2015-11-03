#include <stdio.h>
#include <time.h>

#define SHOW_CONSOLE
#include "include\graphics.h"

#include "CUDAConnector.h"

using namespace ege;

#define CELL_X 500
#define CELL_Y 500
#define CELL_SIZE 2
#define CELL_COLOR (((200)<<16) | ((200)<<8) | (200))


char cells[CELL_X + 2][CELL_Y + 2] = { 0 };
char cellsNext[CELL_X + 2][CELL_Y + 2] = { 0 };


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


void updateScene(bool status = true)
{
	cleardevice();
	if (status)
		updateStatus();

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


void generate(int probability = 25, bool update = true)
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


void CPUConsole(int iterateTime)
{
	clock_t begin, end;
	begin = clock();
	for (int iterator = 0; iterator < iterateTime; iterator++)
	{

	}
	end = clock();
	printf("Time used %fms.\n", double(end - begin) * 1000 / CLOCKS_PER_SEC);
}


void GPUConsole(int iterateTime)
{
	clock_t begin, end;
	begin = clock();
	for (int iterator = 0; iterator < iterateTime; iterator++)
	{

	}
	end = clock();
	printf("Time used %fms.\n", double(end - begin) * 1000 / CLOCKS_PER_SEC);
}


int main()
{
	GUI();

	system("pause");
	return 0;
}