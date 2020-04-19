all:mdns

mdns:mdns.c
	gcc $< -o $@ -O2 -luv
