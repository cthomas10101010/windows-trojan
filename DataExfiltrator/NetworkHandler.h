#ifndef NETWORKHANDLER_H
#define NETWORKHANDLER_H

#include <string>

namespace NetworkHandler {
    bool ExfiltrateData(const std::string& data, const std::string& ip, int port);
}

#endif // NETWORKHANDLER_H
