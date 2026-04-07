In questa cartella sono presenti i file contenenti le informazioni
sul meshing, l'unico file che comunica con l'esterno è mesh_generator.m,
gli altri sono indipendenti e fatti per generare, visualizzare o 
modificare mesh varie.

Nel particolare:
1) confront_plotter.m:  	genera un plot contenente curve di regressione 
				al variare di determinati parametri.
2) mesh_generator_noinpfile.m:	fa quello che fa mesh_generator ma lasciando al
				programmatore la scelta sui parametri senza dover
				fare richiesta con mesh_requester.cpp (utile per
				debug veloci)
3) search_optimal_params.m:	serve solo a formattare bene i dati sperimentali (vedi progetto)
4) star_port_plotter.m:		dato un file contenente i vertici iniziali, genera 
				la sezione del porto, formattata bene
5) vector_mesh_generator.m:	fa la stessa cosa di confront_plotter in teoria (boh non
				so perché ho fatto entrambi vai a capire)