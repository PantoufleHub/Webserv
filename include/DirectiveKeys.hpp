#pragma once

// Directive bitmask values used during configuration parsing.
// Names match the historical code so the parsing logic can be reused without edits.

#define autoindex 4096
#define redirection 2048
#define upload_store 1024
#define server_name 512
#define listen_config 256
#define location_config 128
#define root_config 64
#define cgi_pass 32
#define index_config 16
#define error_config 8
#define allow_method_config 4
#define client_max_body_size 1
