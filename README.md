# SOProgetto

Progetto relativo al corso di Sistemi Operativi : 2° anno Università d'informatica di Trento

## Autori

[Paolo Tasin](https://github.com/Piro17) [194224]  - [Stefano Perenzoni](https://github.com/Perenz) [193713] - [Marcello Rigotti](https://github.com/rigottimarcello) [192865]

## Specifiche manuale 

Eseguibile digitando **make hand**, è necessario ci sia aperta una centralina (eseguibile **centralina**) per permettere la connessione tra i due.
Il processo manuale si occuperà autonomamente di recuperare il PID della centralina tramite un processo di supporto.

***A questo punto ci sono due possibili insiemi di comandi***:

* Il **primo** è relativo ai comandi eseguibili mentre non si è in controllo di alcun dispositivo, devono permettere all'utente di assicurarsi il controllo diretto di un dispositivo
Si ha:
	* **control** idDispositivo: indicando un idDispositivo valido il processo manuale contatterà la centralina tramite FIFO la quale risponderà con le informazione principali sul dispositivo cercato, cioe PID e tipo. A questo punto si sta controllando un dispositivo e si possono eseguire i comandi del secondo insieme.

	* **exit**: Verrà eseguita un uscita sicura dal sistema, tale comando è eseguibile anche mentre si controlla un dispositivo.
	
	* **help** :Verranno indicati i diversi comandi permessi da entrambi gli insieme, tale comando è eseguibile anche mentre si controlla un dispositivo.
	
* Il **secondo** insieme è relativo ai comandi eseguibili dopo aver eseguito il commando 'control' avendo quindi il controllo di un dispositivo del quale verranno indicate le informazioni
principali nella Command Line Interface
Si ha:
	* **switch** *"label"* *"valore"*: Permette di agire sugli interruttori dei dispositivi e di conseguenza cambiarne lo stato ed alcuni registri
		* *label disponibili*: 	
			* **accensione** per agire su BULB o controllori(TIMER e HUB) tramite i valori on/off
			* **apertura** per agire su FRIDGE/WINDOW o controllori tramite i valori on/off (Utilizzando questa label verso un dispositivo di controllo si agirà sull'apertura sia dei FRIDGE che delle WINDOW controllate						* **aperturaF** per agire su WINDOW o controllori tramite i valori on/off (Utilizzando questa label verso un dispositivo di controllo si agirà sull'apertura delle sole WINDOW)
			* **aperturaW** per agire su WINDOW o controllori tramite i valori on/off (Utilizzando questa label verso un dispositivo di controllo si agirà sull'apertura dei soli FRIDGE)
			* **termostato** per agire su FRIDGE o controllori tramite un valore numerico intero, permetterà di cambiare la temperatura interna del dispositivo controllato
	* **set "label" "valore"**: Permette di agire sui registri dei FRIDGE e di eventuali loro controllori.
		* *label disponibili*: 
			* *termostato*: Si comporta in maniera identica alla switch con label termostato
			* *perc* per settare la percentuale di riempimento, accetta valori interi compresi tra 0 e 100
			* *delay* per settare l'intervallo di tempo dopo il quale un frigo aperto si chiude automaticamente
	
	* **release**: Per rilasciare il dispositivo controllato, solamente dopo aver rilasciato quello attuale si potrà prendere il controllo di uno nuovo, sempre tramite comando 'control'.
	* **exit**: Per eseguire un uscita sicura dal sistema
	* **help**: Verranno indicati i diversi comandi permessi da entrambi gli insieme

## Specifiche *NON* manuale 
Eseguibile digitando **make exec** nella cartella project.

* **Comandi:**
	* **clear** : Pulisce l'output

	* **help** : Restituisce l'elenco dei comandi diponibili

	* **exit** : Esce dalla centralina uccidendo i processi e chiudendo la FIFO utilizzata nei comandi manuali

	* **add** *"tipoDispositivo"*: Aggiunge un dispositivo al magazzino

		* *Lista dispositivi disponibili:*
			* Hub (versione ibrida con controllo di dispositivi etereogenei)
			* Bulb
			* Fridge
			* Window
			* Timer (parzialmente implementato)
	* **list** : Restituisce l'intera lista dei dispositivi collegati riferendosi al vero albero dei processi. Per restituire tale lista vi è una comunicazione attraverso l'intero albero dei processi via pipe (pipe tra padre e figlio). Non vengono utilizzate altre strutture al di fuori delle pipe. Il meccanismo basilare è quello di una dfs su un albero non binario in cui ogni nodo risponde e i nodi già visitati inoltrano tale risposta verso la radice (la centralina nel nostro caso). Il meccanismo verrà spiegato più in dettaglio durante la discussione del progetto.

	* **delete** :

		* *delete* "idDispositivo": Elimina il dispositivo "idDispositivo" e, nel caso in cui esso sia un dispositivo di controllo con dei figli, elimina anche tali figli.
		* *delete --all* : Elimina tutti i dispositivi collegati alla centralina.
		* *switch* : Cambia lo stato di un dispositivo (vedi comandi manuali per i dettagli delle label e delle varie pos). Funziona sia su dispositivi di interazione che dispositivi di controllo (agisce a cascata sui dispositivi controllati)

	* **info** :

		* *info* "idDispositivo": Restituisce le informazioni relative al dispositivo . Nel caso di un dispositivo di controllo restituisce anche:
			* il massimo tra i tempi di utilizzo delle varie categorie di dispositivi controllati
			* lo stato dell'interruttore sull'hub e la relative situazione di override in cui gli stati degli interruttori dei figli sono diversi da quelli del dispositivo di controllo (switch da manuale o switch singolo su dispostivo controllato da hub).
	* **info** "idSouce" *-f* : Funzionamento uguale al comando precedentemente indicato ma, nel caso di dispositivi di controllo, forza la risposta anche dei figli restituendo quello che è un sottoalbero dell'albero dei processi.
	* **link** "idSouce" to "idDestinazione"  : Collega il dispositivo (e il suo sottabero se disp. controllo) al dispositivo di controllo . Questo comando (per scelte implementative) non agisce quando "idSouce" e "idDestinazione" appartengono entrambi al magazzino. In tutti gli altri casi esso funziona correttamente. In fase iniziale si potrà collegare i dispositivi alla centralina utilizzando il comando *link to 0*

	* **unlink** :
		* *unlink --all* : scollega tutti i dispositivi dalla centralina e li ripone nel magazzino non mantenendo i collegamenti precedentemente creati.
		* *unlink* "idDispositivo" scollega il dispositivo (anche il suo sottalbero nel caso di dispositivi di controllo) e lo ripone nel magazzino.
	* **power** : accende la centralina se spenta, spegne la centralina se accesa.

## Conclusioni
Per questioni di tempo una spiegazione più specifica verrà effettuata in fase di discussione del progetto.

