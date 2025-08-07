/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: varodrig <varodrig@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 10:53:55 by crios             #+#    #+#             */
/*   Updated: 2025/08/07 15:58:13 by varodrig         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"
#include "Channel.hpp"
#include <cassert>

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }
    
    int port = std::atoi(argv[1]);
    std::string password = argv[2];
    
    // Validation du port
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
