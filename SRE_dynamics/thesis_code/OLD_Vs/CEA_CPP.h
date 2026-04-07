#ifndef CEACPP_H
#define CEACPP_H
#include <vector>
#include <string>
#include <cstdlib>
#include <fstream>

// non è proprio una libreria standard come viene immaginata
// è più un file che racchiude una serie di metodi che devono 
// lavorare a stretto contatto con MATLAB per poter funzionare.
using namespace std;

const string matlabDIR = "C:\\Program Files\\MATLAB\\R2024b\\bin\\matlab.exe";
const string cppREQ    = "CEACPP_SUPPORT\\request.txt";
const string MATANS    = "CEACPP_SUPPORT\\answer.txt";
const string CEAPATH   = "C:/Users/Leonardo Perina/OneDrive - Politecnico di Milano/Desktop/UNIVERSITA/TERZO_ANNO/SECONDO_SEMESTRE/PROP_LAB/codice/implementazione_C/CEACPP_SUPPORT/cpp_CEA_comunicator.m";

// dentro request:
// P_MPa, c_star

vector<float> send_request(float P_MPa, float c_star) {
    ofstream write; write.open(cppREQ, ios::out | ios::trunc);
    write << P_MPa << "," << c_star << endl;
    write.close();
    const string command = "\"" + matlabDIR + "\" -batch \"run('" + CEAPATH + "')\"";
    cout << "Comando da eseguire: " << command << endl;
    int request = system("CEACPP_SUPPORT\\run_mat.bat");
    vector<float> data;
    if (request != 0) {
        data.push_back(-1); 
        cerr << "Errore in call di request" << endl;
        return data;
    }
    ifstream read(MATANS);
    if (!read) {
        data.push_back(-1);
        cerr << "Errore in fase di lettura" << endl;
        return data;
    }
    string line;
    while (getline(read, line)) {
        try {
            float val = stof(line);
            data.push_back(val);
        } catch (...) {
            cerr << "Valore non valido: " << line << endl;
        }
    }
    return data;
}

#endif