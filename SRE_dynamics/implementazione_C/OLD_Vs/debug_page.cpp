#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include "THDYN_SUPPORT.h"
using namespace std;
using namespace chrono; // a sua volta std::

// funzioni utili solo nel main
void print_to_file(vector<Point> data, const string filename);

//  !!!
bool condition(int N, float h_fr, float D) {
    //  return ( (N == 3 && approx_equal(h_fr, 0.49) ) || 
    //     (N == 40 && approx_equal(h_fr, 0.2) ) ); // genera solo le mesh (N=3, h_fr = 0.49) e (N=40, h_fr=0.2)
    // return (N == 10); // genera tutte le mesh con N == 10
    return true; // genera tutte le mesh
}

// NB!!! se nella cartella ci sono dati passati, verranno sovrascritti!!!
int main() {
    // =================== modificare solo questa sezione e condition()
    float endD = 3.2; // diametro case esterno
    vector<int>  Nv = {20}; // vettore di N
    vector<float> h_fv = {0.2124}; // vettore di h_fr
    vector<float> D_v  = {1.9025}; // vettore di diametri iniziali
    string FOLDER = "MESH_TEST"; // cartella in cui vengono salvati i file
    float step = 0.01;
    int n_p = Nv[0]*100;
    // =================== ------------------------------
    
    ofstream serial1, log;
    int file_index = 1;
    int n_iter = Nv.size()*h_fv.size()*D_v.size();
    float endR = endD/2; 
    log.open(FOLDER+"/log.txt", ios::out | ios::trunc);
    #ifdef SAFEMODE
    cout << "SAFE MODE ATTIVATA" << endl;
    log  << "SAFE MODE ATTIVATA" << endl;
    #endif
    // la NASA dice che oltre tre indendazioni bisognerebbe fare una
    // sottofunzione Questo è quello che penso di ciò:++
    // https://media.istockphoto.com/id/161351449/vector/middle-finger-emoticon.jpg?s=612x612&w=0&k=20&c=H2RSyCnTE92TmPh_A-nGCd9exS1PBh5Ag9dY-57ZlEc=
    auto tot_ex_start = high_resolution_clock::now();
    for(int k = 0; k < D_v.size(); k++) {
        float D = D_v.at(k);
        for (int i = 0; i < Nv.size(); i++) {
            int N = Nv.at(i);
            for(int j = 0; j < h_fv.size(); j++) {
                auto start_time = high_resolution_clock::now();
                // scritte a terminale (copiato a log)
                cout << "=== ITERAZIONE "<< file_index << " / " << n_iter <<" === " << endl;
                cout << " N    = " << N << endl;
                log  << "=== ITERAZIONE "<< file_index << " / " << n_iter <<" === " << endl;
                log  << " N    = " << N << endl;
                float h_fr = h_fv.at(j);
                cout << " h_   = " << h_fr << endl;
                cout << " step = " << step << endl;
                log  << " h_   = " << h_fr << endl;
                log  << " step = " << step << endl;
                serial1.open(FOLDER+"/graph_data"+to_string(file_index)+".txt", ios::out | ios::trunc);
                file_index ++;
                serial1 << N << "," << h_fr << "," << step << "," << D <<",";
                vector<float> pv;
                if ( condition(N, h_fr, D) ) {
                    cout << " flag! Richiesta di meshing" << endl;
                    vector<Point> buffer;
                    pv = getBurningCurve(N, D, h_fr, step, n_p, endD, &buffer);
                    // buffer = swap_rf_sys(buffer, C2P);
                    print_to_file(buffer, FOLDER+"/mesh_data"+to_string(file_index -1)+".txt");
                } else {
                    pv = getBurningCurve(N, D, h_fr, step, n_p, endD);
                }
                float maxP = -FLT_MAX;
                for(auto& i : pv) {
                    if (i > maxP) maxP = i;
                    serial1 << i << ",";
                }
                if (maxP >= 2*( 2*endR*pi )) {
                    // tipicamente se il valore è assurdamente grande, non è arrivato a convergenza
                    cout << "warning! Possibile mancata convergenza!" << endl;
                    log  << "warning! Possibile mancata convergenza!" << endl;
                }
                serial1.close();
                auto stop_time = high_resolution_clock::now();
                auto exe_time  = duration_cast<milliseconds>(stop_time - start_time);
                cout << "file numero "<< (file_index - 1) << " creato!" << endl;
                cout << "execution time: " << exe_time.count()/1000.0f << "s" << endl << endl;
                log  << "file numero "<< (file_index - 1) << " creato!" << endl;
                log  << "execution time: " << exe_time.count()/1000.0f << "s" << endl << endl;
            }
        }
    }
    auto tot_ex_end = high_resolution_clock::now();
    auto tot_duration = duration_cast<milliseconds>(tot_ex_end - tot_ex_start);
    cout << "total execution time: " << tot_duration.count()/1000.0f << "s" << endl;
    log  << "total execution time: " << tot_duration.count()/1000.0f << "s" << endl;
    return 0;
}

void print_to_file(vector<Point> data, const string filename) {
    ofstream file;
    file.open(filename, ios::out | ios::trunc);
    for( auto& d : data ) {
        file << d.x << "," << d.y << endl;
    }
    file.close();
}

// old versions (bruttine e malmesse, c'è pure un rimasuglio della vecchia versione 
// in cui invece che vector abbiamo costruito la libreria usando float*x, *y)
// non sono stati due bei giorni di lavoro...
/*
vector<int> Nv = {18};
    float D = 1;
    vector<float> h_fv = {0.35};
    float step = 0.01;
    int n_p = 1000;
    int file_index = 1;
    // int starting_time = time_point_cast<seconds>(system_clock::now());
    for (int i = 0; i < Nv.size(); i++) {
        int N = Nv.at(i);
        for(int j = 0; j < h_fv.size(); j++) {
            float h_fr = h_fv.at(j);
            serial1.open("data_transfer/graph_data"+to_string(file_index)+".txt", ios::out | ios::trunc);
            serial2.open("data_transfer/mesh_data" +to_string(file_index)+".txt", ios::out | ios::trunc);
            file_index ++;
            serial1 << N << "," << h_fr << "," << step << "," << D <<",";
            vector<float> pv = getBurningCurve(N, D, h_fr, step, n_p);
            for(auto& i : pv) {
                serial1 << i << ",";
            }
            serial1.close();
            cout << "file numero "<< (file_index - 1) << " creato!" << endl;
        }
    }
    // auto finish_time = system_clock::now();
    // cout << "starting time : " << starting_time << endl;
    return 0;
*/
// secondo esperimento un po' meh
/*
int main() {
    float *starting_theta, *starting_rho;
    int L = star_port_builder(D, N, h_fr, &starting_theta, &starting_rho);
    float *bvx, *bvy;
    swap_rf_sys(starting_rho, starting_theta, &bvx, &bvy, L, P2C);
    float *vx = bvx, *vy = bvy;
    // L = chaikin_iterative(bvx, bvy, L, 3, 0, &vx, &vy);
    L = chaikin(bvx, bvy, &vx, &vy, L);
    serial.open("data_transfer/starting_vert.txt");
    for(int i = 0; i < L; i++)
        serial << vx[i] << "," << vy[i] << endl;
    serial.close();
    delete[] starting_rho, starting_theta, bvx, bvy;
    float step = 0.05; // step massimo accettabile in queste condizioni
    int n_steps = 8;
    float *meshx, *meshy, *nextx, *nexty;
    string filename;
    for(int i = 0; i < n_steps; i++) {
        int Lm = generate_mesh(vx, vy, &meshx, &meshy, L, 1e4);
        int L_nm = filter_mesh(meshx, meshy, step, &nextx, &nexty, Lm);
        int L_vx = find_vertex(nextx, nexty, &vx, &vy, L_nm);
        L = L_vx;
        filename = "data_transfer/mesh"+to_string(i)+".txt";
        serial.open(filename, ios::out | ios::trunc);

        serial << "vx,vy" << endl;
        for(int i = 0; i < L_nm; i++)
            serial << nextx[i] << "," << nexty[i] << endl;
        serial.close();
        cout << " === iteration " << (i+1) << endl;
        cout << "debug -> Lvx  = " << L_vx << endl;
    }
    delete[] vx, vy;
    return 0;
}
*/
/*
serial.open("transfer_buffer.txt", ios::out | ios::trunc);
    serial << "theta, peak, theta_p, peak_p" << endl;
    for(int i = 0; i < L; i++) {
        serial << theta[i] << "," << peak[i] << "," << y[i] << "," << x[i] << endl;
        cout   << theta[i] << "," << peak[i] << ","  << y[i] << "," << x[i] << endl;
    }
    serial2.open("mesh_buffer.txt", ios::out | ios::trunc);
    
    serial2 << "meshx, meshy, nextx, nexty" << endl;
    for(int i = 0; i < Lm; i++) {
        serial2 << meshx[i] << "," << meshy[i] 
        << "," << nextx[i]<< "," <<nexty[i] << endl;
        cout << meshx[i] << "," << meshy[i]
        << "," << nextx[i]<< "," <<nexty[i] << endl;
    }
*/
// primo esperimento
/*
int main() {
    cout << h << endl;
    float *theta, *peak;
    int L = star_port_builder(D, N, h_fr, &theta, &peak);
    float *y, *x;
    swap_rf_sys(peak, theta, &x, &y, L, P2C);
    floint L = star_port_builder(D, N, h_fr, &theta, &peak);at* meshx, *meshy, *nextx, *nexty, *vx, *vy;
    int Lm = generate_mesh(x, y, &meshx, &meshy, L, 1e3);
    int L_nm = filter_mesh(meshx, meshy, 0.01, &nextx, &nexty, Lm);
    int L_vx = find_vertex(nextx, nexty, &vx, &vy, L_nm);

    // qua simulo la prossima iterazione
    float *nm2x, *nm2y;
    int L_next = generate_mesh(vx, vy, &nm2x, &nm2y, L_vx, 1e3);

    // comunicazione con MATLAB
    serial1.open("data_transfer/current_mesh.txt", ios::out | ios::trunc);
    serial2.open("data_transfer/next_mesh.txt", ios::out | ios::trunc);
    serial3.open("data_transfer/next_vert.txt", ios::out | ios::trunc);
    serial4.open("data_transfer/next_mesh2.txt", ios::out | ios::trunc);
    serial1 << "meshx, meshy" << endl;
    for(int i = 0; i < Lm; i++) {
        serial1 << meshx[i] << "," << meshy[i] << endl;
    }
    serial2 << "nextx, nexty" << endl;
    for(int i = 0; i < L_nm; i++) {
        serial2 << nextx[i] << "," << nexty[i] << endl;
    }
    serial3 << "vx, vy" << endl;
    for(int i = 0; i < L_vx; i++) {
        serial3 << vx[i] << "," << vy[i] << endl;
    }
    serial2 << "nextmx, nextmy" << endl;
    for(int i = 0; i < L_next; i++) {
        serial4 << nm2x[i] << "," << nm2y[i] << endl;
    }
    cout << "debug >: Lm     = " << Lm << endl;
    cout << "debug >: L_nm   = " << L_nm << endl; 
    cout << "debug >: L_vx   = " << L_vx << endl;
    cout << "debug >: L_nm2  = " << L_next << endl;
    cout << "debug >: perim  = " << calculate_perim(meshx, meshy, Lm) << endl;
    delete[] y, theta;
    delete[] x, peak;
    delete[] meshx, meshy, nextx, nexty, vx, vy;
    delete[] nm2x, nm2y;
    return 0;
}
*/
// REMAKE (qua ho capito che usare i puntatori forse non era così una grande idea...)
// primo esperimento con la nuova libreria, grande successo!!!
/*
int main() {
    debug.open("data_transfer/debug_ch0.txt", ios::out | ios::trunc);
    vector<Point> start = star_port_builder(D, N, h_fr);
    start = swap_rf_sys(start, P2C);
    vector <Point> mesh  = generate_mesh(start, n_points_per_it);
    mesh = remove_duplicates(mesh);
    float r_min = find_r_min(mesh, N, h_fr, D, step);
    vector <Point> newm = translate_mesh(mesh, step);
    newm = filter_mesh(newm, r_min, N);
    cout << "debug: rmin = " << r_min << endl;
    float theta = angoloFraStelle(N, D, h_fr) / 2.0f;
    cout << "debug: theta = " << theta*180/pi << endl;
    newm = generate_mesh(newm, n_points_per_it);
    for(auto& i : start) {
        debug << i.tostring() << endl;
    }
    debug.close();
    debug.open("data_transfer/debug_ch1.txt", ios::out | ios::trunc);
    for(auto& i : mesh) {
        debug << i.tostring() << endl;
    }
    debug.close();
    debug.open("data_transfer/debug_ch2.txt", ios::out | ios::trunc);
    for(auto& i : newm) {
        debug << i.tostring() << endl;
    }
    debug.close();
    return 0;
}
*/
// secondo esperimento con nuovo ordinamento (GRAN SUCCESSO!!!)
/*
vector<Point> start = star_port_builder(D, N, h_fr);
    start = swap_rf_sys(start, P2C);
    vector <Point> mesh  = generate_mesh(start, n_points_per_it);
    mesh = remove_duplicates(mesh);
    serial1.open("data_transfer/graph_data.txt", ios::out | ios::trunc);
    serial1 << N << endl << h_fr << endl << step << endl << D << endl;
    float perim = calculate_perim(mesh, D);
    serial1 << perim << endl;
    int n_it = 10;
    float r_min;
    int i_max = 1000, count = 0;
    while (count < i_max && perim > 0) {
        r_min = find_r_min(mesh, N, h_fr, D, step);
        vector <Point> newm = translate_mesh(mesh, step);
        newm = filter_mesh(newm, r_min, N); // filtro la mesh traslata
        newm = generate_mesh(newm, n_points_per_it); // ripopolo la mesh
        perim = calculate_perim(newm, D);
        serial1 << perim << endl;
        mesh = newm;
    }
    if(count >= i_max) cout << "WARNING!!! Iterazioni massime raggiunte" << endl;
    
*/
/*
ofstream serial1;
float D = 1;
float h_fr = 0.2;
int N = 40;
float h = h_fr/2*D;
float step = 0.01; 

int n_points_per_it = 1e3;

using namespace std::chrono;

int main() {
    vector<Point> start = star_port_builder(D, N, h_fr);
    start = swap_rf_sys(start, P2C);
    vector <Point> mesh  = generate_mesh(start, n_points_per_it);
    mesh = remove_duplicates(mesh);
    serial1.open("data_transfer/graph_data.txt", ios::out | ios::trunc);
    serial1 << N << endl << h_fr << endl << step << endl << D << endl;
    float perim = calculate_perim(mesh, D);
    serial1 << perim << endl;
    float r_min;
    int i_max = 1000, count = 0;

    while (count < i_max && perim > 0) {
        step = 0.01*(count + 1);
        r_min = find_r_min(mesh, N, h_fr, D, step);
        vector <Point> newm = translate_mesh(mesh, step);
        // newm = filter_mesh_safe(newm, r_min, N, 2.0f/100.0f); // filtro la mesh traslata
        newm = filter_mesh(newm, r_min, N); // filtro la mesh traslata
        newm = generate_mesh(newm, n_points_per_it); // ripopolo la mesh
        perim = calculate_perim(newm, 4*D);
        for ( auto& n : newm ) {
            serial1 << n.x << "," << n.y << endl;
        }
        cout << perim << ",";
        mesh = newm;
    }
    if(count >= i_max) cout << "WARNING!!! Iterazioni massime raggiunte" << endl;
    return 0;
}
*/
// per fare dell'ordine, per ogni step di burn del grano vengono fatti
// i seguenti passi:
// 1) genero i vertici della figura (star_port_builder oppure find_vertex)
// 2) popolo la mesh, partendo dai vertici (generate_mesh)
// 3) traslo e filtro la nuova mesh (filter_mesh)
// 4) salvo il valore di perimetro
// questo sistema non porta molto lontano...
// una strategia alternativa è la seguente:
// 1) genero i vertici della figura (star_port_builder)
// 2) genero la mesh iniziale
// 3) applico chaikin alla mesh
// 4) per ogni step, traslo la mesh e la filtro
// 5) ...
// terza strategia:
// 1) genero i vertici (star_port_builder)
// 2) genero la mesh iniziale
// 3) sposto la mesh e la filtro
// 4) la ripopolo usando interpolante cubico