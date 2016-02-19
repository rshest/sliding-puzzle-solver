#ifndef __NPUZZLE__
#define __NPUZZLE__

#include <array>


template <int N = 3, int M = N>
class npuzzle {
public:
    typedef std::array<int8_t, N*M> cell_arr;
    typedef int16_t move;

    struct position {
        cell_arr cells;
        int8_t   blank_pos;

        position(const int8_t* in_cells = nullptr) : 
            blank_pos(-1) {
            if (in_cells) {
                std::copy(in_cells, in_cells + N*M, cells.begin());
                blank_pos = std::find(cells.begin(), cells.end(), 0) - cells.begin();
            } else {
                for (int i = 0; i < N*M; i++) cells[i] = i + 1;
                cells[M*N - 1] = 0;
            }
        }

        size_t operator () () const {
            std::size_t res = 0;
            for (auto& v : cells) {
                res ^= v + 0x9e3779b9 + (res << 6) + (res >> 2);
            }
            return res;
        }

        bool operator == (const position& rhs) const {
            return cells == rhs.cells;
        }
    };

    void get_moves(const position& pos, std::vector<move>& res) const {
        int bx = pos.blank_pos%N;
        int by = pos.blank_pos/N;
        if (bx > 0    ) res.push_back(-1);
        if (bx < N - 1) res.push_back( 1);
        if (by > 0    ) res.push_back(-N);
        if (by < M - 1) res.push_back( N);
    }

    inline float get_cost(const position& pos, const move& m) const {
        return 1.0f;
    }

    inline float estimate_cost(const position& source) const {
        int res = 0;
        for (int i = 0; i < N*M; i++) {
            int vx = i%N;
            int vy = i/N;
            int v = source.cells[i];
            if (v > 0) {
                int tx = (v - 1) % N;
                int ty = (v - 1) / N;
                res += abs(vx - tx) + abs(vy - ty);
            }
        }
        return (float)res;
    }

    inline bool is_target(const position& pos) const {
        const int ncells = N*M;
        for (int i = 1; i < ncells; i++) {
            if (pos.cells[i - 1] != i) return false;
        }
        return pos.cells[ncells - 1] == 0;
    }


    inline void apply_move(const position& pos, const move& m, position& new_pos) const {
        new_pos.cells = pos.cells;
        new_pos.blank_pos = pos.blank_pos + m;
        std::swap(new_pos.cells[new_pos.blank_pos], new_pos.cells[pos.blank_pos]);
    }

    inline void unapply_move(const position& pos, const move& m, position& new_pos) const {
        new_pos.cells = pos.cells;
        new_pos.blank_pos = pos.blank_pos - m;
        std::swap(new_pos.cells[new_pos.blank_pos], new_pos.cells[pos.blank_pos]);
    }
};



#endif // __NPUZZLE__
