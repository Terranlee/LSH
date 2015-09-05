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
        delete[] input_dfe;
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

    void LSH_DFE::release_max_file(){
        max_unload(me);
    }

    // check the parameters from CPU and DFE are the same
    // otherwise it will not get correct result
    bool LSH_DFE::check_parameters(){
        int dfe_dout = max_get_constant_uint64t(mf, "dout");
        int dfe_din = max_get_constant_uint64t(mf, "din");

        if(dfe_dout != DOUT || dfe_din != in_d.size2()){
            cout<<"------------------------------"<<endl;
            cout<<"DFE configuration failed."<<endl;
            cout<<"Parameters are as followed:"<<endl;
            cout<<"dfe_dout : "<<dfe_dout<<",   dout : "<<DOUT<<endl;
            cout<<"dfe_din : "<<dfe_din<<",   din : "<<in_d.size2()<<endl;
            cout<<"------------------------------"<<endl;
            return false;
        }
        return true;
    }

    bool LSH_DFE::prepare_max_file(){
        cout<<"----------loading DFE----------"<<endl;
        mf = LSH_init();
        bool check = check_parameters();
        if(!check)
            return false;
        me = max_load(mf, "*");
        cout<<"----------loading DFE finished----------"<<endl;
        return true;
    }

    // two functions related to the load and release of the max file
    void LSH_DFE::prepare(){
        bool check = prepare_max_file();
        if(!check)
            exit(1);
    }

    void LSH_DFE::release(){
        release_max_file();
    }

    void LSH_DFE::set_mapped_rom(LSH_actions_t* actions){
        int DIN = in_d.size2();
        float* hashFunction = (float*)&(actions->param_hashFunction0000);
        for(int i=0; i<DOUT; i++){
            for(int j=0; j<DIN; j++)
                hashFunction[i*DIN+j] = m_hash(i, j);
        }
    }

    void LSH_DFE::set_cell_width(float cell){
        m_cell_width = cell;
    }

    /*
    // add this if you want to use LMEM
    void LSH_DFE::load_to_LMEM(){
        LSH_writeLMEM_actions_t actions;
        actions.param_N = in_d.size1();
        actions.instream_input_cpu1 = input_dfe;
        LSH_writeLMEM_run(me, &actions);
        cout<<"finish writing to LMEM"<<endl;
    }
    */

    void LSH_DFE::rehash_data_projection_dfe(){
        // locality sensitive hashing using DFE
        LSH_actions_t actions;
        
        actions.param_N = in_d.size1();
        actions.param_cellWidth = m_cell_width;

        actions.outstream_output_cpu = (int64_t*)m_new_grid_dfe.data();
        actions.instream_input_cpu = input_dfe;

        // add these if you want to use LMEM
        //actions.lmem_address_cpu2LMEM = 0;
        //actions.lmem_arr_size_cpu2LMEM = in_d.size1() * in_d.size2();

        set_mapped_rom(&actions);

        LSH_run(me, &actions);
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
        input_dfe = new float[in_d.size1() * in_d.size2()];
        int index = 0;
        for(unsigned int i=0; i<in_d.size1(); i++)
            for(unsigned int j=0; j<in_d.size2(); j++)
                input_dfe[index++] = in_d(i, j);

        m_new_grid_cpu.resize(in_d.size1());
        m_new_grid_dfe.resize(in_d.size1());

        hash_set_dimensions();
    }
    
    void LSH_DFE::compare_hash_result(){
        assert(m_new_grid_dfe.size() == m_new_grid_cpu.size());
        int counter = 0;
        for(unsigned int i=0; i<m_new_grid_dfe.size(); i++){
            if((m_new_grid_dfe[i].first == m_new_grid_cpu[i].first) && (m_new_grid_dfe[i].second == m_new_grid_cpu[i].second))
                counter++;
        }
        cout<<counter<<" same over "<<m_new_grid_cpu.size()<<endl;
        if(counter != m_new_grid_cpu.size()){
            cout<<"result is not the same, write result to answer_cpu/dfe"<<endl;
			std::ofstream fout0("answer_cpu");
            for(unsigned int i=0; i<m_new_grid_cpu.size(); i++)
                fout0<<m_new_grid_cpu[i].first<<" "<<m_new_grid_cpu[i].second<<endl;
            fout0.close();
			std::ofstream fout1("answer_dfe");
            for(unsigned int i=0; i<m_new_grid_dfe.size(); i++)
                fout1<<m_new_grid_dfe[i].first<<" "<<m_new_grid_dfe[i].second<<endl;
            fout1.close();
        }
    }

    void LSH_DFE::lsh_main_function(){
        // add this if you want to use LMEM
        //load_to_LMEM();

        cout<<"----------check accuracy of LSH using dfe----------"<<endl;
        // compare the result of LSH using CPU and DFE
        hash_generate();
        rehash_data_projection();
        rehash_data_projection_dfe();
        compare_hash_result();
        cout<<"----------end of check accuracy----------"<<endl;

        cout<<"----------test performance of LSH using dfe----------"<<endl;
        // continuesly do locality sensitive hashing for num_iter times to increase recall rate
        int num_iter = 20;
        
        float begin = get_clock();
        for(int i=0; i<num_iter; i++){
            hash_generate();
            rehash_data_projection_dfe();
        }
        cout<<get_clock() - begin<<endl;

        begin = get_clock();
        for(int i=0; i<num_iter; i++){
            //hash_generate();
            rehash_data_projection();
        }
        cout<<get_clock() - begin<<endl; 
        cout<<"----------end test performance of LSH using dfe----------"<<endl;   
    }

    void LSH_DFE::fit(){
        srand(0);
        
        init_data_structure();
        
        prepare();
        lsh_main_function();
        release();

    }

    void LSH_DFE::test(){
        // currently do nothing
        return;
    }
}
