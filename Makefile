all:
	gcc -Wall -Wextra -pedantic -std=c11 -o bin/humiture humiture.c -lwiringPi

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

shellcheck:
	shellcheck bin/create-graphs.sh
	shellcheck bin/gather-data.sh
