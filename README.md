# SOProgetto

Progetto relativo al corso di Sistemi Operativi : 2° anno Università d'informatica di Trento

## Autori

[Paolo Tasin](https://github.com/Piro17) [194224]  - [Stefano Perenzoni](https://github.com/Perenz) [193713] - [Marcello Rigotti](https://github.com/rigottimarcello) []

## Specifiche manuale 

Eseguibile digitando **make hand**, è necessario ci sia aperta una centralina (eseguibile **centralina**) per permettere la connessione tra i due.
Il processo manuale si occuperà autonomamente di recuperare il PID della centralina tramite un processo di supporto.

***A questo punto ci sono due possibili insiemi di comandi***:

* Il **primo** è relativo ai comandi eseguibili mentre non si è in controllo di alcun dispositivo, devono permettere all'utente di assicurarsi il controllo diretto di un dispositivo
Si ha:
	* **control** *<idDispositivo>*: indicando un idDispositivo valido il processo manuale contatterà la centralina tramite FIFO la quale risponderà con le informazione principali sul dispositivo cercato, cioe PID e tipo. A questo punto si sta controllando un dispositivo e si possono eseguire i comandi del secondo insieme.

	* **exit**: Verrà eseguita un uscita sicura dal sistema, tale comando è eseguibile anche mentre si controlla un dispositivo.
	
	* **help** :Verranno indicati i diversi comandi permessi da entrambi gli insieme, tale comando è eseguibile anche mentre si controlla un dispositivo.
	
* Il **secondo** insieme è relativo ai comandi eseguibili dopo aver eseguito il commando 'control' avendo quindi il controllo di un dispositivo del quale verranno indicate le informazioni
principali nella Command Line Interface
Si ha:
	* **switch** *<label>* *<valore>*: Permette di agire sugli interruttori dei dispositivi e di conseguenza cambiarne lo stato ed alcuni registri
		* *label disponibili*: 	
			* **accensione** per agire su BULB o controllori(TIMER e HUB) tramite i valori on/off
			* **apertura** per agire su FRIDGE/WINDOW o controllori tramite i valori on/off (Utilizzando questa label verso un dispositivo di controllo si agirà sull'apertura sia dei FRIDGE che delle WINDOW controllate						* **aperturaF* per agire su WINDOW o controllori tramite i valori on/off (Utilizzando questa label verso un dispositivo di controllo si agirà sull'apertura delle sole WINDOW)
			* **aperturaW** per agire su WINDOW o controllori tramite i valori on/off (Utilizzando questa label verso un dispositivo di controllo si agirà sull'apertura dei soli FRIDGE)
			* **termostato** per agire su FRIDGE o controllori tramite un valore numerico intero, permetterà di cambiare la temperatura interna del dispositivo controllato
	* ***set <label><valore>***: Permette di agire sui registri dei FRIDGE e di eventuali loro controllori
				* *label disponibili*: 
					* **termostato**: Si comporta in maniera identica alla switch con label termostato
					* **perc** per settare la percentuale di riempimento, accetta valori interi compresi tra 0 e 100
					* **delay** per settare l'intervallo di tempo dopo il quale un frigo aperto si chiude automaticamente
	* **release**: Per rilasciare il dispositivo controllato, solamente dopo aver rilasciato quello attuale si potrà prendere il controllo di uno nuovo, sempre tramite comando 'control'
			* **exit**: Per eseguire un uscita sicura dal sistema
			* **help**: Verranno indicati i diversi comandi permessi da entrambi gli insieme

## Specifiche *NON* manuale 

