#ifndef __SLIDING_PUZZLE_SVG__
#define __SLIDING_PUZZLE_SVG__

#include <string>
#include <map>

#include "sliding_puzzle.hpp"


const std::map<std::string, std::vector<const char*>>
COLORMAPS = {   
    {"c12", {
    "8dd3c7", "ffffb3", "bebada", "fb8072", "80b1d3", "fdb462",
    "b3de69", "fccde5", "d9d9d9", "bc80bd", "ccebc5", "ffed6f"}},
    {"c20", {
    "1f77b4", "aec7e8", "ff7f0e", "ffbb78", "2ca02c",
    "98df8a", "d62728", "ff9896", "9467bd", "c5b0d5",
    "8c564b", "c49c94", "e377c2", "f7b6d2", "7f7f7f",
    "c7c7c7", "bcbd22", "dbdb8d", "17becf", "9edae5"}},
    {"spectral", {
    "9e0142","d53e4f","f46d43","fdae61","fee08b","ffffbf",
    "e6f598","abdda4","66c2a5","3288bd","5e4fa2"
    }},
    {"puor", {
    "7f3b08","b35806","e08214","fdb863","fee0b6","f7f7f7",
    "d8daeb","b2abd2","8073ac","542788","2d004b"
    }},
    {"d3b", {
    "393b79", "5254a3", "6b6ecf", "9c9ede", "637939", "8ca252",
    "b5cf6b", "cedb9c", "8c6d31", "bd9e39", "e7ba52", "e7cb94",
    "843c39", "ad494a", "d6616b", "e7969c", "7b4173", "a55194",
    "ce6dbd", "de9ed6"}}
};

class sliding_puzzle_svg {
public:
    int             columns;
    float           cell_width, cell_height;
    float           border_width, border_margin;
    float           caption_height;
    std::string     colormap;
    bool            show_turn_numbers;

    sliding_puzzle_svg() : 
        cell_width(20.0f), cell_height(20.0f),
        border_width(0.1f), border_margin(0.05f),
        columns(5), colormap("c12"), 
        show_turn_numbers(true), caption_height(15.0f) {}

    void gen_piece_path(std::ostream& os, const sliding_puzzle::piece& piece) const {
        std::string res;
        for (int j = 0; j < piece.height; j++) {
            for (int i = 0; i < piece.width; i++) {
                int cx = i + piece.offs.dx;
                int cy = j + piece.offs.dy;
                if (piece.is_set(cy, cx)) {
                    float x1 = i*cell_width;
                    float x2 = (i + 1)*cell_width;
                    float y1 = j*cell_height;
                    float y2 = (j + 1)*cell_height;

                    os << "M";
                    os << x1 << " " << y1 << " ";
                    os << x2 << " " << y1 << " ";
                    os << x2 << " " << y2 << " ";
                    os << x1 << " " << y2 << " z ";
                }
            }
        }
    }

    void gen_piece(std::ostream& os, const sliding_puzzle::piece& piece, 
        const offset& offset, int id, float opacity = 1.0f) {
        os << "<path d=\""; 
        gen_piece_path(os, piece);
        auto cmit = COLORMAPS.find(colormap);
        if (cmit == COLORMAPS.end()) cmit = COLORMAPS.find("c12");
        auto cm = cmit->second;
        const char* color = cm[id%cm.size()];
        float dx = offset.dx*cell_width;
        float dy = offset.dy*cell_height;
        os << "\" " <<
            "fill=\"#" << color << "\" opacity=\"" << opacity << "\" " << 
            "transform=\"translate(" << dx << "," << dy << ")\"" << ">" << 
            "<title>" << "Piece: \"" << id << "\"</title>" << "</path>";
    }

    void gen_board(std::ostream& os, const sliding_puzzle& sp, 
        const sliding_puzzle::position& pos, const sliding_puzzle::move* pshow_move = NULL) {
        const int npieces = sp._pieces.size();
        for (int i = 0; i < npieces; i++) {
            os << "\n  ";
            const sliding_puzzle::piece& piece = sp._pieces[i];
            gen_piece(os, piece, pos.offsets[i], i);
        }
        if (pshow_move) {
            //  the ghost move piece
            int8_t piece_id = pshow_move->piece_id;
            offset offs = pos.offsets[piece_id];
            offs.dx += pshow_move->dx;
            offs.dy += pshow_move->dy;
            gen_piece(os, sp._pieces[piece_id], offs, piece_id, 0.2f);
        }
    }

    void gen_solution(std::ostream& os, const sliding_puzzle& sp,
        const sliding_puzzle::position& source, std::vector<sliding_puzzle::move>& solution) {
        os << "<svg xmlns=\"http://www.w3.org/2000/svg\" " <<
            "xmlns:xlink=\"http://www.w3.org/1999/xlink\" " <<
            "shape-rendering=\"crispEdges\">\n";
        os << "<defs> <pattern id=\"crosshatch\" patternUnits=\"userSpaceOnUse\" "
            "x=\"0\" y=\"0\" width=\"5\" height=\"5\"><g style=\"fill:none; stroke:#dde; stroke-width:1\">"
            "<path d=\"M0,0 l5,5\"/><path d=\"M5,0 l-5,5\"/></g></pattern></defs>";
        
        os << "<style>\n/* <![CDATA[ */\n " <<
            ".move_text { font-size:11px; font-family:Arial; fill:#0e004a; font-weight:bold; } \n"
            ".frame { fill: url(#crosshatch) #fff; stroke:#004a00; stroke-width:1;"
                " stroke-linecap:square; stroke-linejoin:round; }" <<
            ".text_bg { fill:white; opacity:0.4; rx:3; ry:3; }" <<
            "\n/* ]]> */'n</style>";

        int CELL_SIDE = 10;
        int nmoves = solution.size();
        sliding_puzzle::position pos = source;
        float border_w = border_width*cell_width;
        float border_m = border_margin*cell_width;

        float board_w = sp._cols*cell_width + (border_w + border_m)*2;
        float board_h = sp._rows*cell_height + (border_w + border_m)*2 + caption_height;
        
        int row = 0, col = 0;
        int cur_move = -1;
        while (true) {
            os << "<g transform=\"translate(" << board_w*col << "," << board_h*row << ")\">\n";
            os << "  <rect rx=\"5\" ry=\"5\" x=\"" << border_m << "\" y=\"" << (border_m + caption_height) << "\" " <<
                "width=\"" << (board_w - border_m * 2) << "\" height=\"" << (board_h - border_m*2 - caption_height) << "\" " <<
                "class=\"frame\"></rect>";

            os << "  <g transform=\"translate(" << (border_w + border_m) << "," << 
                (border_w + border_m + caption_height) << ")\">";
            gen_board(os, sp, pos, (cur_move < nmoves - 1) ? &solution[cur_move + 1] : nullptr);
            os << "\n  </g>\n";
            
            if (cur_move < nmoves - 1) {
                std::stringstream ss;
                if (show_turn_numbers) {
                    ss << (cur_move + 2) << ": ";
                }
                ss << sliding_puzzle::move_str(solution[cur_move + 1]);
                os << "\n  <text dy=\"1.2em\" dx=\"0.3em\" class=\"move_text\"> " << ss.str() << "</text>";
            }
            os << "</g>\n";
            col++;
            if (col > columns) {
                col = 0;
                row++;
            }
            cur_move++;
            if (cur_move >= nmoves) break;
            sp.apply_move(pos, solution[cur_move], pos);
        }
        os << "</svg>";
    }

};

#endif // __SLIDING_PUZZLE_SVG__
