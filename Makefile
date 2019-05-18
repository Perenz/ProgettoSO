define NEWLINE

endef

help:
	$(info Progetto SO realizzato da:Paolo Tasin, Stefano Perenzoni e Marcello Rigotti)
	$(NEWLINE)
	$(info Centralina per controllo domotico)
	$(NEWLINE)
	$(info Digitare 'make build' per compilare l'intero sistema e generare gli eseguibili)
	$(NEWLINE)
	$(info Digitare 'make exec' per compilare l'intero sistema, generare ed infine eseguire gli eseguibili)
	$(NEWLINE)
	$(info Digitare 'make clean' per rimuovere gli eseguibili)


build: compilaCmd compilaMain compilaManuale

compilaCmd:
	gcc -std=gnu90 -o componenti/BULB componenti/bulb.c
	gcc -std=gnu90 -o componenti/FRIDGE componenti/fridge.c
	gcc -std=gnu90 -o componenti/WINDOW componenti/window.c
	gcc -std=gnu90 -o componenti/HUB componenti/hub.c
	gcc -std=gnu90 -o componenti/TIMER componenti/timer.c

compilaManuale:
	gcc -std=gnu90 -o supporto/CENPIDREAD supporto/getCenPid.c
	gcc -std=gnu90 -o manuale/CENPIDWRITE manuale/manuale.c

compilaMain:
	gcc -std=gnu90 -o PRINCIPALE strutture/list.c main.c

binary=PRINCIPALE supporto/CENPIDREAD manuale/CENPIDWRITE componenti/BULB componenti/FRIDGE componenti/WINDOW componenti/HUB componenti/TIMER

clean:
	rm -f $(binary)

hand:
	gcc -std=gnu90 -o manuale/CENPIDWRITE manuale/manuale.c
	manuale/CENPIDWRITE

exec: build
	./PRINCIPALE