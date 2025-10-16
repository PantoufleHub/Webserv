#pragma once

#include <map>
#include <string>

using namespace std;

// HTTP constants reused by the config parser and future server modules.
static const string HTTP_VERSION = "HTTP/1.1";

static const string METHOD_GET = "GET";
static const string METHOD_POST = "POST";
static const string METHOD_DELETE = "DELETE";
static const string METHOD_NONE = "none";

const string HEADER_CONTENT_TYPE = "Content-Type";
const string HEADER_CONTENT_LENGTH = "Content-Length";
const string HEADER_CONNECTION = "Connection";
const string HEADER_HOST = "Host";
const string HEADER_ACCEPT = "Accept";
const string HEADER_TRANSFER_ENCODING = "Transfer-Encoding";
const string HEADER_LOCATION = "Location";

const string TYPE_HTML = "text/html";
const string TYPE_PLAIN = "text/plain";
const string TYPE_PNG = "image/png";
const string TYPE_JPG = "image/jpg";

const string ENCODING_CHUNKED = "chunked";

const int HTTP_CODE_OK = 200;
const int HTTP_CODE_CREATED = 201;
const int HTTP_CODE_NO_CONTENT = 204;
const int HTTP_CODE_MOVED_PERMANENTLY = 301;
const int HTTP_CODE_FOUND = 302;
const int HTTP_CODE_BAD_REQUEST = 400;
const int HTTP_CODE_FORBIDDEN = 403;
const int HTTP_CODE_NOT_FOUND = 404;
const int HTTP_CODE_METHOD_NOT_ALLOWED = 405;
const int HTTP_CODE_REQUEST_TIMEOUT = 408;
const int HTTP_CODE_CONFLICT = 409;
const int HTTP_CODE_LENGTH_REQUIRED = 411;
const int HTTP_CODE_PAYLOAD_TOO_LARGE = 413;
const int HTTP_CODE_URI_TOO_LONG = 414;
const int HTTP_CODE_UNSUPPORTED_MEDIA = 415;
const int HTTP_CODE_TOO_MANY_REQUESTS = 429;
const int HTTP_CODE_INTERNAL_SERVER_ERROR = 500;
const int HTTP_CODE_NOT_IMPLEMENTED = 501;
const int HTTP_CODE_BAD_GATEWAY = 502;
const int HTTP_CODE_SERVICE_UNAVAILABLE = 503;
const int HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED = 505;

const size_t REQUEST_EOF = 4; // \r\n\r\n length

// Server limits
const size_t CONTENT_LENGTH_SIZE = 16;
const size_t MAX_REQUEST_LENGTH = 1048576;

// Config file constants
static const std::string DEFAULT_UPLOAD_STORE = "/misc/var/uploads/";

const std::map<std::string, int>& getDirectivesMap();

// Log file constants
const string LOG_FOLDER = "misc/logs/";
const string CONN_LOG_FILE = LOG_FOLDER + "connections.log";
const string REQ_LOG_FILE = LOG_FOLDER + "requests.log";
const string RESP_LOG_FILE = LOG_FOLDER + "responses.log";
const string ERROR_LOG_FILE = LOG_FOLDER + "errors.log";

