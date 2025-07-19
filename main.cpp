/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 10:53:55 by crios             #+#    #+#             */
/*   Updated: 2025/07/15 15:49:22 by crios            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"
#include "Channel.hpp"
#include <cassert>

int main(int argc, char **argv)
{
    // ./ircserv <port> <password>
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }
    
    int port = std::atoi(argv[1]);
    std::string password = argv[2];
    
    // Les ports < 1024 sont réservés au système (ports privilégiés)
    // Les ports > 65535 n'existent pas (limite des ports TCP/UDP)
    if (port < 1024 || port > 65535) {
        std::cerr << "Error: Port must be between 1024-65535" << std::endl;
        return 1;
    }
    
    if (password.empty()) {
        std::cerr << "Error: Password cannot be empty" << std::endl;
        return 1;
    }
    
    Server ser(port, password);
    
    std::cout << "---- SERVER ----" << std::endl;
    try {
        signal(SIGINT, Server::SignalHandler);
        signal(SIGQUIT, Server::SignalHandler);
        ser.ServerInit();
    }
    catch(const std::exception& e) {
        ser.CloseFds();
        std::cerr << e.what() << std::endl;
        return 1; // Return error code
    }
    std::cout << "The Server Closed!" << std::endl;
    return 0; // Add this return statement
}
