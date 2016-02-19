#ifndef __ASTAR__
#define __ASTAR__

#include <vector>
#include <set>
#include <unordered_set>
#include <cassert>

#include "pool_alloc.hpp"

template <typename TProblem, typename TPos = TProblem::position, typename TMove = TProblem::move>
class astar {
public:
    astar(const TProblem& problem, const TPos& source) :
        _problem(problem), _source(source), _has_solution(false){
        node* pn0 = _pool.allocate();
        *pn0 = { _source, TMove(), 0.0f, 0.0f, true };
        _front.insert(pn0);
        _visited.insert(pn0);
    }

    ~astar() {
        for (node* n : _visited) _pool.free(n);
    }

    bool step() {
        node* pn0 = *(_front.begin());
        _front.erase(pn0);
        pn0->in_front = false;

        if (_problem.is_target(pn0->pos)) {
            //  found the target node, done
            _found_target = pn0->pos;
            _has_solution = true;
            return true;
        }

        //  expand possible moves from this node
        _moves.clear();
        _problem.get_moves(pn0->pos, _moves);

        for (const auto& move : _moves) {
            node* pnew = _pool.allocate();
            _problem.apply_move(pn0->pos, move, pnew->pos);

            pnew->cost_from_src  = pn0->cost_from_src + _problem.get_cost(pn0->pos, move);
            pnew->move           = move;
            pnew->in_front       = true;

            auto it = _visited.find(pnew);
            if (it == _visited.end()) {
                //  a completely new node 
                pnew->cost_to_dst = _problem.estimate_cost(pnew->pos);
                _front.insert(pnew);
                _visited.insert(pnew);
                continue;
            } 
            
            node* pn = *it;
            if (pn->in_front) {
                //  the node is already in the front queue
                if (pn->cost_from_src > pnew->cost_from_src) {
                    //  the new node is better, replace the old one in the front queue
                    _front.erase(pn);
                    pn->cost_from_src   = pnew->cost_from_src;
                    pn->cost_to_dst     = _problem.estimate_cost(pnew->pos);
                    pn->move            = move;
                    _front.insert(pn);
                }
            } else {
                //  the node had been already visited
                if (pn->cost_from_src > pnew->cost_from_src) {
                    //  found a better node, update the path/cost 
                    pn->cost_from_src   = pnew->cost_from_src;
                    pn->cost_to_dst     = _problem.estimate_cost(pnew->pos);
                    pn->move            = move;
                }
            }
            _pool.free(pnew);
        }

        return (_front.size() == 0);
    }

    void solve() {
        while (!step()) {
        }
    }

    bool get_solution(std::vector<TMove>& res) const {
        if (!_has_solution) return false;
        res.clear();
        node n{ _found_target };
        while (true) {
            auto& it = _visited.find(&n);
            if (it == _visited.end()) {
                return false;
            }
            const node* pn = *it;
            _problem.unapply_move(pn->pos, pn->move, n.pos);
            res.push_back(pn->move);
            if (n.pos == _source) {
                break;
            }
        }
        std::reverse(res.begin(), res.end());
        return true;
    }

private:
    struct node {
        TPos    pos;            //  node's position
        TMove   move;           //  move that lead to this pos
        float   cost_from_src;  //  cost from source node (real one)
        float   cost_to_dst;    //  heuristically estimated cost to target node
        bool    in_front;       //  whether this node is still in the front queue

        inline float total_cost() const {
            return cost_from_src + cost_to_dst;
        }

        struct hash {
            std::size_t operator() (const node* node) const {
                return node->pos();
            }
        };
        
        struct less {
            bool operator() (const node* nl, const node* nr) const {
                const float costl = nl->total_cost();
                const float costr = nr->total_cost();
                return (costl == costr) ? (nl < nr) : (costl < costr);
            }
        };

        struct eq {
            bool operator() (const node* lhs, const node* rhs) const {
                return lhs->pos == rhs->pos;
            }
        };
    };

    typedef std::set<node*, typename node::less> node_queue;
    typedef std::unordered_set<node*, typename node::hash, typename node::eq> node_set;
    typedef pool_alloc<node> node_pool;
    typedef std::vector<TMove> move_vec;

    const TProblem& _problem;       //  reference to the problem 
    TPos            _source;        //  starting position
    TPos            _found_target;  //  actually located target position (may differ from the real one)
    bool            _has_solution;  //  whether the solution has been actually found

    move_vec        _moves;         //  moves container (transient)

    node_queue      _front;         //  front node queue
    node_set        _visited;       //  visited node registry

    node_pool       _pool;          //  node object allocator
};

#endif