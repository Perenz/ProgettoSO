all: compilaCmd compilaMain

compilaCmd:
	gcc -o componenti/BULB componenti/bulb.c
	gcc -o componenti/FRIDGE componenti/fridge.c
	gcc -o componenti/WINDOW componenti/window.c
	gcc -o componenti/HUB componenti/hub.c
	gcc -o componenti/TIMER componenti/timer.c

compilaMain:
	gcc -o PRINCIPALE strutture/list.c main.c

binaries=BULB PRINCIPALE

clean:
	rm -f $(binaries)