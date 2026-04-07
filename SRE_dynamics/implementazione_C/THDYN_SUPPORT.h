#ifndef THDYN_SUPPORT_H
#define THDYN_SUPPORT_H
#include <iostream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include "SOLID_PROP2.h"
using namespace std;
using namespace chrono;

struct Propellant {
    // qua sono salvati i parametri costanti
    float T_ch;         // proprietà del propellente anche se il nome è forviante
    float a, n;         // parametri di Vieil
    float gamma, gammaS;// gamma e gamma congelata
    float R;            // costante dei gas
    float rho_solid;    // densità propellente SOLIDO
    float u_exit;       // velocità caratteristica in uscita
    float sigma_T;      // sensibilità termica
    float T_ref;        // temperatura di riferimento per la sensibilità termica (in questo caso 288, non ha senso cambiarla)
    float T_amb;        // temperatura ambiente
};
struct Booster {
    // qua sono salvati i parametri costanti del booster
    float L;
    
    float rapp_aree;
    float At; // area di gola
    float P_ext; // pressione esterna, diciamo che sia una prop del booster perché è "dove" vola il booster
};
struct Grain {
    int N;
    float D; // diametro iniziale
    float h_fr;
    float D_ext; // nome forviante, è il diametro massimo INTERNO alle pareti del booster ma ESTERNO al grano
};

// la parte un po' più delicata, mandare la request e 
// ricevere la risposta da MATLAB, questa è la parte 
// che succhia via un botto di tempo
#define UPDATEPARAMS // serve a definire se i parametri costanti vanno corretti oppure no
string send_request(float P_MPa, Propellant* target) {
    string log = "\n >: Richiesta di update\n";
    auto tot_ex_start = high_resolution_clock::now();
    float P_bar = P_MPa * 10;
    ofstream write("CEACPP_SUPPORT\\request.txt", ios::out | ios::trunc);
    write << P_bar;
    write.close();
    int request = system("CEACPP_SUPPORT\\run_mat.bat > CEACPP_SUPPORT\\run_log.txt 2>&1"); // spiaze, se usate questo codice dovete anche cambiare il file .bat (è solo una riga per vostra fortuna)
    int wait_ms = 0;
    while (!filesystem::exists("CEACPP_SUPPORT\\answer.txt") && wait_ms < 3000) { // max 3 sec
        this_thread::sleep_for(chrono::milliseconds(100));
        wait_ms += 100;
    }
    cout << flush << endl;
    auto tot_ex_end = high_resolution_clock::now();
    auto tot_duration = duration_cast<milliseconds>(tot_ex_end - tot_ex_start);
    if (request != 0) {
        log += " >: La richiesta di update ha fallito dopo "+to_string(tot_duration.count() / 1000.0f) + "s\n";
        return log;
    }
    log += " >: Richiesta andata a buon fine dopo "+to_string(tot_duration.count() / 1000.0f) + "s\n";
    string line;
    vector<float> data;
    ifstream read("CEACPP_SUPPORT\\answer.txt");
    if (!read) {
        log += " >: Impossibile aprire il file di lettura... Richiesta abortita\n";
        return log;
    }
    while (getline(read, line)) {
        // niente try-catch, qua si muore da eroi (velocemente)
        float val = stof(line);
        data.push_back(val);
    }
    (*target).T_ch      = data.at(0);
    (*target).gamma     = data.at(1);
    (*target).gammaS    = data.at(2);
    (*target).R         = data.at(3);
    return log;
}
// roba utile per i file (per coerenza return_grain_data_DIR dovrebbe stare in SOLID_PROP2.h, però sticazzi)
string floatToStringUnderscore(float x) {
    string s = to_string(x);
    replace(s.begin(), s.end(), '.', '_');
    return s;
}
string return_grain_data_DIR(Grain grain) {
    // restituisce il nome del file con i dati del grano
    string name = "N" + to_string(grain.N); // questo è int quindi no problema
    float D = grain.D;
    if (approx_equal(D, (int)D)) name += "D"+to_string((int)D);
    else name += "D"+floatToStringUnderscore(D);
    float hf = grain.h_fr;
    if (approx_equal(hf, (int)hf)) name += "h"+to_string((int)hf);
    else name += "h"+floatToStringUnderscore(hf);
    float De = grain.D_ext;
    if (approx_equal(De, (int)De)) name += "De"+to_string((int)De);
    else name += "De"+floatToStringUnderscore(De);
    return name;
}
// qua tutte le funzioni termodinamiche 
inline float vieille(Propellant prop, float Pk_Mpa) {
    return exp(prop.sigma_T*(prop.T_amb - prop.T_ref))*(prop.a * pow(Pk_Mpa, prop.n)) / 1000.0f; // TeCnIcAmEnTe sta prendendo <math.h> da SOLID che non è troppo elegante

    // si chiama Pk perché è il propellente usato a questa iterazione, non Pkp1
    // converto vieille in m/s, per essere coerente con tutto il resto
}
float get_step(Propellant prop, float Pk_Mpa, float dt) {
    return vieille(prop, Pk_Mpa)*dt;
}
float dV_dt(float perim, Booster booster, Propellant prop, float Pk_Mpa) {
    float Ab = perim * booster.L;
    return Ab*vieille(prop, Pk_Mpa);
}
float mp_gen (float perim, Booster booster, Propellant prop, float Pk_Mpa) {
    float Ab = perim * booster.L;
    return Ab*vieille(prop, Pk_Mpa)*prop.rho_solid;
}
bool is_choking(Booster booster, Propellant prop, float Pk_Pa) {
    float Pk = Pk_Pa;
    float gamma = prop.gammaS;
    float C = pow(2.0f/( gamma + 1.0f ), gamma / ( gamma - 1.0f ));
    float lhs = booster.P_ext/Pk;
    return ( lhs <= 1.05f*C );
}
float mp_out(Booster booster, Propellant prop, float Pk_Pa) {
    float At    = booster.At;          // m²
    float gamma = prop.gammaS;          // rapporto di calori specifici
    float Tch   = prop.T_ch;           // K
    float R     = prop.R;              // J/(kg·K)
    float P_ex  = booster.P_ext;       // Pa

    if (is_choking(booster, prop, Pk_Pa)) {
        // Portata critica (choking)
        float term = powf(2.0f/(gamma+1.0f), (gamma+1.0f)/(2.0f*(gamma-1.0f)));
        return At * Pk_Pa * sqrtf(gamma / (R * Tch)) * term;
    } else {
        // Portata subsonica (no choking)
        float pressure_ratio = P_ex / Pk_Pa;
        float term1 = powf(pressure_ratio, 2.0f / gamma);
        float term2 = powf(pressure_ratio, (gamma+1.0f) / gamma);
        float under_sqrt = (2.0f * gamma) / (R * Tch * (gamma - 1.0f)) * (term1 - term2);
        return At * Pk_Pa * sqrtf(under_sqrt);
    }
}
inline float vandenkerckhove(float gamma) {
    return sqrt( gamma* pow( 2.0f/(gamma+1.0f) , (gamma+1.0f)/(gamma-1.0f) ) );
}
// inline float c_star(Propellant prop) {
//     return sqrt( prop.R*prop.T_ch ) / vandenkerckhove(prop.gamma);
// }
inline float c_star(Booster bst, Propellant prop, float Pk_Pa) {
    return Pk_Pa*bst.At / mp_out(bst, prop, Pk_Pa);
    // uso la c_star reale, non la teorica
}
float cT(Booster booster, Propellant prop, float Pk_Pa) {
    float g = prop.gamma;
    float sqr = sqrt( 2*g/(g-1.0f) * (1 - pow(booster.P_ext/Pk_Pa, (g-1.0f)/g) ) );
    return vandenkerckhove(prop.gammaS)*sqr;
}
float get_dPdt (float perim, float A, Booster booster, 
        Propellant prop, float Pk_Mpa) {
    float Pk   = Pk_Mpa*1e6f; // Pa
    float Vc   = A*booster.L;
    float dVdt = dV_dt(perim, booster, prop, Pk_Mpa);
    float mpo  = mp_out(booster, prop, Pk);
    float mpg  = mp_gen(perim, booster, prop, Pk_Mpa);
    if (mpo < 0) mpo = 0;
    if (mpg < 0) mpg = 0;
    float R    = prop.R;
    float Tch  = prop.T_ch;
    float T1 = (R*Tch/Vc*(mpg - mpo)); // in Pa/s
    float T2 = dVdt * Pk/Vc; // in Pa/s
    float dPdt =  T1 - T2; 
    return dPdt;
}
float get_data(const std::vector<float>& y_data,
                    const std::vector<float>& x_data,
                    float x)
{
    // questa tristemente l'ha fatta tutta chatGPT, è forse l'unico pezzo di codice che non saprei neanche spiegare, ero tanto stanco...
    // (chiariamo però che come tutti gli altri pezzi di codice fatti da AI, è un'ottimizzazione di una versione che ho fatto io.
    // Deve rimanere pur sempre uno strumento e non un sostitutivo)
    const int n = static_cast<int>(x_data.size());
    if (n == 0) return 0.0f;
    if (n == 1 || !std::isfinite(x)) return y_data[0];
    auto eqx = [](float a, float b) { return std::fabs(a - b) < 1e-9f; };
    // Extrapolazione a sinistra (clamp pendenza)
    if (x <= x_data[0]) {
        int j = 1;
        while (j < n && eqx(x_data[j], x_data[0])) j++; // salta duplicati
        if (j == n) return y_data[0];                   // tutti uguali → costante
        float dx = x_data[j] - x_data[0];
        if (!std::isfinite(dx) || eqx(dx, 0.0f)) return y_data[0];
        float dy = y_data[j] - y_data[0];
        float m = dy / dx;
        const float m_max = 1e3f; // pendenza massima
        m = std::max(std::min(m, m_max), -m_max);
        float val = y_data[0] + m * (x - x_data[0]);
        return std::isfinite(val) ? val : y_data[0];
    }
    // Extrapolazione a destra (piatto)
    if (x >= x_data[n - 1]) {
        return std::isfinite(y_data[n - 1]) ? y_data[n - 1] : 0.0f;
    }
    // Interpolazione interna (ricerca binaria)
    int lo = 0, hi = n - 1;
    while (hi - lo > 1) {
        int mid = (lo + hi) / 2;
        if (x_data[mid] <= x) lo = mid;
        else hi = mid;
    }
    float x1 = x_data[lo], y1 = y_data[lo];
    float x2 = x_data[hi], y2 = y_data[hi];
    if (!std::isfinite(x1) || !std::isfinite(x2) ||
        !std::isfinite(y1) || !std::isfinite(y2))
        return 0.0f;

    if (eqx(x2, x1)) return y1; // duplicati → valore piatto

    float t = (x - x1) / (x2 - x1);
    float val = y1 + t * (y2 - y1);

    // Clamp finale per sicurezza
    if (!std::isfinite(val)) val = y1;
    if (val < 0.0f) val = 0.0f;

    return val;
}
// questa è la nuova getPcurve, che restituisce i dati, raccogliendo i dati della mesh dal file MATLAB. 
// è brutto che non possa essere c++ a fare la mesh, ma ci sono troppi bug e il tempo è troppo poco per 
// capire che cosa sta accadendo...
void update_prop(const std::vector<float>& p_data,
                 const std::vector<float>& T_data,
                 const std::vector<float>& g_data,
                 const std::vector<float>& s_data,
                 const std::vector<float>& R_data,
                 const std::vector<float>& u_data,
                 float P_MPa,
                 Propellant* prop) {
    prop->T_ch   = get_data(T_data, p_data, P_MPa);
    prop->gamma  = get_data(g_data, p_data, P_MPa);
    prop->gammaS = get_data(s_data, p_data, P_MPa);
    prop->R      = get_data(R_data, p_data, P_MPa);
    prop->u_exit = get_data(u_data, p_data, P_MPa);
}
string load_grainDATA(vector<float> *p_data, vector<float> *A_data, vector<float> *s_data, string filename) {
    string log = "";
    ifstream file(filename);
    if (!file) {
        log += "Il file contenente i vertici non esiste o è irraggiungibile\n";
        cout << "error! Refere to log" << endl;
        return log;
    }
    string line;
    float v1, v2, v3;
    while (file >> v1) {
        // leggo perim, area, step
        (*p_data).push_back(v1);
        file    .ignore(1); // salta la virgola
        file >> v2;
        (*A_data).push_back(v2);
        file    .ignore(1); // salta la virgola
        file >> v3;
        (*s_data).push_back(v3);
    }
    file.close();
    return log;
}
string load_propDATA(vector<float> *p_data, vector<float> *T_data, vector<float> *g_data,
                     vector<float> *s_data, vector<float> *R_data, vector<float> *u_data,
                     string filename) {
    string log = "";
    ifstream file(filename);
    if (!file) {
        log += "Il file contenente i vertici non esiste o è irraggiungibile\n";
        cout << "error! Refer to log" << endl;
        return log;
    }
    float v1, v2, v3, v4, v5, v6;
    while (file >> v1) {
        p_data->push_back(v1);
        file.ignore(1); // salta la virgola
        file >> v2;
        T_data->push_back(v2);
        file.ignore(1);
        file >> v3;
        g_data->push_back(v3);
        file.ignore(1);
        file >> v4;
        s_data->push_back(v4);
        file.ignore(1);
        file >> v5;
        R_data->push_back(v5);
        file.ignore(1);
        file >> v6;
        u_data->push_back(v6);
    }
    file.close();
    return log;
}
string getPcurve(vector<float>* Pvec, // vecchia versione, tecnicamente funziona bene però non è stata aggiornata fino alla fine
    vector<float>* perim_buffer, vector<float>* step_buffer,
    vector<float>* timev, vector<float>* area_buffer,
    vector<float>* thrust_buffer, vector<float>* dPdt_buffer,
    vector<float>* cs_buffer, vector<float>* ct_buffer,
    Booster booster, Propellant prop, Grain grain,
    float startingP, float n_p, string filename, string prop_filename, float eps_ = 0.01) {
    // margin è 2 MPa aka 20 bar
    string log = "\n >: PCurve() chiamata\n";
    // load dei dati
    vector<float> p_data_G;
    vector<float> A_data_G;
    vector<float> s_data_G;
    vector<float> p_data_P;
    vector<float> T_data_P;
    vector<float> g_data_P;
    vector<float> s_data_P; // in questo caso è gs data
    vector<float> R_data_P;
    vector<float> u_data_P;
    
    log += load_grainDATA(&p_data_G, &A_data_G, &s_data_G, filename);
    #ifdef UPDATEPARAMS
    log += load_propDATA(&p_data_P, &T_data_P, &g_data_P, &s_data_P, &R_data_P, &u_data_P, prop_filename);
    #endif
    log += " >: dati caricati con successo\n";
    cout << "dati caricati con successo" << endl;
    // =============
    int   count_lim = 1000000, count = 0;
    float Pk = startingP;
    float perim;
    float dt = 0.0001; // il primo passo è preso comicamente piccolo
    float cum_time = dt;
    float cum_step = 0;
    do {
        // aggiorno i parametri di propellente
        #ifdef UPDATEPARAMS
        update_prop(p_data_P, T_data_P, g_data_P, s_data_P, R_data_P, u_data_P, Pk, &prop);
        #endif
        // aggiorno step e tempo
        (*timev)    .push_back(cum_time);
        float step  = get_step(prop, Pk, dt);
        cum_step    += step;
        // aggiorno perimetro e area (quindi Ab e Vch)
        perim       = max( get_data(p_data_G, s_data_G, cum_step), 0.0f );
        float A     = max(get_data(A_data_G, s_data_G, cum_step), 1e-9f);
        // aggiorno il dPdt
        float dPdt  = get_dPdt(perim, A, booster, prop, Pk);
        dPdt /= 1e6; // riportiamo in Mpa/s
        // calcolo della thrust
        float Pk_Pa     = Pk*1e6f;
        float c_s       = c_star(booster, prop, Pk_Pa);
        float c_T       = cT(booster, prop, Pk_Pa);
        float T         = mp_out(booster, prop, Pk_Pa) * c_T*c_s;
        // debug e controlli di uscita
        if (count % 100 == 0) {
            log += " >: debug -> cstar = "+to_string(c_s) + "\n";
            log += " >: debug -> cT = "+to_string(c_T) + "\n";
        }
        if (!isfinite(perim)) {
            log += " >: Abort! Perimeter is not define \n";
            break;
        }
        if (!isfinite(A)) {
            log += " >: Abort! Area is not define \n";
            break;
        }
        if (!isfinite(dPdt)) {
            log += " >: Abort! dPdt is not define \n";
            break;
        }
        float T_thresh = 100;
        if( T < T_thresh || ( Pk*1e6 < booster.P_ext && cum_time > 0.1f)) {
            log += "Simulazione terminata\n";
            break;
        }
        // aggiornamendo di Pk
        Pk = Pk + dt*dPdt;
        if (Pk < 0) Pk = 0.01f; // il che è assurdo, ma almeno permette di non andare totalmente nell'impossibile
        // aggiornamento dei buffer
        (*perim_buffer) .push_back(perim);
        (*step_buffer)  .push_back(step);
        (*area_buffer)  .push_back(A);
        (*dPdt_buffer)  .push_back(dPdt);
        (*thrust_buffer).push_back(T);
        (*Pvec)         .push_back(Pk);
        (*cs_buffer)    .push_back(c_s);
        (*ct_buffer)    .push_back(c_T);
        // smussa la variazione di dt per stabilità (questo lo ha fatto chatGPT, secondo me è una stronzata però funziona per dt_max piccoli)
        float target = eps_ * Pk / max(fabs(dPdt), 1e-9f);
        dt = 0.9f*dt + 0.1f*target;
        float dt_min = 1e-6f;
        float dt_max = 0.1f;
        dt = min(max(dt, dt_min), dt_max);
        cum_time += dt;
        count ++;
        if (count % 100 == 0) {
            log += " >: iterazione "+ to_string(count) + "\n";
            log += " >: dPdt :  "+ to_string(dPdt) + "\n";
            cout << "iterazione: " << count << endl;
            cout << "dPdt : " << dPdt << endl;
        }
    } while (perim > 0 && count < count_lim);
    return log;
}
float get_QS_dPdt (float perim, float A, Booster booster, 
        Propellant prop, float Pk_Mpa) {
    float Pk   = Pk_Mpa * 1e6f; // converto in Pa
    float Vc   = A*booster.L;
    float Ab   = perim*booster.L;
    float G    = vandenkerckhove(prop.gammaS);
    float cs   = c_star(booster, prop, Pk);
    float rhoc = Pk / prop.R / prop.T_ch; // anche il CEA calcola la rho di camera con la legge dei gas perfetti lol
    float At   = booster.At;
    float dPdt;
    float T1   = Vc / pow((G*cs), 2 );
    float T2   = (prop.rho_solid - rhoc)*Ab*vieille(prop, Pk_Mpa);
    float T3   = Pk * At / cs ;
    dPdt       = (T2 - T3) / T1; // torno in MPa / s
    return dPdt;
}
float get_dPdt_UPDATE(float perim, float A, Booster booster, 
        Propellant prop, float Pk_Mpa, string* log, float lastdPdt, float cumtime, float margin = 0.1) {
    float dPdt_test = get_dPdt(perim, A, booster, prop, Pk_Mpa); // questo margin è abbastanza sperimentale, però sembra funzionare
    if (fabs(dPdt_test) < margin) {
        // if (fabs(lastdPdt) > margin) {
        //     (*log) += " >: change of regime detected at "+to_string(cumtime)+"s\n";
        // }
        return get_QS_dPdt(perim, A, booster, prop, Pk_Mpa);
    }
    // if (fabs(lastdPdt) < margin) { // questo debug è scartato perché ha dei comportamenti un po' anomali
    //     (*log) += " >: change of regime detected at "+to_string(cumtime)+"s\n";
    // }
    return dPdt_test;
}
string getPcurve_UPDATE(vector<float>* Pvec,
    vector<float>* perim_buffer, vector<float>* step_buffer,
    vector<float>* timev, vector<float>* area_buffer,
    vector<float>* thrust_buffer, vector<float>* dPdt_buffer,
    vector<float>* cs_buffer, vector<float>* ct_buffer,
    Booster booster, Propellant prop, Grain grain,
    float startingP, float n_p, string filename, string prop_filename, float eps_ = 0.01) {
    string log = "\n >: PCurve() chiamata\n";
    // load dei dati
    vector<float> p_data_G;
    vector<float> A_data_G;
    vector<float> s_data_G;
    vector<float> p_data_P;
    vector<float> T_data_P;
    vector<float> g_data_P;
    vector<float> s_data_P; // in questo caso è gs data
    vector<float> R_data_P;
    vector<float> u_data_P;
    
    log += load_grainDATA(&p_data_G, &A_data_G, &s_data_G, filename);
    #ifdef UPDATEPARAMS
    log += load_propDATA(&p_data_P, &T_data_P, &g_data_P, &s_data_P, &R_data_P, &u_data_P, prop_filename);
    #endif
    log += " >: dati caricati con successo\n";
    cout << "dati caricati con successo" << endl;
    // =============
    int   count_lim = 1000000, count = 0;
    float Pk = startingP;
    float perim;
    float dt = 0.0001; // il primo passo è preso comicamente piccolo
    float cum_time = dt;
    float cum_step = 0;
    do {
        // aggiorno i parametri di propellente
        #ifdef UPDATEPARAMS
        update_prop(p_data_P, T_data_P, g_data_P, s_data_P, R_data_P, u_data_P, Pk, &prop);
        #endif
        // aggiorno step e tempo
        (*timev)    .push_back(cum_time);
        float step  = get_step(prop, Pk, dt);
        cum_step    += step;
        // aggiorno perimetro e area (quindi Ab e Vch)
        perim       = max( get_data(p_data_G, s_data_G, cum_step), 0.0f );
        float A     = max(get_data(A_data_G, s_data_G, cum_step), 1e-9f);
        // aggiorno il dPdt
        float dPdt = get_dPdt_UPDATE(perim, A, booster, prop, Pk, &log, dPdt, cum_time);
        dPdt /= 1e6; // riportiamo in Mpa/s
        // calcolo della thrust
        float Pk_Pa     = Pk*1e6f;
        float c_s       = c_star(booster, prop, Pk_Pa);
        float c_T       = cT(booster, prop, Pk_Pa);
        float T         = mp_out(booster, prop, Pk_Pa) * c_T*c_s;
        // debug e controlli di uscita
        if (count % 100 == 0) {
            log += " >: debug -> cstar = "+to_string(c_s) + "\n";
            log += " >: debug -> cT = "+to_string(c_T) + "\n";
        }
        if (!isfinite(perim)) {
            log += " >: Abort! Perimeter is not define \n";
            return log;
        }
        if (!isfinite(A)) {
            log += " >: Abort! Area is not define \n";
            return log;
        }
        if (!isfinite(dPdt)) {
            log += " >: Abort! dPdt is not define \n";
            return log;
        }
        float T_thresh = 100;
        if( T < T_thresh || ( Pk*1e6 < booster.P_ext && cum_time > 0.1f)) {
            log += "Simulazione terminata\n";
            break;
        }
        // aggiornamendo di Pk
        float prevPk = Pk;
        Pk = Pk + dt*dPdt;
        if (Pk < 0) Pk = 0.01f; // il che è assurdo, ma almeno permette di non andare totalmente nell'impossibile

        // tipicamente inutile perché parte già chokato lol
        if (!is_choking(booster, prop, prevPk*1e6) && is_choking(booster, prop, Pk*1e6)) {
            log += " >: nozzle is choking at time "+to_string(cum_time)+"s\n";
        }

        // aggiornamento dei buffer
        (*perim_buffer) .push_back(perim);
        (*step_buffer)  .push_back(step);
        (*area_buffer)  .push_back(A);
        (*dPdt_buffer)  .push_back(dPdt);
        (*thrust_buffer).push_back(T);
        (*Pvec)         .push_back(Pk);
        (*cs_buffer)    .push_back(c_s);
        (*ct_buffer)    .push_back(c_T);
        // smussa la variazione di dt per stabilità (questo lo ha fatto chatGPT, secondo me è una stronzata però funziona per dt_max piccoli)
        float target = eps_ * Pk / max(fabs(dPdt), 1e-9f);
        dt = 0.9f*dt + 0.1f*target;
        float dt_min = 1e-12f;
        float dt_max = 0.1f;
        dt = min(max(dt, dt_min), dt_max);
        cum_time += dt;
        count ++;
        if (count % 100 == 0) {
            log += " >: iterazione "+ to_string(count) + "\n";
            log += " >: dPdt :  "+ to_string(dPdt) + "\n";
            cout << "iterazione: " << count << endl;
            cout << "dPdt : " << dPdt << endl;
        }
    } while (perim > 0 && count < count_lim);
    if (perim > 0) {
        log += " >: numero massimo di iterate raggiunto : "+to_string(count)+"\n";
    } else {
        log += " >: burn finita nominalmente all'iterata : "+to_string(count)+"\n";
    }
    return log;
}




#endif



// inline float get_new_P(float perim, float A, Booster booster, 
//         Propellant prop, float Pk, float dt) {
//     // metodo di Eulero in avanti, Luca Dedé mi tirerebbe uno schiaffo, però funziona
//     return Pk + dt*get_dPdt(perim, A, booster, prop, Pk, dt);
// }

// VECCHIA VERSIONE: calcola qua la mesh, tristemente funziona male...
// string getPcurve(vector<Point> starting_mesh, 
//     vector<float>* Pvec,
//     vector<float>* perim_buffer, vector<float>* step_buffer,
//     vector<float>* timev, vector<float>* area_buffer,
//     vector<float>* thrust_buffer, vector<float>* dPdt_buffer,
//     Booster booster, Propellant prop, Grain grain,
//     float startingP, float n_p,float eps_ = 0.01, float margin = 2) {
//     // margin è 2 MPa aka 20 bar
//     string log = "\n >: PCurve() chiamata\n";
//     vector<Point> newm;
//     vector<Point> mesh = starting_mesh;
//     int count_lim = 1000000, count = 0;
//     float Pk = startingP;
//     float Pk_mean = startingP;
//     float perim;
//     float dt = 0.0001; // il primo passo è preso comicamente piccolo
//     float cum_time = dt;
//     do {
//         (*timev).push_back(cum_time);
//         float step = get_step(prop, Pk, dt);
//         float rm   = find_r_min(mesh, grain.N, grain.h_fr, grain.D, step);
//         newm = translate_mesh(mesh, step);
//         #ifdef SAFEMODE
//         newm = filter_mesh_safe(newm, rm, grain.N); // filtro la mesh traslata
//         #else 
//         newm = filter_mesh(newm, rm, grain.N); // filtro la mesh traslata
//         #endif
//         newm = generate_mesh(newm, n_p);
//         perim = calculate_perim(newm, booster.D_ext);
//         float A = calculate_area(newm, booster.D_ext);
//         float dPdt = get_dPdt(perim, A, booster, prop, Pk, dt);
//         dPdt /= 1e6; // riportiamo in Mpa/s
//         // calcolo della thrust
//         float Pk_Pa = Pk*1e6f;
//         float c_s = c_star(prop);
//         float c_T = cT(booster, prop, Pk_Pa);
//         float T   = mp_out(booster, prop, Pk_Pa) * c_T*c_s;
//         // aggiornamendo di Pk
//         Pk = Pk + dt*dPdt;
//         #ifdef UPDATEPARAMS
//         if (abs(Pk - Pk_mean) > margin) {
//             cout << " >: request sent!" << endl;
//             log += send_request(Pk, &prop);
//             Pk_mean = Pk;
//         }
//         #endif
//         if (Pk < 0) Pk = 0.1f; // il che è assurdo, ma almeno permette di non andare totalmente nell'impossibile
//         // aggiornamento dei buffer
//         (*perim_buffer) .push_back(perim);
//         (*step_buffer)  .push_back(step);
//         (*area_buffer)  .push_back(A);
//         (*dPdt_buffer)  .push_back(dPdt);
//         (*thrust_buffer).push_back(T);
//         (*Pvec).push_back(Pk);
//         dt =( !approx_equal(dPdt, 0) ) ?  eps_* Pk / fabs(Pk + dt*dPdt) : dt = eps_/100;
//         cum_time += dt;
//         mesh = newm;
//         count ++;
//         if (count % 100 == 0) {
//             log += " >: iterazione "+ to_string(count) + "\n";
//             log += " >: dPdt :  "+ to_string(dPdt) + "\n";
//             cout << "iterazione: " << count << endl;
//             cout << "dPdt : " << dPdt << endl;
//         }
//     } while (perim > 0 && count < count_lim);
//     return log;
// }


// altra vecchia versione

/*
string getPcurve(vector<float>* Pvec,
    vector<float>* perim_buffer, vector<float>* step_buffer,
    vector<float>* timev, vector<float>* area_buffer,
    vector<float>* thrust_buffer, vector<float>* dPdt_buffer,
    vector<float>* cs_buffer, vector<float>* ct_buffer,
    Booster booster, Propellant prop, Grain grain,
    float startingP, float n_p, string filename, float eps_ = 0.01, float margin = 2) {
    // margin è 2 MPa aka 20 bar
    string log = "\n >: PCurve() chiamata\n";
    // load dei dati
    vector<float> p_data;
    vector<float> A_data;
    vector<float> s_data;
    ifstream file(filename);
    if (!file) {
        log += "Il file contenente i vertici non esiste o è irraggiungibile\n";
        cout << "error! Refere to log" << endl;
        return log;
    }
    string line;
    float v1, v2, v3;
    while (file >> v1) {
        // leggo perim, area, step
        p_data.push_back(v1);
        file.ignore(1); // salta la virgola
        file >> v2;
        A_data.push_back(v2);
        file.ignore(1); // salta la virgola
        file >> v3;
        s_data.push_back(v3);
    }
    // =============
    int   count_lim = 1000000, count = 0;
    float Pk = startingP;
    float Pk_mean = startingP;
    float perim;
    float dt = 0.0001; // il primo passo è preso comicamente piccolo
    float cum_time = dt;
    float cum_step = 0;
    do {
        (*timev).push_back(cum_time);
        float step = get_step(prop, Pk, dt);
        cum_step += step;
        perim = max( get_data(p_data, s_data, cum_step), 0.0f );
        float A = max(get_data(A_data, s_data, cum_step), 1e-9f);
        float dPdt = get_dPdt(perim, A, booster, prop, Pk, dt);
        dPdt /= 1e6; // riportiamo in Mpa/s
        // calcolo della thrust
        float Pk_Pa = Pk*1e6f;
        // float c_s = c_star(prop);
        float c_s = c_star(booster, prop, Pk_Pa);
        float c_T = cT(booster, prop, Pk_Pa);
        float T   = mp_out(booster, prop, Pk_Pa) * c_T*c_s;
        if (count % 100 == 0) {
            log += " >: debug -> cstar = "+to_string(c_s) + "\n";
            log += " >: debug -> cT = "+to_string(c_T) + "\n";
        }
        if (!isfinite(perim)) {
            log += " >: Abort! Perimeter is not define \n";
            break;
        }
        if (!isfinite(A)) {
            log += " >: Abort! Area is not define \n";
            break;
        }
        if (!isfinite(dPdt)) {
            log += " >: Abort! dPdt is not define \n";
            break;
        }
        float T_thresh = 100;
        if( T < T_thresh || ( Pk*1e6 < booster.P_ext && cum_time > 0.1f)) {
            log += "Simulazione terminata\n";
            break;
        }
        // aggiornamendo di Pk
        Pk = Pk + dt*dPdt;
        #ifdef UPDATEPARAMS
        if (abs(Pk - Pk_mean) > margin) {
            cout << " >: request sent!" << endl;
            log += send_request(Pk, &prop);
            Pk_mean = Pk;
        }
        #endif
        if (Pk < 0) Pk = 0.01f; // il che è assurdo, ma almeno permette di non andare totalmente nell'impossibile
        // aggiornamento dei buffer
        (*perim_buffer) .push_back(perim);
        (*step_buffer)  .push_back(step);
        (*area_buffer)  .push_back(A);
        (*dPdt_buffer)  .push_back(dPdt);
        (*thrust_buffer).push_back(T);
        (*Pvec)         .push_back(Pk);
        (*cs_buffer)    .push_back(c_s);
        (*ct_buffer)    .push_back(c_T);
        // smussa la variazione di dt per stabilità (questo lo ha fatto chatGPT, secondo me è una stronzata però funziona per dt_max piccoli)
        float target = eps_ * Pk / max(fabs(dPdt), 1e-9f);
        dt = 0.9f*dt + 0.1f*target;
        float dt_min = 1e-6f;
        float dt_max = 0.1f;
        dt = min(max(dt, dt_min), dt_max);
        cum_time += dt;
        count ++;
        if (count % 100 == 0) {
            log += " >: iterazione "+ to_string(count) + "\n";
            log += " >: dPdt :  "+ to_string(dPdt) + "\n";
            cout << "iterazione: " << count << endl;
            cout << "dPdt : " << dPdt << endl;
        }
    } while (perim > 0 && count < count_lim);
    return log;
}

*/