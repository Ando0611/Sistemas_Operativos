pull: 
	docker pull rockylinux:latest

run-without-user-db-port:
	docker run --name rockylinux-http-server -d rockylinux:latest

run without-persistence:
	docker run --name rockylinux-http-server \
	-e HTTP_SERVER_USERNAME=admin \
	-e HTTP_SERVER_PASSWORD=admin \
	-p 8080:8080 \
	-d rockylinux:latest


run-with-persistence:
	docker run --name rockylinux-http-server \
	-e HTTP_SERVER_USERNAME=admin \
	-e HTTP_SERVER_PASSWORD=admin \
	-v pgdata:/var/lib/postgresql/data \
	-p 8080:8080 \
	-d rockylinux:latest

check-docker-volumes:
	docker volume ls

connect-to-rockylinux-http-server-macos:
	pgcli --host localhost -U super_user -d mydatabase
	