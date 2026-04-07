#include <iostream>
#include "THDYN_SUPPORT.h"
using namespace std;

int main() {
    // modificare solo questa struct
    Grain grain    = {
        11,          // numero di punte N   [ punte ] 
        2,      // diametro iniz. grano [ m ]
        0.3622,       // h_frac               [ - ]
        3.2          // D_ext                [ m ]
    };
    //==============================
    vector<Point> vert = star_port_builder(grain.D, grain.N, grain.h_fr);
    vert = swap_rf_sys(vert, P2C);
    ofstream mesh_loader;

    string filename = "mesh_generator\\" + return_grain_data_DIR(grain)+".txt";
    cout << filename << endl;
    mesh_loader.open(filename, ios::out | ios::trunc);
    for (auto& i : vert)
        mesh_loader << i.x << "," << i.y << "," << grain.D_ext <<endl;
    mesh_loader.close();
    return 0;
}