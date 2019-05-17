all: compilaCmd compilaMain compilaManuale

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

binaries=PRINCIPALE supporto/CENPIDREAD manuale/CENPIDWRITE componenti/BULB componenti/FRIDGE componenti/WINDOW componenti/HUB componenti/TIMER

clean:
	rm -f $(binaries)

hand:
	gcc -std=gnu90 -o manuale/CENPIDWRITE manuale/manuale.c
	manuale/CENPIDWRITE

exec: all
	./PRINCIPALE