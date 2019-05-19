define NEWLINE

endef

help:
	$(info Progetto SO realizzato da:Paolo Tasin, Stefano Perenzoni e Marcello Rigotti)
	$(NEWLINE)
	$(info Centralina per controllo domotico)


build: compilaCmd compilaMain compilaManuale

compilaCmd:
	gcc -g -std=gnu90 -o componenti/BULB componenti/bulb.c
	gcc -g -std=gnu90 -o componenti/FRIDGE componenti/fridge.c
	gcc -g -std=gnu90 -o componenti/WINDOW componenti/window.c
	gcc -g -std=gnu90 -o componenti/HUB componenti/hub.c
	gcc -g -std=gnu90 -o componenti/TIMER componenti/timer.c

compilaManuale:
	gcc -std=gnu90 -o supporto/CENPIDREAD supporto/getCenPid.c
	gcc -std=gnu90 -o manuale/CENPIDWRITE manuale/manuale.c

compilaMain:
	gcc -g -std=gnu90 -o PRINCIPALE strutture/list.c strutture/comandi.c main.c 

binaries=PRINCIPALE supporto/CENPIDREAD manuale/CENPIDWRITE componenti/BULB componenti/FRIDGE componenti/WINDOW componenti/HUB componenti/TIMER

clean:
	rm -f $(binaries)

hand:
	gcc -std=gnu90 -o manuale/CENPIDWRITE manuale/manuale.c
	manuale/CENPIDWRITE

exec: all
	./PRINCIPALE