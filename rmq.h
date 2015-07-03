//
// Created by andrey on 7/3/15.
//

#ifndef RANGEMINMAXQUERY_RMQ_H
#define RANGEMINMAXQUERY_RMQ_H

#include <limits>
#include <stdexcept>

#include "detail.h"

namespace rmq {

    /*
     * next terms mean:
     * RMQ - range minimum/maximum query
     * static - input data doesn't change
     * dynamic - input data may change
     * offline - we know all of queries before
     * online - we don't know all of queries
     */

    enum rmq_type {
        rmq_min = 0,
        rmq_max
    };

    template<typename T, rmq_type type = rmq_min>
    class rmq_abstract {
    public:
        void init(T* a, std::size_t size) {
            input_ = a;
            size_ = size;
        }
        rmq_type getType() const { return type; }
        T* getData() const { return input_; }
        std::size_t getSize() const { return size_; }

        /*
         * pre-processing
         */
        void preprocessing() {
            detail::timer timer_(name_ + " pre-processing time: ");
            preprocessing_impl();
        }

        /*
         * min/max query for interval [l,r]
         */
        T query(int l, int r) {
            if (l > r || l < 0 || r < 0 || l >= size_ || r >= size_) {
                throw std::runtime_error("range isn't correct");
            }
            detail::timer timer_(name_ + " query time: ");
            return query_impl(l, r);
        }
    protected:

        virtual void preprocessing_impl() {}
        virtual T query_impl(int l, int r) = 0;

        T* input_ = nullptr;
        std::size_t size_ = 0;
        std::string name_;
    };

    /*
     * RMQ static online - simple linear algorithm
     * Memory O(1)
     * Pre-processing O(1)
     * Query O(n)
     */
    template<typename T, rmq_type type = rmq_min>
    class rmq_static_simple_linear_solver : public rmq_abstract<T, type> {
    public:
        typedef T value_type;
        typedef rmq_abstract<T, type> base_t;
        rmq_static_simple_linear_solver() {
            base_t::name_ = "simple linear algorithm";
        }
    protected:
        virtual value_type query_impl(int l, int r) {
            value_type min_ = std::numeric_limits<value_type>::max();
            value_type max_ = std::numeric_limits<value_type>::min();
            for (int i=l; i<=r; ++i) {
                min_ = std::min(min_, base_t::input_[i]);
                max_ = std::max(max_, base_t::input_[i]);
            }
            //std::max_element(&a[r], &a[l]);
            //std::min_element(&a[r], &a[l]);
            return type == rmq_min ? min_ : max_;
        }
    };

    /*
     * RMQ static online - algorithm uses full table
     * Memory O(n^2)
     * Pre-processing O(n^2)
     * Query O(1)
     */
    template<typename T, rmq_type type = rmq_min>
    class rmq_static_full_table_solver : public rmq_abstract<T, type> {
    public:
        typedef T value_type;
        typedef rmq_abstract<T, type> base_t;
        rmq_static_full_table_solver() {
            base_t::name_ = "full table";
        }
        virtual ~rmq_static_full_table_solver() {
            detail::memory_free_2d(pp_full_table_);
        }
    protected:
        virtual void preprocessing_impl() override {
            pp_full_table_ = detail::memory_alloc_2d<value_type>(base_t::size_, base_t::size_);
            detail::memory_set_2d(pp_full_table_, 0, base_t::size_, base_t::size_);
            typedef const value_type& (*bin_func_t)(const value_type& a, const value_type& b);
            bin_func_t bin_func = type == rmq::rmq_min ? &std::min<value_type> : &std::max<value_type>;
            for (int xy=0; xy<base_t::size_; ++xy) {
                pp_full_table_[xy][xy] = base_t::input_[xy];
                for (int r=xy+1; r<base_t::size_; ++r) {
                    pp_full_table_[xy][r] = bin_func(pp_full_table_[xy][r-1], base_t::input_[r]);
                }
            }
        }

        virtual value_type query_impl(int l, int r) {
            return pp_full_table_[l][r];
        }

        value_type** pp_full_table_ = nullptr;
    };

    /*
     * RMQ static online - algorithm uses sparse table
     * Memory O(n^2)
     * Pre-processing O(n^2)
     * Query O(1)
     */
    template<typename T, rmq_type type = rmq_min>
    class rmq_static_sparse_table_solver : public rmq_abstract<T, type> {
    public:
        typedef T value_type;
        typedef rmq_abstract<T, type> base_t;
        rmq_static_sparse_table_solver() {
            base_t::name_ = "sparse table";
        }
        virtual ~rmq_static_sparse_table_solver() {
            detail::memory_free_2d(pp_sparse_table_);
        }
    protected:
        virtual void preprocessing_impl() override {

        }

        virtual value_type query_impl(int l, int r) {
            return pp_sparse_table_[l][r];
        }

        value_type** pp_sparse_table_ = nullptr;
    };
}
#endif //RANGEMINMAXQUERY_RMQ_H
