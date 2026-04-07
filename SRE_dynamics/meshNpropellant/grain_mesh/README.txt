In questa cartella sono presenti i file contenenti le informazioni
sul meshing, l'unico file che comunica con l'esterno è mesh_generator.m,
gli altri sono indipendenti e fatti per generare, visualizzare o 
modificare mesh varie.

Nel particolare:
1) mesh_generator.m: genera la mesh di regressione partendo da un file 
                iniziale. Ha bisogno per partire di un file che contenga i 
                vertici della geometria e il raggio esterno del booster, il
                tutto in formato .txt
                Funziona in modo un po' sporco per geometrie regolari come
                la circolare.
2) mesh_generator_noinpfile.m:	fa quello che fa mesh_generator ma lasciando al
				programmatore la scelta sui parametri senza dover
				fare richiesta con mesh_requester.cpp (utile per
				debug veloci)

L'output di questi due script è un file che finisce in _graph.txt che 
contiene le informazioni relative alla curva di regressione. In particolare
contiene: [perimetro (burning area), area (volume interno) e step]