
find_file(WINDEPLOYOPENSSL_CRYPTO
	NAMES libcrypto-1_1-x64.dll
	PATHS ${OPENSSL_ROOT} ${OPENSSL_ROOT}/bin
	NO_DEFAULT_PATH)
find_file(WINDEPLOYOPENSSL_SSL
	NAMES libssl-1_1-x64.dll
	PATHS ${OPENSSL_ROOT} ${OPENSSL_ROOT}/bin
	NO_DEFAULT_PATH)

function(windeployopenssl directory)
	install(
		FILES ${WINDEPLOYOPENSSL_CRYPTO} ${WINDEPLOYOPENSSL_SSL}
		DESTINATION ${directory})
endfunction()

mark_as_advanced(WINDEPLOYOPENSSL_CRYPTO)
mark_as_advanced(WINDEPLOYOPENSSL_SSL)
