@echo off

docker run -d ^
-p 8001:8001 ^
--name kimai2 ^
-v kimai_data:/opt/kimai/var/data ^
kimai/kimai2:apache-debian-master-prod

docker exec kimai2 ^
/opt/kimai/bin/console kimai:create-user admin admin@example.com ROLE_SUPER_ADMIN adminsecret
