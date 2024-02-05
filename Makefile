all:
	mkdir bin
	gcc -o bin/krun-sock-cli-test test/vsock_client.c
	gcc -o bin/krun-run test/krun.c -lkrun
	gcc -o bin/krun-sock-server-test unix_sock.c
	gcc -o bin/unix-cli test/unix_client.c
	cp -r test/fedora-test bin

clean:
	rm -rf bin
	
