//
// Created by andrey on 7/3/15.
//

#ifndef RANGEMINMAXQUERY_RMQ_H
#define RANGEMINMAXQUERY_RMQ_H

#include <limits>
#include <stdexcept>
#include <cmath>

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
        typedef const T& (*min_max_func_t)(const T&, const T&);
    public:
        void init(T* a, std::size_t size) {
            input_ = a;
            size_ = size;
            min_max_func_ = type == rmq_min ? &std::min<T> : &std::max<T>;
        }
        rmq_type getType() const { return type; }
        T* getData() const { return input_; }
        std::size_t getSize() const { return size_; }
        const T& getInitValue() const { return type == rmq_min ? std::numeric_limits<T>::max() : std::numeric_limits<T>::min(); }
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
        min_max_func_t min_max_func_;
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
            value_type o_res = base_t::getInitValue();
            for (int i=l; i<=r; ++i) {
                o_res = base_t::min_max_func_(o_res, base_t::input_[i]);
            }
            return o_res;
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
            for (int xy=0; xy<base_t::size_; ++xy) {
                pp_full_table_[xy][xy] = base_t::input_[xy];
                for (int r=xy+1; r<base_t::size_; ++r) {
                    pp_full_table_[xy][r] = base_t::min_max_func_(pp_full_table_[xy][r-1], base_t::input_[r]);
                }
            }
        }

        virtual value_type query_impl(int l, int r) {
            return pp_full_table_[l][r];
        }

        value_type** pp_full_table_ = nullptr;
    };

    /*
     * RMQ static online - algorithm uses sparse table (log2)
     * Memory O(n * log n)
     * Pre-processing O(n * log n)
     * Query O(1) TODO: not quite. look at log2
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
            std::size_t size_x = std::log2(base_t::size_) + 1;
            size_x = std::max(1ul, size_x);
            pp_sparse_table_ = detail::memory_alloc_2d<value_type>(size_x, base_t::size_);
            detail::memory_set_2d(pp_sparse_table_, 0, size_x, base_t::size_);
            for (int i=0; i<base_t::size_; ++i) {
                pp_sparse_table_[0][i] = base_t::input_[i];
            }
            for (int j=1; j<size_x; ++j) {
                int k = 1 << (j-1);
                for (int i=0; i<base_t::size_ && k<base_t::size_; ++i, ++k) {
                    pp_sparse_table_[j][i] = base_t::min_max_func_(pp_sparse_table_[j-1][i], pp_sparse_table_[j-1][k]);
                }
            }
        }

        virtual value_type query_impl(int l, int r) {
            std::size_t size_ = r-l+1;
            int log_2 = std::log2(size_);
            int k = 1 << log_2;
            return base_t::min_max_func_(pp_sparse_table_[log_2][l], pp_sparse_table_[log_2][r-k+1]);
        }

        value_type** pp_sparse_table_ = nullptr;
    };

    /*
     * RMQ static online - sqrt decomposition
     * Memory O(sqrt(n))
     * Pre-processing O(n)
     * Query O(sqrt(n))
     */
    template<typename T, rmq_type type = rmq_min>
    class rmq_static_sqrt_decomposition_solver : public rmq_abstract<T, type> {
    public:
        typedef T value_type;
        typedef rmq_abstract<T, type> base_t;
        rmq_static_sqrt_decomposition_solver() {
            base_t::name_ = "sqrt decomposition";
        }
        virtual ~rmq_static_sqrt_decomposition_solver() {
            detail::memory_free_1d(p_sqrt_);
        }
    protected:
        virtual void preprocessing_impl() override {
            float fStep = std::sqrt(base_t::size_);
            step_ = fStep;
            std::size_t size_x = fStep + 0.5f;
            p_sqrt_ = detail::memory_alloc_1d<value_type>(size_x);
            for (int i = 0; i < size_x; ++i) {
                p_sqrt_[i] = base_t::getInitValue();
                for (int j = i*step_; j < i*step_ + step_ && j < base_t::size_; ++j) {
                    p_sqrt_[i] = base_t::min_max_func_(p_sqrt_[i], base_t::input_[j]);
                }
            }
        }

        virtual value_type query_impl(int l, int r) {
            value_type o_res = base_t::getInitValue();
            if (r-l+1 < step_) {
                for (int i = l; i <= r; ++i) {
                    o_res = base_t::min_max_func_(o_res, base_t::input_[i]);
                }
            }
            else {
                int ll = l / step_;
                if (l % step_ > 0)
                    ++ll;
                int rr = r / step_;
                if (r & step_ != step_-1)
                    --rr;
                for (int i = ll*step_-1; i >= l ; --i) {
                    o_res = base_t::min_max_func_(o_res, base_t::input_[i]);
                }
                for (int i = ll; i <= rr; ++i) {
                    o_res = base_t::min_max_func_(o_res, p_sqrt_[i]);
                }
                for (int i = (rr+1)*step_; i <= r; ++i) {
                    o_res = base_t::min_max_func_(o_res, base_t::input_[i]);
                }
            }
            return o_res;
        }

        value_type* p_sqrt_ = nullptr;
        int step_ = 0;
    };
}
#endif //RANGEMINMAXQUERY_RMQ_H
