#include <iostream>
#include <fstream>
#include <chrono>
#include "THDYN_SUPPORT.h"
using namespace std;

int main() {
    // ===== compilare solo questa parte ====
    Booster bst    = {
        150*1e-3,              // Lunghezza (aka altezza) del booster       [ m ]
        8,                  // rapporto fra aree del nozzle                 [ - ]
        1.767e-4,                // Area di gola                            [ m^2 ]
        0.101325*1e6         // Pressione esterna                           [Pa] NB!!!!
    };
    Propellant prp = {
        3000,   // I - Temperatura di burn      [ K ]
        4.28,   // parametro n di Vieil         [ / ]
        0.1,    // parametri a di Vieil          [ / ] (default. 0.39!)
        1.13,   // I - gamma del propellente    [ - ]
        1.15,   // I - gammaS del propellente   [ - ]
        300,    // I - costante R               [ J/Kg/K ]
        1690,   // densità SOLIDA               [ Kg/m3 ]
        790,    // I -  velocità caratt.        [ m/s ]
        0.002,  // sensibilità termica          [ 1/K ]
        288,    // Temperatura di riferimento   [ K ]
        288     // Temperatura ambiente         [ K ]
    };
    // i valori con I - non sono da compilare, tanto vengono
    // sovrascritti immediatamente dal CEA. Sono già presenti
    // solo per evitare che la prima sovrascrizione dia errori
    Grain grain    = {
        11,          // numero di punte N   [ punte ] 
        2,      // diametro iniz. grano [ m ]
        0.3622,       // h_frac               [ - ]
        50e-3,           // diametro interno del booster a vuoto   [ m ] (proprietà del booster, ma serve più qui che lì)
    };
    int n_p = 1e3; // numero di punti per mesh, (OCCHIO a non salire troppo, 1e3 è SUFFICIENTE) 
    float P_start = 0.5; //MPa!!!, pressione iniziale in camera
    string FOLDER =     "RUN_TO_SAVE\\ACCENSIONE2"; // LA CARTELLA DEVE GIà ESISTERE (usare file system può essere pericoloso per evitare di dare troppo potere a user irresponsabili hehe)
    string prop_data  = "CEA_DATA\\NH4CLO468_HTPB14_Al18.txt";
    // ======================================
    

    #ifndef MANUAL_CHOICE_FOR_MPATH // per ordine la macro si trova in SOLID_PROP2, anche se centra veramente poco con quella lib.h
    string log_str = "MESH_MODE       : automatic grain data loading\n";
    string grain_data = "mesh_generator\\" + return_grain_data_DIR(grain) + "_graphs.txt";
    cout << grain_data << endl;
    #else
    string log_str = "MESH_MODE       : manual grain data loading\n";
    string grain_data = "mesh_generator\\circular_data_graphs.txt"; // inserire qua i dati custom, se servono (non sono assolutamente presenti due easter-eggs nelle geometrie custom)
    cout << grain_data << endl;
    #endif
    // carico i dati di Prop iniziale
    #ifdef UPDATEPARAMS
    cout << prop_data  << endl;
    vector<float> p_data_P;
    vector<float> T_data_P;
    vector<float> g_data_P;
    vector<float> s_data_P;
    vector<float> R_data_P;
    vector<float> u_data_P;
    log_str += "PARAMETERS MODE : corrected parameters simulation\n";
    log_str += load_propDATA(&p_data_P, &T_data_P, &g_data_P, 
        &s_data_P, &R_data_P, &u_data_P, 
        prop_data);
    update_prop(p_data_P, T_data_P, g_data_P, 
        s_data_P, R_data_P, u_data_P, P_start, 
        &prp);
    #else
    log_str += "PARAMETERS MODE : frozen parameters simulation\n";
    #endif
    // genero le variabili di buffer (di fatto i dati salvati)
    vector<float> perim_buff;
    vector<float> step_buff;
    vector<float> time_v;
    vector<float> area_buff;
    vector<float> thrust_buff;
    vector<float> dPdt_buff;
    vector<float> Pvec;
    vector<float> cs_buff;
    vector<float> ct_buff;
    // tutto il programma di fatto è in questa funzione
    log_str += getPcurve_UPDATE(&Pvec,
    &perim_buff, &step_buff, &time_v, &area_buff,
    &thrust_buff, &dPdt_buff,
    &cs_buff, &ct_buff,
    bst, prp, grain,
    P_start, n_p, grain_data.c_str(), prop_data.c_str());
    // salvo i dati generati e il log
    ofstream serial, log;
    serial.open(FOLDER+"/simulation_data.txt", ios::out | ios::trunc);
    log   .open(FOLDER+"/log.txt", ios::out | ios::trunc); // ancora da compilare l'output
    log << log_str << endl;
    for (int i = 0; i < perim_buff.size(); i++ ){
        serial << perim_buff.at(i)  << ",";
        serial << time_v.at(i)      << ",";
        serial << Pvec.at(i)        << ",";
        serial << area_buff.at(i)   << ",";
        serial << step_buff.at(i)   << ",";
        serial << thrust_buff.at(i) << ",";
        serial << dPdt_buff.at(i)   << ",";
        serial << cs_buff.at(i)     << ",";
        serial << ct_buff.at(i)     << endl;
    }
    cout << "debug: iterazioni a convergenza = " << perim_buff.size() << endl;
    // chiudo i file e libero le memorie (che di fatto da quando uso vector<> non mi preoccupano più)
    serial.close();
    log.close();
    return 0;
}


// come mi sono sentito ad aver finito questo codice:
//https://uploads.dailydot.com/2024/09/rubbing-hands-emoji.jpg?q=65&auto=format&w=1600&ar=2:1&fit=crop


// old v
    // // prima cosa, aggiorno i parametri con la pressione iniziale
    // #ifdef UPDATEPARAMS
    // string log_str = send_request(P_start, &prp);
    // #else
    // string log_str = "NO UPDATE SIM \n";
    // #endif
