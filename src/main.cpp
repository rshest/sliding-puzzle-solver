#pragma once

#include <assert.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <type_traits>
#include <queue>
#include <unordered_set>
#include <chrono>

#include "cmd_param.hpp"

#include "sliding_puzzle.hpp"
#include "sliding_puzzle_svg.hpp"



int main(int argc, char *argv[]) { 
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <puzzle layout file> [--svg=svg_path] "
            "[--cw=CELL_WIDTH] [--ch=CELL_HEIGHT] [--columns=COLUMNS] [--colormap=COLORMAP]\n";
        return 1;
    }

    //  read the puzzle file
    std::string path(argv[1]);
    std::ifstream fs(path);
    if (!fs.is_open()) {
        std::cerr << "Could not open file: '" << path << "'\n";
        return 1;
    }

    sliding_puzzle sp;
    sp.parse(fs);
    fs.close();

    //  solve the puzzle
    astar<sliding_puzzle> solver(sp, sp.get_source());
    
    using namespace std::chrono;
    auto start = system_clock::now();
    solver.solve();

    //  print the result
    std::vector<sliding_puzzle::move> solution;
    bool solved = solver.get_solution(solution);
    std::cout << "Moves: " << sliding_puzzle::moves_str(solution) << 
        "\n(total of " << solution.size() << ")\nElapsed time: " << 
        duration_cast<seconds>(system_clock::now() - start).count() << " seconds\n" << std::endl;

    //  generate svg
    std::string svg_path;
    cmd_param param(argc, argv);
    if (param.get("svg", svg_path)) {

        sliding_puzzle_svg svg;
        param.get("cw", svg.cell_width);
        param.get("ch", svg.cell_height);
        param.get("columns", svg.columns);
        param.get("colormap", svg.colormap);

        std::ofstream svg_fs(svg_path);
        if (!svg_fs.is_open()) {
            std::cerr << "Could not create SVG file: '" << svg_path << "'\n";
            return 1;
        }

        svg.gen_solution(svg_fs, sp, sp.get_source(), solution);
        svg_fs.close();
    }

    return 0;
}

