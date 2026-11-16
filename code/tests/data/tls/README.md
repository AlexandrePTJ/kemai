Test-only TLS material for `tst_certificateTrust.cpp` (local `QSslServer`): private keys published on purpose, never
used anywhere else. `selfsigned` and `other` are distinct self-signed certificates for localhost / 127.0.0.1,
`wronghost` is issued for other.example, `expired` was valid in January 2020 only. Regenerate with
`openssl req -x509 -newkey rsa:2048 -nodes …` (see git history of this folder).
