# The Make creates the self signed certificates using openssl. 

server_certificate:client_certificate 

	@openssl genrsa -out server.key 4096

	@echo --------------------------------------------------
	@echo "Generating server Certificate"
	@echo --------------------------------------------------
	
	@openssl req -new -key server.key -out server.csr -subj "/C=US/ST=Illinois/L=Chicago/O=Sans Pareil Technologies, Inc./OU=OSS/CN=config-db/emailAddress=support@sptci.com/"

	@openssl x509 -req -days 7300 -sha512 -extensions v3_req -CA ca.crt -CAkey ca.key -CAcreateserial -in server.csr -out server.crt

	@make clean_temp

	@echo --------------------------------------------------
	@echo "Server Certificate generated Successfully"
	@echo --------------------------------------------------
	
client_certificate:root_certificate
	
	@openssl genrsa -out client.key 4096

	@echo --------------------------------------------------
	@echo "Generating client Certificate"
	@echo --------------------------------------------------
	
	@openssl req -new -key client.key -out client.csr -subj "/C=US/ST=Illinois/L=Chicago/O=Sans Pareil Technologies, Inc./OU=OSS/CN=config-db-client/emailAddress=support@sptci.com/"

	@openssl x509 -req -days 7300 -sha512 -extensions v3_req -CA ca.crt -CAkey ca.key -CAcreateserial -in client.csr -out client.crt

	@echo --------------------------------------------------
	@echo "client Certificate generated Successfully"
	@echo --------------------------------------------------

root_certificate:clean
	
	@openssl genrsa -des3 -out ca.key 4096

	@echo --------------------------------------------------
	@echo "Generating root Certificate"
	@echo --------------------------------------------------
		
	@openssl req -new -key ca.key -out ca.csr -subj "/C=US/ST=Illinois/L=Chicago/O=Sans Pareil Technologies, Inc./OU=OSS/CN=sptci.com/emailAddress=support@sptci.com/"

	@openssl x509 -req -days 7300 -sha512 -extensions v3_ca -signkey ca.key -in ca.csr -out ca.crt

	@echo --------------------------------------------------
	@echo "CA/root Certificate generated Successfully"
	@echo --------------------------------------------------

clean:clean_temp
	@echo --------------------------------------------------
	@echo "Started cleaning of folders....		"
	@echo --------------------------------------------------

	@rm -rf root/
	@rm -rf server/
	@rm -rf client/
	@echo --------------------------------------------------
	@echo "Cleaning Completed.		"
	@echo --------------------------------------------------

clean_temp:
	@echo --------------------------------------------------
	@echo "Started cleaning of temp files....		"
	@echo --------------------------------------------------

	@rm -rf *.csr
	#@rm -rf *.key
	#@rm -rf *.crt
	@rm -rf *.srl
	@echo --------------------------------------------------
	@echo "Cleaning Completed for temp Files.		"
	@echo --------------------------------------------------
