#ifndef SOLID_PROP2_H
#define SOLID_PROP2_H
#include <iostream>
#include <math.h>
#include <cmath>
#include <float.h>
#include <unordered_set>
#include <vector>
#include <numeric>
#define pi M_PI // M_PI diventa sempre un pelo pesante da riportare
using namespace std;
#define MANUAL_CHOICE_FOR_MPATH // particolarmente utile se serve usare file generati manualmente per la mesh
// #define SAFEMODE // per eseguire in modalità SAFE
enum INSTRUCTIONS {
    C2P = 2,
    P2C = 3,
    MEM_NOT_ALLOCATED = -1,
    TYPE_UNREC = -2,
    WARNING = -3
};
class Point {
    public:
    float x, y;
    Point() : x(0), y(0) {};
    Point(float x, float y) : x(x), y(y) {};
    Point operator+(const Point& other) const;
    Point operator-(const Point& other) const;
    template <typename T>
    Point operator+(T value) const { return Point(x + value, y + value); }
    template <typename T> 
    Point operator-(T value) const { return Point(x - value, y - value);} 
    template <typename T> 
    Point operator/(T value) {
        if (value == 0) return Point(FLT_MAX, FLT_MAX);
        else return Point(x / value, y / value);
    }
    template <typename T> 
    Point operator*(T value) const { return Point(x * value, y * value);}
    template <typename T>
    Point& operator=(T value) {
        x = value; y = value;
        return *this;
    }
    bool operator==(const Point& other) const {if(x == other.x && y == other.y) return true; return false;};
    string tostring();
    float norm() { return sqrt(pow(x, 2) + pow(y, 2)); }
};
Point  Point::operator+(const Point& other) const {
    return Point(this->x + other.x, this->y + other.y);
}
Point  Point::operator-(const Point& other) const {
    return Point(this->x - other.x, this->y - other.y);
}
string Point::tostring() {
    return to_string(x) + "," + to_string(y);
}
vector<float> getX (vector <Point> p) {
    vector <float> x;
    for (auto& i : p)
        x.push_back(i.x);
    return x;
}
vector<float> getY (vector <Point> p) {
    vector <float> y;
    for (auto& i : p)
        y.push_back(i.y);
    return y;
}
// trasformazioni/f matematiche/funzioni custom / ecc.
vector<Point> swap_rf_sys(const std::vector<Point>& pts, int TYPE) {
    vector<Point> result;
    result.reserve(pts.size());
    if (TYPE == C2P) {
        // cartesiano → polare
        for (const auto& p : pts) {
            float r = sqrt(p.x * p.x + p.y * p.y);
            float theta = atan2(p.y, p.x);
            result.emplace_back(r, theta);
        }
    }
    else if (TYPE == P2C) {
        // polare → cartesiano
        for (const auto& p : pts) {
            float x = p.x * cos(p.y);
            float y = p.x * sin(p.y);
            result.emplace_back(x, y);
        }
    }
    else {
        // tipo non riconosciuto: restituisce vuoto
    }
    return result;
}
inline float dist(Point p1, Point p2) {
    return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
}
inline Point lerp(Point p1, Point p2, float perc) {
    return p1 + (p2 - p1)*perc;
}
inline auto approx_equal = [](float a, float b, float tol = 1e-6) {
    return fabs(a - b) < tol;
};
float av_dist (vector<Point> pts) {
    float av = 0.0f;
    for(int i = 0; i < pts.size(); i++) {
        int next = (i+1)%pts.size();
        av += dist(pts.at(i), pts.at(next));
    }
    return av / static_cast<float>( pts.size() );
}
// Hash per unordered_set
struct PointHash {
    std::size_t operator()(const Point& p) const {
        // Arrotonda per evitare problemi di precisione
        auto roundCoord = [](float val) {
            return static_cast<int>(std::round(val * 1e6));
        };
        std::size_t h1 = std::hash<int>()(roundCoord(p.x));
        std::size_t h2 = std::hash<int>()(roundCoord(p.y));
        return h1 ^ (h2 << 1);
    }
};
std::vector<Point> remove_duplicates(const std::vector<Point>& v) {
    // rimuove anche i punti che sono mediamente più vicini ai loro vicini
    std::unordered_set<Point, PointHash> seen;
    std::vector<Point> result;
    result.reserve(v.size());
    for (const auto& p : v) {
        if (seen.insert(p).second) { // se era nuovo
            result.push_back(p);
        }
    }
    return result;
}
vector<Point> remove_clusters(vector<Point> v, float threshold_factor = 0.8f) {
    if (v.size() < 3) return v; // troppo pochi punti per filtrare

    // Calcolo media delle distanze tra punti consecutivi
    float total_d = 0.0f;
    for (size_t i = 0; i < v.size(); i++) {
        size_t next = (i + 1) % v.size();
        total_d += dist(v[i], v[next]);
    }
    float av_d = total_d / v.size();

    // Soglia minima consentita
    float min_allowed_d = av_d * threshold_factor;

    vector<Point> buff;
    buff.reserve(v.size());

    for (size_t i = 0; i < v.size(); i++) {
        size_t next = (i + 1) % v.size();
        size_t prev = (i + v.size() - 1) % v.size();

        // distanza media locale
        float this_av = (dist(v[i], v[next]) + dist(v[i], v[prev])) / 2.0f;

        // Mantieni solo se la distanza media locale supera la soglia
        if (this_av >= min_allowed_d) {
            buff.push_back(v[i]);
        }
    }

    return buff;
}
vector<Point> LPFilter(vector<Point> mesh) {
    vector<float> r_values;
    for (auto &p : mesh) r_values.push_back(p.x);

    vector<float> r_smooth(r_values.size());

    int window = 3; // finestra di smoothing
    for (int i = 0; i < r_values.size(); i++) {
        float sum = 0;
        int count = 0;
        for (int w = -window; w <= window; w++) {
            int idx = (i + w + r_values.size()) % r_values.size(); // wrap-around
            sum += r_values[idx];
            count++;
        }
        r_smooth[i] = sum / count;
    }

    // Poi sostituisci i valori r con quelli smussati
    for (int i = 0; i < mesh.size(); i++) {
        mesh[i].x = r_smooth[i];
    }
    return mesh;
}
int find_closest(float target, vector <float> v) {
    float dist = FLT_MAX;
    int ind = 0;
    for (int i = 0; i < v.size(); i++) {
        float D = abs(target - v.at(i));
        if (D < dist) {
            ind = i;
            dist = D;
        }
    }
    return ind;
} 
// funzioni geometriche specifiche del grano
double angoloFraStelle(int N, double D, double h_frac) {
    double R_e = D / 2.0;
    double R_i = R_e * (1.0 - h_frac);

    // Angolo della valle (prendiamo k=0)
    double beta = M_PI / N;

    // Coordinate valle
    double Vx = R_i * cos(beta);
    double Vy = R_i * sin(beta);

    // Coordinate punte adiacenti
    double P1x = R_e * cos(0.0);
    double P1y = R_e * sin(0.0);

    double P2x = R_e * cos(2.0 * M_PI / N);
    double P2y = R_e * sin(2.0 * M_PI / N);

    // Vettori valle->punta
    double Vp1x = P1x - Vx;
    double Vp1y = P1y - Vy;
    double Vp2x = P2x - Vx;
    double Vp2y = P2y - Vy;

    // Prodotto scalare
    double dot = Vp1x * Vp2x + Vp1y * Vp2y;

    // Moduli
    double magVp1 = sqrt(Vp1x*Vp1x + Vp1y*Vp1y);
    double magVp2 = sqrt(Vp2x*Vp2x + Vp2y*Vp2y);

    double angle = acos(dot / (magVp1 * magVp2));

    return angle; // radianti
}
// generazione della stella
vector<Point> star_port_builder(float D, int N, float h_frac) {
    int L = 2*N+1;
    float h = h_frac/2*D;
    float thetai = 2*pi/N;
    vector<Point> pol;
    pol.reserve(L);
    fill(pol.begin(), pol.end(), 0); // inizializzo a zero
    for(int i = 0; i < N; i++) {
        pol.push_back( Point(D/2.0f,   thetai*i) );
        pol.push_back( Point(D/2.0f-h, thetai*i + thetai/2) );
    }
    pol.push_back( Point(D/2.0f, 0) );
    return pol;
}
// generazione della mesh
std::vector<Point> generate_mesh(std::vector<Point> vert, int n_points_out) {
    int L = vert.size();
    if (L < 2 || n_points_out < 2) return vert;

    std::vector<float> arc;
    arc.push_back(0);

    float total_len = 0;
    // lunghezza segmenti da 0 a L-1
    for (int i = 1; i < L; i++) {
        total_len += dist(vert[i-1], vert[i]);
        arc.push_back(total_len);
    }
    // segmento chiusura da L-1 a 0
    total_len += dist(vert[L-1], vert[0]);
    arc.push_back(total_len);

    float step = total_len / (n_points_out - 1);
    std::vector<Point> mesh;

    float target_dist = 0.0f;
    int seg = 0;
    const float TOLL = 1e-6;

    for (int i = 0; i < n_points_out; i++) {
        while (true) {
            float seg_start = arc[seg];
            float seg_end = arc[seg+1]; // arc ha L+1 elementi

            if (target_dist <= seg_end || seg == L-1) {
                int next = (seg + 1) % L;
                float seg_len = dist(vert[seg], vert[next]);
                float t = (seg_len < TOLL) ? 0.0f : (target_dist - seg_start) / seg_len;

                Point p = vert[seg] + (vert[next] - vert[seg]) * t;
                mesh.push_back(p);
                break;
            } else {
                seg++;
                if (seg >= L) seg = 0;
            }
        }
        target_dist += step;
    }

    return mesh;
}
float find_r_min(vector <Point> mesh, int N, float h_frac, float D, float step) {
    mesh = swap_rf_sys(mesh, C2P);
    float theta = angoloFraStelle(N, D, h_frac) / 2.0f;
    float d = 2*cos(theta);
    float r_min = FLT_MAX;
    for (auto& i : mesh) {
        if (i.x < r_min) r_min = i.x;
    }
    return r_min + step*d;
}
vector <Point> translate_mesh(vector<Point> old_mesh, float step) {
    vector <Point> der, nor;
    vector <Point> new_mesh;
    int n_p = old_mesh.size();
    for(int i = 0; i < n_p; i++) {
        int next = (i+1) % n_p;
        int prev = (i - 1 + n_p) % n_p;
        der.push_back(old_mesh.at(next) - old_mesh.at(prev));
        nor.emplace_back(-der.at(i).y, der.at(i).x);
        nor.at(i) = nor.at(i)/nor.at(i).norm();
        new_mesh.push_back(old_mesh.at(i) - nor.at(i)*step);
    }
    new_mesh = swap_rf_sys(new_mesh, C2P);
    // per evitare possibili casi di scambi, implemento un (orribile) BubbleSort per riordinare gli indici (grazie GeeksForGeeks)
    for (int i = 0; i < n_p-1; i++) {

        // Last i elements are already
        // in place
        for (int j = 0; j < n_p - i - 1; j++) {
          
          	// Comparing adjacent elements
            if (new_mesh.at(j).y > new_mesh.at(j+1).y) {
                auto buff = new_mesh.at(j);
                new_mesh.at(j)     = new_mesh.at(j + 1);
                new_mesh.at(j + 1) = buff;
            }
        }
    }
    return swap_rf_sys(new_mesh, P2C);
}
vector<Point> filter_mesh(vector<Point> mesh, float r_min, int N) {
    // Passa da cartesiane a polari
    mesh = swap_rf_sys(mesh, C2P);

    vector<Point> filtered_mesh;
    filtered_mesh.reserve(mesh.size());
    for (auto &p : mesh) {
        float r = p.x;
        float theta = p.y;
        // Normalizza l'angolo tra 0 e 2π per sicurezza
        while (theta < 0)      theta += 2 * M_PI;
        while (theta >= 2*M_PI) theta -= 2 * M_PI;

        // Se il raggio è troppo piccolo, clamp a r_min
        // Soglia "zona di transizione" per ammorbidire il clamp
        float r_soft = r_min * 1.1f; // ad esempio 10% sopra r_min
        if (r < r_min) {
            r = r_min;
            vector<float> spikes;
            float ang2 = 2.0f*pi / N / 2.0f;
            for(int i = 0; i < N; i++) {
                spikes.push_back( ( (2.0f * M_PI * i) / N) + ang2 );
            }
            int ind = find_closest(theta, spikes);
            theta = spikes[ind];
        }
        filtered_mesh.emplace_back(r, theta); // ancora in coordinate polari
    }
    // Rimuovo duplicati se serve
    filtered_mesh = remove_duplicates(filtered_mesh);
    // Torno a coordinate cartesiane
    filtered_mesh = swap_rf_sys(filtered_mesh, P2C);
    filtered_mesh = remove_clusters(filtered_mesh);
    return filtered_mesh;
}
vector<Point> filter_mesh_safe(vector<Point> mesh, float r_min, int N) {
    // Passa da cartesiane a polari
    mesh = swap_rf_sys(mesh, C2P);

    vector<Point> filtered_mesh;
    filtered_mesh.reserve(mesh.size());
    for (auto &p : mesh) {
        float r = p.x;
        float theta = p.y;
        // Normalizza l'angolo tra 0 e 2π per sicurezza
        while (theta < 0)      theta += 2 * M_PI;
        while (theta >= 2*M_PI) theta -= 2 * M_PI;
        if (r >= r_min) {
            filtered_mesh.emplace_back(r, theta);
        }
    }
    // Rimuovo duplicati se serve
    filtered_mesh = remove_duplicates(filtered_mesh);
    filtered_mesh = LPFilter(filtered_mesh);
    // Torno a coordinate cartesiane
    filtered_mesh = swap_rf_sys(filtered_mesh, P2C);
    return filtered_mesh;
}
// calcoli geometrici (che sta bruciando)
float calculate_perim(vector<Point> mesh, float D) {
    // la mesh è sufficientemente densa per poter evitare di calcolare gli
    // effetti di bordo quando un punto è oltre D e l'altro no
    vector <bool> overD; // true se è oltre il bordo
    vector <Point> buff = swap_rf_sys(mesh, C2P);
    for(auto& b : buff) {
        if (b.x > D/2) overD.push_back(true); 
        else overD.push_back(false);
    }
    float burning_perim = 0;
    for(int i = 0; i < mesh.size(); i++) {
        float next = (i + 1) % mesh.size();
        if(!overD.at(i) && !overD.at(next)) 
            burning_perim += dist(mesh.at(i), mesh.at(next));
    }
    return burning_perim;
}
inline float getdA(Point P1, Point P2) {
    // inline per raccogliere tutta la velocità possibile dove serve
    return 0.5f * P1.x * P2.x * sinf(P2.y - P1.y);
}
float calculate_area(vector<Point> mesh, float D) {
    // calcola l'area libera in sezione, se i punti vanno oltre
    // il bordo del booster, vengono "spostati" sulla circonferenza
    float A = 0;
    mesh = swap_rf_sys(mesh, C2P);
    for(int i = 0; i < mesh.size(); i++) {
        int next = (i+1)%mesh.size();
        Point p1 = mesh.at(i);    if (p1.x > D) p1.x = D;
        Point p2 = mesh.at(next); if (p2.x > D) p2.x = D;
        A += getdA(p1, p2);
    }
    return A;
}
// funzione che riassume un po' tutto
// queste due funzioni funzionano solo a step costante (che non è realistico)
// rimangono per motivi di debug, ma non hanno nessun significato fisico
vector<float> getBurningCurve(int N, float D, float h_fr, float step, float n_points_per_it, float endD) {
    // finalmente questa è la funzione crispy che mette tutto assieme *bacio dello chef*
    vector<float> p_vec; // perimeter vec (NON PRESSURE VEC)
    vector<Point> start = star_port_builder(D, N, h_fr);
    start = swap_rf_sys(start, P2C);
    vector <Point> mesh  = generate_mesh(start, n_points_per_it);
    mesh = remove_duplicates(mesh);
    float perim = calculate_perim(mesh, endD);
    p_vec.push_back(perim);
    float r_min;
    int i_max = 1000, count = 0;
    while (count < i_max && perim > 0) {
        r_min = find_r_min(mesh, N, h_fr, D, step);
        vector <Point> newm = translate_mesh(mesh, step);
        #ifdef SAFEMODE
        newm = filter_mesh_safe(newm, r_min, N); // filtro la mesh traslata
        #else 
        newm = filter_mesh(newm, r_min, N); // filtro la mesh traslata
        #endif
        newm = generate_mesh(newm, n_points_per_it); // ripopolo la mesh
        perim = calculate_perim(newm, endD);
        p_vec.push_back( perim );
        mesh = newm;
        count ++;
    }
    if(count >= i_max) p_vec.push_back(WARNING);
    return p_vec;
}
vector<float> getBurningCurve(int N, float D, float h_fr, float step, float n_points_per_it, float endD, vector<Point>* buffer) {
    // finalmente questa è la funzione crispy che mette tutto assieme *bacio dello chef*
    vector<float> p_vec;
    vector<Point> start = star_port_builder(D, N, h_fr);
    start = swap_rf_sys(start, P2C);
    vector <Point> mesh  = generate_mesh(start, n_points_per_it);
    mesh = remove_duplicates(mesh);
    for (auto& nm : mesh) {
        (*buffer).push_back(nm);
    }
    float perim = calculate_perim(mesh, endD);
    p_vec.push_back(perim);
    float r_min;
    int i_max = 100, count = 0;
    while (count < i_max && perim > 0) {
        r_min = find_r_min(mesh, N, h_fr, D, step);
        vector <Point> newm = translate_mesh(mesh, step);
        #ifdef SAFEMODE
        newm = filter_mesh_safe(newm, r_min, N); // filtro la mesh traslata
        #else 
        newm = filter_mesh(newm, r_min, N); // filtro la mesh traslata
        #endif
        newm = generate_mesh(newm, n_points_per_it); // ripopolo la mesh
        perim = calculate_perim(newm, endD);
        p_vec.push_back( perim );
        mesh = newm;
        count ++;
        for (auto& nm : newm) {
            (*buffer).push_back(nm);
        }
    }
    if(count >= i_max) p_vec.push_back(WARNING);
    return p_vec;
}
#endif