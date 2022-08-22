#
# Looking for OpenSSL
# 

function(kemai_find_openssl directory)

	message("Looking for OpenSSL in ${directory}")

	find_file(_WINDEPLOYOPENSSL_CRYPTO
		NAMES libcrypto-1_1-x64.dll
		PATHS ${directory} ${directory}/bin
		NO_DEFAULT_PATH)
	find_file(_WINDEPLOYOPENSSL_SSL
		NAMES libssl-1_1-x64.dll
		PATHS ${directory} ${directory}/bin
		NO_DEFAULT_PATH)

	if(_WINDEPLOYOPENSSL_CRYPTO)
		message("-- Found ${_WINDEPLOYOPENSSL_CRYPTO}")
		set(WINDEPLOYOPENSSL_CRYPTO ${_WINDEPLOYOPENSSL_CRYPTO} PARENT_SCOPE)
		mark_as_advanced(WINDEPLOYOPENSSL_CRYPTO)
	endif()

	if(_WINDEPLOYOPENSSL_SSL)
		message("-- Found ${_WINDEPLOYOPENSSL_SSL}")
		set(WINDEPLOYOPENSSL_SSL ${_WINDEPLOYOPENSSL_SSL} PARENT_SCOPE)
		mark_as_advanced(WINDEPLOYOPENSSL_SSL)
	endif()

endfunction()

function(windeployopenssl directory)
	install(
		FILES ${WINDEPLOYOPENSSL_CRYPTO} ${WINDEPLOYOPENSSL_SSL}
		DESTINATION ${directory})
endfunction()

# Try to find from user dir
if(OPENSSL_ROOT)
	kemai_find_openssl(${OPENSSL_ROOT})
endif()

# Try to find from github actions
if (NOT WINDEPLOYOPENSSL_CRYPTO)
	if ($ENV{IQTA_TOOLS})
		kemai_find_openssl($ENV{IQTA_TOOLS})
	endif()
endif()
