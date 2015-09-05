#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <time.h>
#include <cmath>
#include <memory.h>

#include "lsh_dfe.h"

namespace DFECode{
    // const values in this class
    const unsigned int LSH_DFE::DOUT = 16;
    const float LSH_DFE::formalize_max = 100000.0f;

    LSH_DFE::LSH_DFE(){}
    LSH_DFE::~LSH_DFE(){
        
    }

    void LSH_DFE::read_input_data(size_t features_num, size_t elements_num, std::string filename){
        in_d = InputData(elements_num, features_num);
        std::ifstream fin(filename.data());
        std::string title;
        getline(fin, title);
        int which;
        for(size_t i=0; i<elements_num; i++){
            fin>>which;
            for(size_t j=0; j<features_num; j++)
                fin>>in_d(i, j);
        }
        fin.close();
    }

    void LSH_DFE::generate_input_data(size_t features_num, size_t elements_num){
        srand(unsigned(time(NULL)));
        in_d = InputData(elements_num, features_num);
        for(size_t i=0; i<elements_num; i++)
            for(size_t j=0; j<features_num; j++)
                in_d(i, j) = float(rand()) / float(RAND_MAX) * formalize_max;
    }

    float LSH_DFE::get_clock(){
        clock_t t = clock();
        return float(t) / float(CLOCKS_PER_SEC);
    }

    void LSH_DFE::hash_set_dimensions(){
        // set the dimension of hash function
        m_hash = Functions(DOUT, in_d.size2());
    }

    void LSH_DFE::hash_generate(){
        // generate random hyperplane to do hashing
        for(unsigned int i=0; i<m_hash.size1(); i++)
            for(unsigned int j=0; j<m_hash.size2(); j++)
                m_hash(i, j) = float(rand()) / float(RAND_MAX);

        // the parameter of the hash function need to be formalized
        // dist = inner_product(point, hyperplane_parameter) / norm2(hyperplane_parameter)
        for(unsigned int i=0; i<m_hash.size1(); i++){
            float sqr_sum = 0.0f;
            for(unsigned int j=0; j<m_hash.size2(); j++)
                sqr_sum += m_hash(i, j) * m_hash(i, j);
            float sqrt_sum = std::sqrt(sqr_sum);
            for(unsigned int j=0; j<m_hash.size2(); j++)
                    m_hash(i, j) /= sqrt_sum;
        }
    }

    void LSH_DFE::set_cell_width(float cell){
        m_cell_width = cell;
    }

    void LSH_DFE::rehash_data_projection(){        
        // locality sensitive hashing using CPU
        // use int8_t to represent the result of every sub hash function
        // 128 / 8 = 16, which should be equal to DOUT
        int8_t temp[DOUT];
        std::vector<float> mult(DOUT);
        for(unsigned int i=0; i<in_d.size1(); i++){        
            for(unsigned int j=0; j<DOUT; j++){
                float data = 0.0f;
                for(unsigned int k=0; k<in_d.size2(); k++)
                    data += in_d(i, k) * m_hash(j, k);
                mult[j] = data;
            }

            for(unsigned int j=0; j<DOUT; j++)
                temp[j] = (int8_t)(mult[j]/ m_cell_width);
            memcpy(&m_new_grid_cpu[i], temp, sizeof(DimType));
        }
    }

    void LSH_DFE::init_data_structure(){
        m_new_grid_cpu.resize(in_d.size1());

        hash_set_dimensions();
    }
    
    void LSH_DFE::lsh_main_function(){
        float begin;
        cout<<"----------test performance of LSH----------"<<endl;
        // continuesly do locality sensitive hashing for num_iter times to increase recall rate
        int num_iter = 20;
        begin = get_clock();
        for(int i=0; i<num_iter; i++){
            //hash_generate();
            rehash_data_projection();
        }
        cout<<get_clock() - begin<<endl; 
        cout<<"----------end test performance of LSH----------"<<endl;   
    }

    void LSH_DFE::fit(){
        srand(0);
        
        init_data_structure();
        
        lsh_main_function();

    }

    void LSH_DFE::test(){
        // currently do nothing
        return;
    }
}
