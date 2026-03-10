#include "MazeGenerator.h"
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>
#include <stack>

struct Cell {
    int x, y;
};

void GenerateMaze(int maze[MH][MW], int& startX, int& startY, int width, int height) {
    // 1. Initialize maze with walls
    for (int y = 0; y < MH; y++) {
        for (int x = 0; x < MW; x++) {
            maze[y][x] = 1;
        }
    }

    std::random_device rd;
    std::mt19937 rng(rd());
    
    // Ensure width and height are within reasonable bounds and odd (for the carving algorithm)
    if (width > MW) width = MW;
    if (height > MH) height = MH;
    
    // 2. Start at (1, 1)
    startX = 1;
    startY = 1;
    maze[startY][startX] = 0;

    std::stack<Cell> s;
    s.push({startX, startY});

    int dx[] = { 0, 0, 2, -2 };
    int dy[] = { 2, -2, 0, 0 };

    while (!s.empty()) {
        Cell current = s.top();
        
        std::vector<int> neighbors;
        for (int i = 0; i < 4; i++) {
            int nx = current.x + dx[i];
            int ny = current.y + dy[i];

            // Ensure we stay within bounds of the requested size and only visit unvisited cells
            if (nx >= 1 && nx < width - 1 && ny >= 1 && ny < height - 1 && maze[ny][nx] == 1) {
                neighbors.push_back(i);
            }
        }

        if (!neighbors.empty()) {
            std::uniform_int_distribution<int> dist(0, (int)neighbors.size() - 1);
            int dirIndex = neighbors[dist(rng)];

            int nx = current.x + dx[dirIndex];
            int ny = current.y + dy[dirIndex];
            int mx = current.x + dx[dirIndex] / 2;
            int my = current.y + dy[dirIndex] / 2;

            maze[my][mx] = 0;
            maze[ny][nx] = 0;

            s.push({nx, ny});
        } else {
            s.pop();
        }
    }
}
