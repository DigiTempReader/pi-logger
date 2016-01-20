all:
	gcc -Wall -ansi -std=c11 -pedantic -D_GNU_SOURCE -o bin/humiture humiture.c -lwiringPi

clean:
	rm -f bin/humiture

install:
	cp -v systemd/* /etc/systemd/system/
	systemctl daemon-reload

	systemctl start pi-logger-gatherer.service
	systemctl start pi-logger-gatherer.timer
	systemctl enable pi-logger-gatherer.timer

	systemctl start pi-logger-create-graphs.service
	systemctl start pi-logger-create-graphs.timer
	systemctl enable pi-logger-create-graphs.timer
