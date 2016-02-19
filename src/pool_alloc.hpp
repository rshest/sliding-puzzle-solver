#ifndef __POOL_ALLOC__
#define __POOL_ALLOC__

#include <vector>

namespace test { class test_pool_alloc; };

template <typename TElem>
class pool_alloc {
public:
    pool_alloc(size_t start_page_size = 256, float grow_rate = 1.0) :
        _grow_rate(grow_rate) {
        _pages.push_back({ start_page_size });
        init_page(_pages.back());
        _head = _pages.back().mem;
    }

    ~pool_alloc() {
        for (auto p : _pages) {
            ::free(p.mem);
        }
    }

    TElem* allocate() {
        TElem* next_free = *((TElem**)_head);
        TElem* res = _head;
        if (next_free == nullptr) {
            //  no more free cells, allocate new page
            _pages.push_back({ (size_t)(_pages.back().size*_grow_rate) });
            init_page(_pages.back());
            _head = _pages.back().mem;
        } else {
            _head = next_free;
        }
        new (res) TElem();
        return res;
    }

    void free(TElem* elem) {
        elem->~TElem();
        *((TElem**)elem) = _head;
        _head = elem;
    }

    uint64_t size() const { return _size; }

private:
    struct page
    {
        size_t  size;    //  number of elements in the page
        TElem*   mem;    //  pointer to the page memory
    };

    std::vector<page>   _pages;     //  pages registry
    TElem*              _head;      //  pointer to the first free element
    float               _grow_rate; //  page size grow ratio
    size_t              _size;      //  total size, bytes

    void init_page(page& p) {
        size_t page_size = p.size*sizeof(TElem);
        _size += page_size;
        p.mem = (TElem*)malloc(page_size);
        *((TElem**)&p.mem[p.size - 1]) = nullptr;
        for (int i = p.size - 2; i >= 0; i--) {
            *((TElem**)&p.mem[i]) = &p.mem[i + 1];
        }
    }

    static_assert(sizeof(TElem) >= sizeof(TElem*), 
        "Element size must be sufficient to contain a pointer type");

    friend class test::test_pool_alloc;
};

#endif