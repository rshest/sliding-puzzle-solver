#include "stdafx.h"
#include "CppUnitTest.h"

#include <iostream>

#include <pool_alloc.hpp>

#include <astar.hpp>
#include <gridmap.hpp>
#include <npuzzle.hpp>
#include <sliding_puzzle.hpp>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace test
{

typedef std::vector<gridmap::move> moves_vec;

std::string compare(const moves_vec& expected, const moves_vec& real) {
    std::stringstream ss;
    if (expected.size() != real.size()) {
        ss << "Expected size: " << expected.size() << ", real size: " << real.size();
        return ss.str();
    }
    for (size_t i = 0; i < expected.size(); i++) {
        auto& em = expected[i];
        auto& rm = real[i];

        if (em.dx != rm.dx || em.dy != rm.dy) {
            ss << "Moves differ at " << i << ", expected [" << em.dx << ", " << em.dy << "], got [" <<
                rm.dx << ", " << rm.dy << "]";
            return ss.str();
        }
        
    }
    return "";
}

TEST_CLASS(test_gridmap)
{
public:

    TEST_METHOD(test_gridmap0)
    {
        gridmap f(3,
            ".X."
            "..."
            "X.X");

        f.target = { 2, 0 };
        astar<gridmap, gridmap::position, gridmap::move> solver(f, { 0, 0 });

        solver.solve();

        moves_vec solution;
        bool hasSolution = solver.get_solution(solution);
        Assert::IsTrue(hasSolution);

        moves_vec res{{0, 1}, {1, 0}, {1, 0}, {0, -1}};
        Assert::AreEqual(std::string(""), compare(res, solution));
    }

    TEST_METHOD(test_gridmap1)
    {
        gridmap f(7, 
            "....XX."
            ".XX.XX."
            "...X..X"
            ".X.....");

        f.target = { 6, 3 };
        astar<gridmap, gridmap::position, gridmap::move> solver(f, {0, 0});
        
        solver.solve();
        
        moves_vec solution;
        bool hasSolution = solver.get_solution(solution);
        Assert::IsTrue(hasSolution);

        moves_vec res{{0, 1}, {0, 1}, {1, 0}, {1, 0}, 
            {0, 1}, {1, 0}, {1, 0}, {1, 0}, {1, 0}};
        Assert::AreEqual(std::string(""), compare(res, solution));
    }

    TEST_METHOD(test_gridmap2)
    {
        gridmap f(3,
            ".X."
            "..X"
            "X.X");

        f.target = { 2, 0 };
        astar<gridmap, gridmap::position, gridmap::move> solver(f, { 0, 0 });

        solver.solve();

        moves_vec solution;
        bool hasSolution = solver.get_solution(solution);
        Assert::IsFalse(hasSolution);
    }

    TEST_METHOD(test_gridmap3)
    {
        gridmap f(8,
            "....XX.."
            ". X...X."
            ".XX.X.X."
            "....X.X."
            "..X.....");

        f.target = { 6, 0 };
        astar<gridmap, gridmap::position, gridmap::move> solver(f, { 0, 2 });

        solver.solve();

        moves_vec solution;
        bool hasSolution = solver.get_solution(solution);
        Assert::IsTrue(hasSolution);

        moves_vec res{ { 0, 1 },{ 1, 0 },{ 1, 0 },{ 1, 0 },
        { 0, 1 },{ 1, 0 },{ 1, 0 },{ 1, 0 },{ 1, 0 },
        { 0, -1 },{ 0, -1 },{ 0, -1 },{ 0, -1 },{ -1, 0 } };
        Assert::AreEqual(std::string(""), compare(res, solution));
    }
};


TEST_CLASS(test_npuzzle)
{
public:
    
    TEST_METHOD(test_npuzzle8)
    {
        struct np_test {
            std::string cells;
            int min_moves;
        };
        std::vector<np_test> tests = {
            { "123405786", 2 },
            { "123745086", 4 },
            { "123480765", 5 },
            { "123076548", 7 },
            { "413726580", 8 },
            { "162530478", 9 },
            { "512630478", 11 },
            { "126350478", 13 },
            { "356148072", 16 },
            { "160842753", 18 },
            { "436871052", 18 },
            { "302651478", 21 },
            { "012345678", 22 },
            { "503284671", 23 },
            { "874320651", 25 },
            { "876543021", 28 },
            { "876543210", 30 }
        };

        typedef npuzzle<3> np8;

        for (auto test : tests) {
            np8 np;
            std::array<int8_t, 9> start;
            for (int i = 0; i < 9; i++) {
                start[i] = test.cells[i] - '0';
            }
            astar<np8> solver(np, np8::position(&start[0]));

            solver.solve();

            std::vector<np8::move> solution;
            bool hasSolution = solver.get_solution(solution);
            Assert::IsTrue(hasSolution);

            Assert::AreEqual(test.min_moves, (int)solution.size());
        }
    }


    TEST_METHOD(test_npuzzle2)
    {
        typedef npuzzle<4> npuzzle15;
        npuzzle15 np;
        const int8_t start[] =
        { 1,  2, 13,  4,
          7, 14, 10, 15,
          9,  0,  5, 11,
          8,  6,  3, 12 };

        astar<npuzzle15, npuzzle15::position, npuzzle15::move>
            solver(np, npuzzle15::position(start));

        solver.solve();

        std::vector<npuzzle15::move> solution;
        bool hasSolution = solver.get_solution(solution);
        Assert::IsTrue(hasSolution);
        for (auto s : solution) {
            std::cout << s << " ";
        }

        Assert::AreEqual(45, (int)solution.size());
    }

    TEST_METHOD(test_npuzzle3)
    {
        typedef npuzzle<4> npuzzle15;
        npuzzle15 np;
        const int8_t start[] =
        { 6, 14, 3,  13,
          7,  1, 0,   5,
          8, 10, 2,  12,
         15,  9, 11,  4};

        astar<npuzzle15, npuzzle15::position, npuzzle15::move>
            solver(np, npuzzle15::position(start));

        solver.solve();

        std::vector<npuzzle15::move> solution;
        bool hasSolution = solver.get_solution(solution);
        Assert::IsTrue(hasSolution);

        Assert::AreEqual(51, (int)solution.size());
    }
};


TEST_CLASS(test_sliding_puzzle)
{
public:

    TEST_METHOD(test_yank)
    {
        sliding_puzzle sp;
        std::stringstream ss;
        ss << "24600\n88611\n7..53\n\n..65.\n42600\n88311";
        sp.parse(ss);
        
        astar<sliding_puzzle> solver(sp, sp.get_source());

        solver.solve();

        std::vector<sliding_puzzle::move> solution;
        bool hasSolution = solver.get_solution(solution);
        
        Assert::IsTrue(hasSolution);
        Assert::AreEqual(13, (int)solution.size());

    }
};

TEST_CLASS(test_pool_alloc)
{
public:

    struct elem {
        static int s_elems;

        int cookie;
        int val;

        elem() :
            cookie(777),
            val(rand()) {
            s_elems++;
        }

        ~elem() {
            s_elems--;
        }

        operator int() const {
            return val;
        }

        void operator =(int v) {
            val = v;
        }
    };

    TEST_METHOD(test_pool_alloc0)
    {
        {
            pool_alloc<elem> alloc(1, 2.0);
            auto p1 = alloc.allocate();
            *p1 = 11;

            auto p2 = alloc.allocate();
            *p2 = 12;

            auto p3 = alloc.allocate();
            *p3 = 13;

            auto p4 = alloc.allocate();
            *p4 = 14;

            Assert::IsTrue(11 == (int)*p1);
            Assert::IsTrue(12 == (int)*p2);
            Assert::IsTrue(13 == (int)*p3);
            Assert::IsTrue(14 == (int)*p4);

            Assert::AreEqual(4, elem::s_elems);
            Assert::AreEqual(777, p1->cookie);

            alloc.free(p1);
            alloc.free(p4);

            Assert::AreEqual(2, elem::s_elems);

            auto p5 = alloc.allocate();
            *p5 = 15;

            uint64_t* p0 = new uint64_t();

            auto p6 = alloc.allocate();
            *p6 = 16;

            Assert::AreEqual(4, elem::s_elems);

            Assert::IsTrue(p5 == p4);
            Assert::IsTrue(p6 == p1);

            Assert::IsTrue(12 == (int)*p2);
            Assert::IsTrue(13 == (int)*p3);
            Assert::IsFalse(11 == (int)*p1);
            Assert::IsFalse(14 == (int)*p4);

            Assert::IsTrue(15 == (int)*p5);
            Assert::IsTrue(16 == (int)*p6);

            Assert::AreEqual(3, (int)alloc._pages.size());

            delete p0;
            alloc.free(p3);
            alloc.free(p6);
            alloc.free(alloc.allocate());
            alloc.free(p2);
            alloc.free(p6);
            alloc.free(alloc.allocate());

            Assert::AreEqual(0, elem::s_elems);
        }
        Assert::AreEqual(0, elem::s_elems);
    }
};

int test_pool_alloc::elem::s_elems = 0;

}