#ifndef __GRIDMAP__
#define __GRIDMAP__

#include <string>
#include <algorithm>

class gridmap {
public:
    gridmap(int w, const char* txt) :
        width(w), cells(txt) {
        height = cells.size() / w;
    }

    struct position {
        int x, y;

        size_t operator () () const {
            return x*y;
        }

        bool operator == (const position& rhs) const {
            return x == rhs.x && y == rhs.y;
        }
    };

    struct move {
        int dx, dy;
    };

    void get_moves(const position& pos, std::vector<move>& res) const {
        int idx = pos.x + pos.y*width;
        if (pos.x > 0 && cells[idx - 1] != 'X') res.push_back({ -1,  0 });
        if (pos.x < width - 1 && cells[idx + 1] != 'X') res.push_back({ 1,  0 });
        if (pos.y > 0 && cells[idx - width] != 'X') res.push_back({ 0, -1 });
        if (pos.y < height - 1 && cells[idx + width] != 'X') res.push_back({ 0,  1 });
    }

    float get_cost(const position& pos, const move& m) const {
        return 1.0f;
    }

    float estimate_cost(const position& source) const {
        int dx = abs(source.x - target.x);
        int dy = abs(source.y - target.y);
        return (float)(dx + dy);
    }

    bool is_target(const position& pos) const {
        return pos == target;
    }

    void apply_move(const position& pos, const move& m, position& new_pos) const {
        new_pos = { pos.x + m.dx, pos.y + m.dy };
    }

    void unapply_move(const position& pos, const move& m, position& new_pos) const {
        new_pos = { pos.x - m.dx, pos.y - m.dy };
    }


    int             width;
    int             height;
    std::string     cells;
    position        target;
};

#endif // __GRIDMAP__
