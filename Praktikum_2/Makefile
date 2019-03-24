all: udp_srv udp_cli  

udp_srv: udp_srv.c
	gcc -o udp_srv udp_srv.c

udp_cli: udp_cli.c
	gcc -o udp_cli udp_cli.c

clean:
	rm udp_srv udp_cli
