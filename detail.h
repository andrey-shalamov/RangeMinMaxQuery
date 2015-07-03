//
// Created by andrey on 7/3/15.
//

#ifndef RANGEMINMAXQUERY_DETAIL_H
#define RANGEMINMAXQUERY_DETAIL_H

#include <cstring>
#include <ctime>
#include <iostream>

namespace detail {

    template<typename T>
    T* memory_alloc_1d(std::size_t size_) {
        return new T[size_];
    }

    template<typename T>
    void memory_set_1d(T* p, char val, std::size_t size_) {
        if (p == nullptr)
            return;
        memset(p, val, sizeof(T)*size_);
    }

    template<typename T>
    void memory_fill_1d(T* p, T val, std::size_t size_) {
        if (p == nullptr)
            return;
        std::fill(p, p+size_, val);
    }

    template<typename T>
    void memory_free_1d(T*& p) {
        if (p == nullptr)
            return;
        delete[] p;
        p = nullptr;
    }

    template<typename T>
    T** memory_alloc_2d(std::size_t size_x, std::size_t size_y) {
        T** pp = new T*[size_x];
        T* p = new T[size_x*size_y];
        for (int i=0; i<size_x; ++i) {
            pp[i] = &p[i*size_y];
        }
        return pp;
    }

    template<typename T>
    void memory_set_2d(T** pp, char val, std::size_t size_x, std::size_t size_y) {
        if (pp == nullptr)
            return;
        memset(&pp[0][0], val, sizeof(T)*size_x*size_y);
    }

    template<typename T>
    void memory_fill_2d(T** pp, T val, std::size_t size_x, std::size_t size_y) {
        if (pp == nullptr)
            return;
        T* begin_ = &pp[0][0];
        T* end_ = begin_ + size_x * size_y;
        std::fill(begin_, end_, val);
    }

    template<typename T>
    void memory_free_2d(T**& pp) {
        if (pp == nullptr)
            return;
        T* p = &pp[0][0];
        delete[] pp;
        delete[] p;
        pp = nullptr;
    }

    /*
     * simple timer by clocks
     */
    struct timer {
        timer(const std::string& name) {
            name_ = name;
            start_ = std::clock();
        }
        ~timer() {
            std::cout << "timer " << name_ << " : ";
            std::clock_t end_ = std::clock();
            float sec_by_clocks = 1.f * (end_ - start_) / CLOCKS_PER_SEC;
            std::cout << sec_by_clocks << " sec." << std::endl;
        }
        std::string name_;
        std::clock_t start_;
    };
}

#endif //RANGEMINMAXQUERY_DETAIL_H
