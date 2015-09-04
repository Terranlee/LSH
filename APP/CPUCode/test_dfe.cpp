#include <cstdlib>


// test lsh on dfe only when you have maxeler environment
#ifdef DESIGN_NAME
    #include "lsh_dfe.h"
	using namespace DFECode;
    void test_dfe(){

        LSH_DFE lsh; 
        lsh.generate_input_data(2, 1600);
		lsh.set_cell_width(5000);

        cout<<"start execution of locality sensitive hashing"<<endl;
        float begin = LSH_DFE::get_clock();
        lsh.fit();
        float end = LSH_DFE::get_clock();
        cout<<"total time is : "<<end - begin<<endl;
	}
#endif


int main(int argc, char** argv)
{
    test_dfe();

    return 0;
}
