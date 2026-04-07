#ifndef SOLID_PROP_H
#define SOLID_PROP_H
#include <iostream>
#include <math.h>
#include <cmath>
#include <float.h>
#define pi M_PI // M_PI diventa sempre un pelo pesante da riportare

enum INSTRUCTIONS {
    C2P = 2,
    P2C = 3,
    MEM_NOT_ALLOCATED = -1,
    TYPE_UNREC = -2
};

// trasformazioni/funzioni matematiche/funzioni custom/ecc
int swap_rf_sys (float* c1, float* c2, float** nc1, float** nc2, int L, int TYPE) {
    (*nc1) = new float[L];
    (*nc2) = new float[L];
    if ((*nc1) == nullptr || (*nc2) == nullptr) return MEM_NOT_ALLOCATED;
    if (TYPE == C2P) {
        // c1 è la x e c2 la y
        for (int i = 0; i < L; i++) {
            (*nc1)[i] = sqrt(pow(c1[i],2) + pow(c2[i],2));
            (*nc2)[i] = atan2(c2[i], c1[i]);
        }
        return TYPE;
    } else if (TYPE == P2C) {
        for (int i = 0; i < L; i++) {
            (*nc1)[i] = c1[i] * cos(c2[i]);
            (*nc2)[i] = c1[i] * sin(c2[i]);
        }
        return TYPE;
    } else return TYPE_UNREC;
}
float lerp (float a, float b, float perc) {
    return a + perc*(b-a);
}
float qlerp(float x1, float x2, float x3, float perc) {
    const float eps = 1e-6f;
    // Controllo collinearità: se punto medio è sulla retta tra x1 e x3
    float xm_lin = lerp(x1, x3, 0.5f);
    if (fabsf(x2 - xm_lin) < eps) {
        // Collineari → interpolazione lineare
        return lerp(x1, x3, perc);
    }
    // Interpolazione quadratica di Lagrange
    // P(t) = x1 * L0(t) + x2 * L1(t) + x3 * L2(t)
    // con t0 = 0, t1 = 0.5, t2 = 1
    float t0 = 0.0f, t1 = 0.5f, t2 = 1.0f;
    float t = perc;
    float L0 = ((t - t1) * (t - t2)) / ((t0 - t1) * (t0 - t2));
    float L1 = ((t - t0) * (t - t2)) / ((t1 - t0) * (t1 - t2));
    float L2 = ((t - t0) * (t - t1)) / ((t2 - t0) * (t2 - t1));
    return x1 * L0 + x2 * L1 + x3 * L2;
}
float dist(float x1, float y1, float x2, float y2) {
    return sqrt( pow(x2-x1, 2) + pow(y2-y1, 2) );
}
int remove_duplicates(float* mx, float* my, float** tx, float** ty, int n_p) {
    // se questa non funziona è colpa di ChatGPT perchè è l'unica roba che ha fatto
    const float eps = 1e-6f; // tolleranza per confrontare float
    int unique_count = 0;

    // allocazione temporanea massima (caso peggiore: nessun duplicato)
    float* tmpx = new float[n_p];
    float* tmpy = new float[n_p];
    if (tmpx == nullptr || tmpy == nullptr) return MEM_NOT_ALLOCATED;
    for (int i = 0; i < n_p; i++) {
        bool found = false;
        for (int k = 0; k < unique_count; k++) {
            if (fabsf(mx[i] - tmpx[k]) < eps && fabsf(my[i] - tmpy[k]) < eps) {
                found = true;
                break;
            }
        }
        if (!found) {
            tmpx[unique_count] = mx[i];
            tmpy[unique_count] = my[i];
            unique_count++;
        }
    }
    // allocazione finale della dimensione giusta
    *tx = new float[unique_count];
    *ty = new float[unique_count];
    if( (*tx) == nullptr || (*ty) == nullptr ) return MEM_NOT_ALLOCATED;
    for (int i = 0; i < unique_count; i++) {
        (*tx)[i] = tmpx[i];
        (*ty)[i] = tmpy[i];
    }
    delete[] tmpx;
    delete[] tmpy;
    return unique_count;
}
int remove_duplicates_inplace(float** mx, float** my, int n_p) {
    const float eps = 1e-6f;
    int unique_count = 0;
    float* tmpx = new float[n_p];
    float* tmpy = new float[n_p];
    for (int i = 0; i < n_p; i++) {
        bool found = false;
        for (int k = 0; k < unique_count; k++) {
            if (fabsf((*mx)[i] - tmpx[k]) < eps && fabsf((*my)[i] - tmpy[k]) < eps) {
                found = true;
                break;
            }
        }
        if (!found) {
            tmpx[unique_count] = (*mx)[i];
            tmpy[unique_count] = (*my)[i];
            unique_count++;
        }
    }
    delete[] *mx;
    delete[] *my;
    *mx = new float[unique_count];
    *my = new float[unique_count];
    for (int i = 0; i < unique_count; i++) {
        (*mx)[i] = tmpx[i];
        (*my)[i] = tmpy[i];
    }
    delete[] tmpx;
    delete[] tmpy;
    return unique_count;
}
// generazione dei vertici della stella
int star_port_builder(float D, int N, float h_frac,
    float** theta, float** peak) {
    int L = 2*N+1;
    float h = h_frac/2*D;
    float thetai = 2*pi/N;
    (*theta) = new float[L];
    (*peak)  = new float[L];
    for(int i = 0; i < L; i++) {
        (*theta)[i] = 0; 
        (*peak)[i] = 0; // non il modo più efficiente, ma funziona
    }
    for(int i = 0; i < N; i++) {
        (*theta)[2*i] = thetai*i;
        (*theta)[2*i+1]   = thetai*i + thetai/2;
        (*peak) [2*i] = D/2.0;
        (*peak) [2*i+1] = D/2.0 - h;
    }
    (*theta)[L-1] = 0;
    (*peak)[L-1]  = D/2.0;
    return L; // restituisce la dimensione dei due array
}


// creazione della mesh

int generate_mesh(
    float* x, float* y, float** xm, float** ym,
    int L,           // numero punti originali
    int n_points_out // numero punti desiderati in uscita
) {
    if (L < 2 || n_points_out < 2) return 0;

    // Array per lunghezze cumulative
    float* arc = new float[L];
    if (!arc) return 0;

    arc[0] = 0.0f;
    float total_len = 0.0f;
    for (int i = 1; i < L; i++) {
        total_len += dist(x[i-1], y[i-1], x[i], y[i]);
        arc[i] = total_len;
    }

    // (Opzionale) chiusura della curva
    bool closed = (x[0] == x[L-1] && y[0] == y[L-1]);
    if (!closed) {
        total_len += dist(x[L-1], y[L-1], x[0], y[0]);
    }

    float step = total_len / (n_points_out - 1);

    // Allocazione output
    *xm = new float[n_points_out];
    *ym = new float[n_points_out];
    if (*xm == nullptr || *ym == nullptr) {
        delete[] arc;
        return 0;
    }

    // Generazione punti uniformi
    float target_dist = 0.0f;
    int seg = 0;
    for (int i = 0; i < n_points_out; i++) {
        while (true) {
            int next = (seg + 1) % L;
            float seg_len = dist(x[seg], y[seg], x[next], y[next]);
            float seg_start = arc[seg];
            float seg_end = seg_start + seg_len;
            if (target_dist <= seg_end || seg == L - 1) {
                float t = (seg_len < 1e-12f) ? 0.0f :
                          (target_dist - seg_start) / seg_len;
                (*xm)[i] = x[seg] + t * (x[next] - x[seg]);
                (*ym)[i] = y[seg] + t * (y[next] - y[seg]);
                break;
            } else {
                seg++;
                if (seg >= L) seg = 0;
            }
        }
        target_dist += step;
    }

    delete[] arc;
    return n_points_out;
}

float calculate_perim(float *xm, float *ym, int n_p) {
    float per = 0;
    float x1, y1, x2, y2;
    float c = 1000.0f; // converte da m a mm per evitare problemi numerici
    for (int i = 0; i < n_p; i++) {
        x1 = xm[i]; y1 = ym[i];
        (i == n_p - 1) ? x2 = xm[0] : x2 = xm[i+1];
        (i == n_p - 1) ? y2 = ym[0] : y2 = ym[i+1];
        per += dist(x1*c, y1*c, x2*c, y2*c);
    }
    return per/c;
}

// traslazione della mesh (burning del grano)
int translate_mesh(float* meshx, float* meshy, float step, float** bx, float** by, int n_p) {
    float* dx, *dy;
    dx = new float[n_p];
    dy = new float[n_p];
    (*bx) = new float[n_p];
    (*by) = new float[n_p];
    if (dx == nullptr || dy == nullptr || (*bx) == nullptr || (*by) == nullptr) return MEM_NOT_ALLOCATED;
    for (int i = 0; i < n_p; i++) {
        // calcolo la derivata
        float xm1, ym1, xp1, yp1;
        if(i == 0) {
            xm1 = meshx[n_p-1];
            ym1 = meshy[n_p-1];
            xp1 = meshx[i+1];
            yp1 = meshy[i+1];
        } else if(i == n_p - 1) {
            xp1 = meshx[0];
            yp1 = meshy[0];
            xm1 = meshx[i-1];
            ym1 = meshy[i-1];
        } else {
            xp1 = meshx[i+1];
            yp1 = meshy[i+1];
            xm1 = meshx[i-1];
            ym1 = meshy[i-1];
        }
        dx[i] = xp1 - xm1;
        dy[i] = yp1 - ym1;
        float norm_der = sqrt(pow(dx[i], 2) + pow(dy[i], 2));
        float nx = -dy[i], ny = dx[i]; // rotazione di 90°
        if(norm_der != 0) {nx /= norm_der; ny /= norm_der; }
        (*bx)[i] = meshx[i] - step*nx;
        (*by)[i] = meshy[i] - step*ny;
    }
    return n_p;
}
int filter_mesh(float* meshx, float* meshy, float step, float** bx, float ** by, int n_p) {
    // cerca il raggio minimo della nuova mesh e filtra via ogni valore maggiore.
    // Questo codice non è efficiente, ma funziona hehe
    // Questa funzione da sola porterebbe a dei bug, quindi dopo questo verrà usato un altro algoritmo
    // per rimappare i punti della nuova mesh
    // NB!!! il valore restituito è la lunghezza della nuova mesh
    float* rho, *theta;
    int check = swap_rf_sys(meshx, meshy, &rho, &theta, n_p, C2P);
    if (check == MEM_NOT_ALLOCATED) return check;
    float r_min = FLT_MAX;
    for(int i = 0; i < n_p; i++) {
        if (rho[i] < r_min) r_min = rho[i];
    }
    delete[] rho;
    delete[] theta; // theta non è mai servita ma non so come scartarla
    r_min += step*sqrt(2);
    // r_min /= 2; // DEBUG FLAG!!!!
    float* nm1, *nm2;
    check = translate_mesh(meshx, meshy, step, &nm1, &nm2, n_p);
    if(check == MEM_NOT_ALLOCATED) return check;
    int L_nm = 0;
    for(int i = 0; i < n_p; i++) {
        if( sqrt(pow(nm1[i], 2) + pow(nm2[i], 2)) < r_min ) {
            nm1[i] = 0; nm2[i] = 0;
        } else L_nm ++;
    }
    (*bx) = new float[L_nm];
    (*by) = new float[L_nm];
    int iter = 0;
    for(int i = 0; i < n_p; i++) {
        if( nm1[i] != 0 || nm2[i] != 0 ) {
            (*bx)[iter] = nm1[i];
            (*by)[iter] = nm2[i];
            iter++;
        }
    }
    delete[] nm1;
    delete[] nm2;
    return L_nm;
}

// ripopolazione della mesh
bool der_is_equal(float* meshx, float* meshy, int ind, int n_p) {
    // eh si... lo devo ammettere... la mia funzione con 6 if non era elegante come 
    // la versione di ChatGPT.... https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcRL0_-sGJcz3SnlEIFRHMt6i3BBVY5nqogF5w&s
    auto idx = [&](int i) { return (i + n_p) % n_p; };
    auto feq = [](float a, float b, float eps = 1e-6f) {
        return fabsf(a - b) < eps;
    };

    int im2 = idx(ind - 2);
    int im1 = idx(ind - 1);
    int ip1 = idx(ind + 1);
    int ip2 = idx(ind + 2);

    float x   = meshx[ind], y   = meshy[ind];
    float xm2 = meshx[im2],  ym2 = meshy[im2];
    float xm1 = meshx[im1],  ym1 = meshy[im1];
    float xp1 = meshx[ip1],  yp1 = meshy[ip1];
    float xp2 = meshx[ip2],  yp2 = meshy[ip2];

    float dxm = x   - xm2;  float dym = y   - ym2;  // derivata precedente
    float dx  = xp1 - xm1;  float dy  = yp1 - ym1;  // derivata attuale
    float dxp = xp2 - x;    float dyp = yp2 - y;    // derivata successiva

    return feq(dx, dxm) && feq(dx, dxp) && feq(dy, dym) && feq(dy, dyp);
}
int find_vertex(float* meshx, float* meshy, float** vx, float ** vy, int n_p) {
    // salva in vx e vy i valori dei vertici, in pratica "smembra" la mesh
    float* bufvx = new float [n_p]; // worst case scenario, ogni punto è disallineato
    float* bufvy = new float [n_p]; // worst case scenario, ogni punto è disallineato
    if (bufvx == nullptr || bufvy == nullptr ) return MEM_NOT_ALLOCATED;
    int L_vx = 0;
    for(int i = 0; i < n_p; i++) {
        // calcolo la derivata (copia incollato da translate_mesh)
        if (!der_is_equal(meshx, meshy, i, n_p)) {
            bufvx[L_vx] = meshx[i];
            bufvy[L_vx] = meshy[i];
            L_vx ++;
        }
    }
    // infine, copio bufvx su vx
    (*vx) = new float[L_vx];
    (*vy) = new float[L_vx];
    if(*vx == nullptr || *vy == nullptr) return MEM_NOT_ALLOCATED;
    for (int i = 0; i < L_vx; i++) {
        (*vx)[i] = bufvx[i];
        (*vy)[i] = bufvy[i];
    }
    delete[] bufvx, bufvy;
    return L_vx;
}
// algoritmi di chaikin
int chaikin(float* vx, float* vy, float** tx, float **ty, int n_p) {
    // usa l'algoritmo di chaikin per rendere più "smooth" la mappa dei vertici
    // NB questo codice funziona solo per poligoni chiusi
    int Lt = 2*n_p; //n_p è il numero di vertici, quindi di segmenti
    (*tx) = new float[Lt];
    (*ty) = new float[Lt];
    if ((*tx) == nullptr || (*ty) == nullptr) return MEM_NOT_ALLOCATED;
    int count = 0;
    float c1 = 19.0f/20.0f;
    float c2 = 1.0f - c1;
    for(int i = 0; i < n_p; i++) {
        float xi = vx[i], yi = vy[i];
        int next = (i + 1) % n_p; // indice successivo
        float xip1 = vx[next], yip1 = vy[next];
        (*tx)[count]   = c1 * xi + c2 * xip1;
        (*ty)[count++] = c1 * yi + c2 * yip1;
        (*tx)[count]   = c2 * xi + c1 * xip1;
        (*ty)[count++] = c2 * yi + c1 * yip1;
    }
    return Lt;
}
int chaikin_iterative(const float* vx, const float* vy, int n_p,
                             int iterations, int start_offset,
            
                             float** outx, float** outy) {
    if (n_p < 3 || iterations < 1) {
        *outx = nullptr;
        *outy = nullptr;
        return 0;
    }

    // Copia iniziale dei punti di partenza con offset
    float* curx = new float[n_p];
    float* cury = new float[n_p];
    if (!curx || !cury) return MEM_NOT_ALLOCATED;

    for (int i = 0; i < n_p; i++) {
        int j = (i + start_offset) % n_p;
        curx[i] = vx[j];
        cury[i] = vy[j];
    }
    int curN = n_p;

    // Iterazioni di smoothing
    for (int it = 0; it < iterations; it++) {
        int newN = 2 * curN;
        float* newx = new float[newN];
        float* newy = new float[newN];
        if (!newx || !newy) return MEM_NOT_ALLOCATED;

        int count = 0;
        float c1 = 19.0f/20.0f;
        float c2 = 1.0f - c1;
        for (int i = 0; i < curN; i++) {
            int next = (i + 1) % curN; // poligono chiuso
            float xi   = curx[i];
            float yi   = cury[i];
            float xip1 = curx[next];
            float yip1 = cury[next];
            // Q: punto vicino al corrente
            newx[count]   = c1 * xi + c2 * xip1;
            newy[count++] = c1 * yi + c2 * yip1;
            // R: punto vicino al successivo
            newx[count]   = c2 * xi + c1 * xip1;
            newy[count++] = c2 * yi + c1 * yip1;
        }
        delete[] curx;
        delete[] cury;
        curx = newx;
        cury = newy;
        curN = newN;
    }
    *outx = curx;
    *outy = cury;
    return curN;
}


#endif
// codice a partecipazione collettiva, scritto principalmente da LP





// int generate_mesh(float* x, float* y, float** xm, float** ym, int L ,int ceil_points, int N) {
//     // idealmente ceil_points sarà la dimensione di xm e ym, ma nel caso così non dovesse essere, la dimensione effettiva è restituita
//     // L = 2*N+1 -> N = (L-1)/2
//     // nota: per correttezza andrebbe ripopolata la mesh proporzionalmente alla distanza dei punti, per ora funziona ma nel caso questa è una flag
//     // int N = (L-1)/2;
//     // int n_p = ceil_points /2 /N; // il numero di punti per "lato". NON è ovvio che ceil_points/2N * 2N = ceil_points (type casting)
//     int n_p = ceil_points; // il nome è forviante...
//     if (n_p == 0) return MEM_NOT_ALLOCATED;
//     float *buffx , *buffy;
//     buffx = new float[n_p*L];
//     buffy = new float[n_p*L];
//     if(buffx == nullptr || buffy == nullptr) return EXIT_FAILURE;
//     float dist_toll = 1e-4;
//     int count = 0;
//     for(int i = 0; i < L; i++) {
//         float x1 = x[i], y1 = y[i], x2, y2, x3, y3;
//         int prev = (i - 1 + L) % L;
//         int next = (i+1) % L;
//         x1 = x[prev]; x2 = x[i]; x3 = x[next];
//         y1 = y[prev]; y2 = y[i]; y3 = y[next];
//         if (dist(x2, y2, x3, y3) > dist_toll) {
//             for(int j = 0; j < n_p; j++) {
//                 float perc = (1.0f*j)/n_p;
//                 // (*xm)[n_p*i + j] = qlerp(x1, x2, x3, perc);
//                 // (*ym)[n_p*i + j] = qlerp(y1, y2, y3, perc);
//                 buffx[count] = lerp(x2, x3, perc);
//                 buffy[count] = lerp(y2, y3, perc);
//                 count ++;
//             }
//         } else {
//             buffx[count] = x[i];
//             buffy[count++] = y[i];
//             buffx[count] = x[next];
//             buffy[count++] = y[next];
//         }
//     }
//     *xm = buffx;
//     *ym = buffy;
//     int newL = remove_duplicates_inplace(xm, ym, count);
//     return newL;
// }