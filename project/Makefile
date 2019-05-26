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
	gcc -std=gnu90 -o binaries/BULB src/componenti/bulb.c
	gcc -std=gnu90 -o binaries/FRIDGE src/componenti/fridge.c
	gcc -std=gnu90 -o binaries/WINDOW src/componenti/window.c
	gcc -std=gnu90 -o binaries/HUB src/componenti/hub.c
	gcc -std=gnu90 -o binaries/TIMER src/componenti/timer.c

compilaManuale:
	gcc -std=gnu90 -o binaries/CENPIDREAD src/supporto/getCenPid.c
	gcc -std=gnu90 -o binaries/MANUALE src/manuale/manuale.c

compilaMain:
	gcc -std=gnu90 -o CENTRALINA src/strutture/list.c src/main.c

binary=CENTRALINA binaries/CENPIDREAD binaries/MANUALE binaries/BULB binaries/FRIDGE binaries/WINDOW binaries/HUB binaries/TIMER

clean:
	rm -f $(binary)

hand:
	gcc -std=gnu90 -o binaries/MANUALE src/manuale/manuale.c
	binaries/MANUALE

exec: build
	./CENTRALINA