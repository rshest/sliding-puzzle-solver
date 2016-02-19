#ifndef __SLIDING_PUZZLE__
#define __SLIDING_PUZZLE__
#include <functional>

#include "astar.hpp"

struct offset {
    int8_t  dx, dy;

    inline bool operator == (const offset& rhs) const {
        return dx == rhs.dx && dy == rhs.dy;
    }

    friend inline offset operator -(const offset& a, const offset& b) {
        return{ a.dx - b.dx, a.dy - b.dy };
    }

    friend inline offset operator +(const offset& a, const offset& b) {
        return{ a.dx + b.dx, a.dy + b.dy };
    }
};

static constexpr offset DIR_OFFSETS[] = { { 1, 0 },{ 0, 1 },{ -1, 0 },{ 0, -1 } };
static constexpr int NUM_DIR = 4;

class sliding_puzzle {
public:
    struct piece {
        std::vector<uint32_t>   mask_rows;
        offset                  offs;      //  bounding box top left
        int8_t                  width;     //  bounding box width
        int8_t                  height;    //  bounding box height

        piece(int nrows, int ncols) {
            assert(ncols < sizeof(uint32_t) * 8);
            mask_rows.resize(nrows, 0);
            width = height = 0;
            offs = { (int8_t)ncols, (int8_t)nrows };
        }

        void set(int8_t row, int8_t col) {
            mask_rows[row] |= 1 << col;
            offs.dx = std::min(offs.dx, col);
            offs.dy = std::min(offs.dy, row);
            width = std::max(width, (int8_t)(col - offs.dx + 1));
            height = std::max(height, (int8_t)(row - offs.dy + 1));
        }

		bool is_set(int8_t row, int8_t col) const {
			return ((1 << col) & mask_rows[row]) != 0;
		}

        bool empty() const { return width == 0 || height == 0; }

        bool overlaps(const piece& p, const offset& poffs) const {
            for (int i = 0; i < p.height; i++) {
                int dx = poffs.dx - p.offs.dx;
                auto row = p.mask_rows[i + p.offs.dy];
                auto row_offs = dx > 0 ? (row << dx) : (row >> -dx);
                if (mask_rows[i + poffs.dy] & row_offs) return true;
            }
            return false;
        }

        void xor_with(const piece& p, const offset& poffs) {
            for (int i = 0; i < p.height; i++) {
                int dx = poffs.dx - p.offs.dx;
                auto row = p.mask_rows[i + p.offs.dy];
                mask_rows[i + poffs.dy] ^= dx > 0 ? (row << dx) : (row >> -dx);
            }
        }
    };

    struct move {
        uint8_t  piece_id;
        int8_t   dx;
        int8_t   dy;

        bool operator == (const move& rhs) const {
            return piece_id == rhs.piece_id && dx == rhs.dx && dy == rhs.dy;
        }
    };

    struct position {
        std::vector<offset> offsets;

        position(int npieces = 0) {
            if (npieces > 0) {
                offsets.resize(npieces);
                std::fill(offsets.begin(), offsets.end(), offset{ 0, 0 });
            }
        }

        size_t operator () () const {
            size_t res = 0;
            for (const auto& v : offsets) {
                res = res*101 + *((const uint16_t*)&v);
            }
            return res;
        }

        bool operator ==(const position& rhs) const {
            return memcmp(&offsets[0], &rhs.offsets[0], sizeof(offset)*offsets.size()) == 0;
        }
    };

    void get_moves(const position& pos, std::vector<move>& res) const {
        const int npieces = _pieces.size();
        piece mask(_rows, _cols);
        for (int i = 0; i < npieces; i++) {
            mask.xor_with(_pieces[i], pos.offsets[i]);
        }

        std::vector<char> visited(_cols*_rows);
        //  iterate through possible moves
        for (int i = 0; i < npieces; i++) {
            //  remove current piece from the mask
            const piece& piece = _pieces[i];
            mask.xor_with(piece, pos.offsets[i]);

            //  gather all the accessible offsets for this piece recursively (depth first)
            std::fill(visited.begin(), visited.end(), 0);
            std::function<void(int8_t, int8_t)> gather_moves = [&](int8_t dx, int8_t dy) {
                visited[(pos.offsets[i].dx + dx) + (pos.offsets[i].dy + dy)*_cols] = true;
                for (int j = 0; j < NUM_DIR; j++) {
                    int8_t dx1 = dx + DIR_OFFSETS[j].dx;
                    int8_t dy1 = dy + DIR_OFFSETS[j].dy;
                    offset offs = { pos.offsets[i].dx + dx1, pos.offsets[i].dy + dy1 };
                    //  find if can move this piece in this direction
                    bool in_bounds = (offs.dx >= 0) & (offs.dy >= 0) &
                        (offs.dx + piece.width <= _cols) &
                        (offs.dy + piece.height <= _rows);
                    if (!in_bounds || visited[offs.dx + offs.dy*_cols] ||
                        mask.overlaps(piece, { offs.dx, offs.dy })) continue;
                    res.push_back({ (uint8_t)i, dx1, dy1 });
                    gather_moves(dx1, dy1);
                }
            };
            gather_moves(0, 0);
            //  restore current piece in the mask
            mask.xor_with(_pieces[i], pos.offsets[i]);
        }
    }

    inline float get_cost(const position& pos, const move& m) const {
        return 1.0f;
    }

    inline float estimate_cost(const position& source) const {
        const int npieces = source.offsets.size();
        float res = 0.0f;
        for (const auto& m : _target) {
            const offset& offs = source.offsets[m.piece_id];
            res += (float)(abs(offs.dx - m.dx) + abs(offs.dy - m.dy));
        }
        
        //  note that manhattan distance heuristics is non-admissible for complex moves
        //  we mitigate it somewhat by lowering it down,
        //  trying to find a balance between closeness to the optimal solution and
        //  the search space size
        return res/2;
    }

    inline bool is_target(const position& pos) const {
        for (const auto& m : _target) {
            const offset& offs = pos.offsets[m.piece_id];
            if (offs.dx != m.dx || offs.dy != m.dy) return false;
        }
        return true;
    }

    inline void apply_move(const position& pos, const move& m, position& new_pos) const {
        new_pos = pos;
        offset& offs = new_pos.offsets[m.piece_id];
        offs.dx += m.dx;
        offs.dy += m.dy;
    }

    inline void unapply_move(const position& pos, const move& m, position& new_pos) const {
        new_pos = pos;
        offset& offs = new_pos.offsets[m.piece_id];
        offs.dx -= m.dx;
        offs.dy -= m.dy;
    }

    position get_source() const {
        const size_t npieces = _pieces.size();
        position res(npieces);
        for (size_t i = 0; i < npieces; i++) res.offsets[i] = _pieces[i].offs;
        return res;
    }

    void parse(std::istream& is) {
        std::string line;
        std::vector<std::string> lines;

        _cols = 0;
        bool is_source = false;
        while (std::getline(is, line)) {
            if (line.empty()) {
                is_source = true;
                break;
            }
            lines.push_back(line);
            _cols = std::max(_cols, (int)line.size());
        }

        _rows = lines.size();
        for (int i = 0; i < _rows; i++) {
            const std::string& line = lines[i];
            for (int j = 0; j < (int)line.size(); j++) {
                char c = line[j];
                int pid = (c <= '9') ? c - '0' : c - 'A' + 10;
                if (pid < 0) continue;
                if ((int)_pieces.size() <= pid)
                    _pieces.resize(pid + 1, sliding_puzzle::piece(_rows, _cols));
                _pieces[pid].set(i, j);
            }
        }

        //  parse the target part
        if (is_source) {
            sliding_puzzle target;
            target.parse(is);
            uint8_t nt = (uint8_t)target._pieces.size();
            for (uint8_t i = 0; i < nt; i++) {
                const piece& p = target._pieces[i];
                if (!p.empty()) {
                    _target.push_back({ i, p.offs.dx, p.offs.dy });
                }
            }
        }
        
    }

	static std::string move_str(const sliding_puzzle::move& move) {
		std::stringstream ss;
		ss << (int)move.piece_id;
		if (move.dx != 0) {
			ss << "LR"[move.dx > 0];
			if (abs(move.dx) > 1) ss << abs(move.dx);
		}
		if (move.dy != 0) {
			ss << "UD"[move.dy > 0];
			if (abs(move.dy) > 1) ss << abs(move.dy);
		}
		return ss.str();
	}

    static std::string moves_str(std::vector<sliding_puzzle::move>& moves) {
        std::stringstream ss;
        int nmoves = moves.size();
        for (int i = 0; i < nmoves; i++) {
			ss << move_str(moves[i]);
            if (i < nmoves - 1) ss << ",";
        }
        return ss.str();
    }

private:
    std::vector<piece>  _pieces;
    std::vector<move>   _target;
    int                 _rows, _cols;

	friend class sliding_puzzle_svg;
};

#endif // __SLIDING_PUZZLE__
