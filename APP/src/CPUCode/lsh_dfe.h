#ifndef __LSH_DFE_H__
#define __LSH_DFE_H__

#include "LSH.h"
#include "MaxSLiCInterface.h"

#include <vector>
#include <boost/numeric/ublas/matrix.hpp>

using std::cout;
using std::endl;
using namespace boost::numeric;

namespace DFECode{
    class LSH_DFE{
    public:
        typedef ublas::matrix<float> InputData;
        typedef ublas::matrix<float> Functions;

        // DimType is the output hash result in high dimension(8 dimension)
        // use int128 to represent the hash result, 
        // should be enough for most cases
        typedef std::pair<int64_t, int64_t> DimType;
        typedef std::vector<DimType> NewGrid;
        
        template<typename T>
        class PairHash{
        public:
            size_t operator()(const std::pair<T, T>& p) const{
                return std::hash<T>()(p.first) ^ (std::hash<T>()(p.second));
            }
        };


        LSH_DFE();
        ~LSH_DFE();

        void fit();
        void test();

        // set the new cell width in higher dimension
        void set_cell_width(float cell_width);

        // load and release of max file
        // use advanced static SLiC interface
        void prepare();
        void release();

        // use this function to get time
        static float get_clock();

        // use this function to read in data
        void read_input_data( size_t features_num, size_t elements_num, std::string filename);
        void generate_input_data(size_t features_num, size_t elements_num);

    protected:
        // the handle for max file/ max engine, and its related functions
        max_file_t* mf;
        max_engine_t* me;
        float* input_dfe;

        // input data
        InputData in_d;

        // hash functions used by the LSH
        Functions m_hash;

        // the width of the cell in the new space
        float m_cell_width;

        // after each locality sensitive hashing, the hashing result is in these structure
        NewGrid m_new_grid_cpu;
        NewGrid m_new_grid_dfe;

        // the output dimension of LSH
        static const unsigned int DOUT;
        static const float formalize_max;

        bool check_parameters();
        bool prepare_max_file();
        void release_max_file();
        void set_mapped_rom(LSH_actions_t* actions);
        void load_to_LMEM();
        
        // two functions to generate the hash functions
        void hash_set_dimensions();
        void hash_generate();

        // use locality sensitive hashing to rehash the data, and assign them to new grids
        // this is the Dataflow Engine version of hash projection
        void rehash_data_projection_dfe();
        // this is the CPU version of hash projection
        void rehash_data_projection();        

        // init the data structures needed for this hash and merge
        void init_data_structure();
        void compare_hash_result();
        void lsh_main_function();
        
    };

}

#endif
