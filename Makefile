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
	gcc -std=gnu90 -o binaries/BULB componenti/bulb.c
	gcc -std=gnu90 -o binaries/FRIDGE componenti/fridge.c
	gcc -std=gnu90 -o binaries/WINDOW componenti/window.c
	gcc -std=gnu90 -o binaries/HUB componenti/hub.c
	gcc -std=gnu90 -o binaries/TIMER componenti/timer.c

compilaManuale:
	gcc -std=gnu90 -o binaries/CENPIDREAD supporto/getCenPid.c
	gcc -std=gnu90 -o binaries/CENPIDWRITE manuale/manuale.c

compilaMain:
	gcc -std=gnu90 -o PRINCIPALE strutture/list.c main.c

binary=PRINCIPALE binaries/CENPIDREAD binaries/CENPIDWRITE binaries/BULB binaries/FRIDGE binaries/WINDOW binaries/HUB binaries/TIMER

clean:
	rm -f $(binary)

hand:
	gcc -std=gnu90 -o binaries/CENPIDWRITE manuale/manuale.c
	binaries/CENPIDWRITE

exec: build
	./PRINCIPALE