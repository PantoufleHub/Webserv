#include "DirectiveKeys.hpp"
#include "Location.hpp"
#include "VirtualServer.hpp"

template <typename T, typename T2>
void SetDirectives(int key, T& value, T2& block) {
    Location location;
    EntryPoint entryPoint;

    switch (key) {
    case server_name:
        block.setNames(value);
        break;
    case location_config:
        location.setNames(value);
        ++value;
        location.setBlockDirective(value);
        location.inheritFromParent(static_cast<VirtualServer&>(block));
        static_cast<VirtualServer&>(block).setLocations(location);
        ++value;
        break;
    case listen_config:
        fillEntryPoint(value, entryPoint, static_cast<VirtualServer&>(block));
        break;
    case allow_method_config:
        block.setAllowedMethods(value);
        break;
    case index_config:
        block.setIndexes(value);
        break;
    case error_config:
        block.setErrors(value);
        break;
    case root_config:
        block.setRoot(value);
        break;
    case cgi_pass:
        ++value;
        break;
    case cgi_index:
        static_cast<Location&>(block).setCgi(value);
        break;
    case client_max_body_size:
        static_cast<Location&>(block).setClientMaxBodySize(value);
        break;
    case upload_store:
        block.setUploadStore(value);
        break;
    case autoindex:
        block.setAutoIndex(value);
        break;
    case redirection:
        block.setRedirect(value);
        break;
    default:
        break;
    }
}
