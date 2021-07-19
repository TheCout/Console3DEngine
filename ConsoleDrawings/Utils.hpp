#pragma once
#include <Windows.h>
#include <iostream>
#include <ctime>
#include <conio.h>
#include <sstream>
#include <vector>
#include <fstream>
#include <strstream>


#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


// STRUCTURES
struct triangle
{
    glm::vec4 p[3];
    COLORREF color;
};

struct mesh
{
    std::vector<triangle> tris;

    bool LoadFromObjectFile(std::string file_name)
    {
        std::ifstream f(file_name);
        if (!f.is_open())
            return false;

        // Local cache of verts
        std::vector<glm::vec4> verts;

        while (!f.eof())
        {
            char line[128];
            f.getline(line, 128);

            std::strstream s;
            s << line;

            char junk;

            if (line[0] == 'v')
            {
                glm::vec4 v;
                s >> junk >> v.x >> v.y >> v.z;
                v.w = 1;
                verts.push_back(v);
            }

            if (line[0] == 'f')
            {
                int f[3];
                s >> junk >> f[0] >> f[1] >> f[2];
                tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
            }
        }
        return true;
    }
};

struct mat4x4
{
    float m[4][4] = { 0 };
};


inline double clockToMilliseconds(clock_t ticks) {
    return (ticks / (double)CLOCKS_PER_SEC) * 1000.0;
}


inline void wait()
{
    std::string dummy;
    std::getline(std::cin, dummy);
}


inline void disableScrollBar(HANDLE handle, CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo)
{
    COORD new_screen_buffer_size;
    new_screen_buffer_size.X = screenBufferInfo.srWindow.Right -
        screenBufferInfo.srWindow.Left + 1; // Columns
    new_screen_buffer_size.Y = screenBufferInfo.srWindow.Bottom -
        screenBufferInfo.srWindow.Top + 1; // Rows
    // Set new buffer size
    SetConsoleScreenBufferSize(handle, new_screen_buffer_size);
}
