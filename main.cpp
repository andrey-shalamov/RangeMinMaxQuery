#include <iostream>
#include <vector>
#include <cassert>
#include <ctime>

#include "rmq.h"

typedef unsigned char value_type;

int main() {
    int n = 0;
    std::cout << "Welcome to RMQ tester." << std::endl;
    std::cout << "Please, enter number of elements for tests: " << std::endl;
    std::cin >> n;

    std::srand(10/*std::time(nullptr)*/);
    std::vector<value_type> A(n);
    for (int i=0; i<A.size(); ++i) {
        A[i] = std::rand() % 256;
    }

    //------------------       - 0,1,2,3,4,5,6,7,8,9 ,10,11,12,13,14,15,16
    //std::vector<value_type> A = {8,2,3,5,1,4,7,8,9,13,53,4, 2, 54,76,78,12};


    typedef rmq::rmq_abstract<value_type, rmq::rmq_min> rmq_t;
    std::vector<rmq_t*> vecRMQ;
    vecRMQ.push_back(new rmq::rmq_static_simple_linear_solver<value_type, rmq::rmq_min>());
//    vecRMQ.push_back(new rmq::rmq_static_full_table_solver<value_type, rmq::rmq_min>());
    vecRMQ.push_back(new rmq::rmq_static_sparse_table_solver<value_type, rmq::rmq_min>());
    vecRMQ.push_back(new rmq::rmq_static_sqrt_decomposition_solver<value_type, rmq::rmq_min>());

    for (int i=0; i<vecRMQ.size(); ++i) {
        vecRMQ[i]->init(&A[0], A.size());
        vecRMQ[i]->preprocessing();
    }

    char c = 0;
    while (c != 'q') {
        std::cout << "enter any simbol to continue or 'q' for exit:" << std::endl;
        std::cin >> c;
        if (c == 'q') {
            std::cout << "bye" << std::endl;
            break;
        }

        int l,r;
        std::cout << "enter left index:" << std::endl;
        std::cin >> l;
        std::cout << "enter right index:" << std::endl;
        std::cin >> r;
        if (l > r) {
            std::cout << "wrong range" << std::endl;
            continue;
        }

        value_type res = 0;
        for (int i=0; i<vecRMQ.size(); ++i) {
            if (i == 0) {
                res = vecRMQ[i]->query(l, r);
            } else {
                value_type next_res = vecRMQ[i]->query(l,r);
                assert(next_res == res);
            }
        }
        std::cout << "min [" << l << ", " << r << "] = " << res << std::endl;
    }
    return 0;
}