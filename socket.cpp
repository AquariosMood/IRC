/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 13:02:42 by crios             #+#    #+#             */
/*   Updated: 2025/06/30 13:25:16 by crios            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"

bool Server::Signal = false; // Initialize static variable


// Method to handle signals
void Server::SignalHandler(int signum)
{
    (void)signum; // Suppress unused parameter warning
    std::cout << std::endl << "Signal Received!" << std::endl;
    Server::Signal = true; // Set the static signal variable to true
}

// Method to close all file descriptors
void Server::CloseFds()
{
    for(size_t i = 0; i < clients.size(); i++){ //-> close all the clients
		std::cout << "Client <" << clients[i].getFd() << "> Disconnected" << std::endl;
		close(clients[i].getFd());
	}
    if (SerSocketFd != -1) //-> close the server socket if it is open
    {
        std::cout << "Server Socket <" << SerSocketFd << "> Closed" << std::endl;
        close(SerSocketFd);
    }
}

void Server::SerSocket()
{
    struct sockaddr_in add; // -> Create a sockaddr_in structure for the server address
    struct pollfd NewPoll; // -> Create a pollfd structure for polling
    
    add.sin_family = AF_INET; // -> Set the address family to IPv4
    add.sin_port = htons(this->Port); // -> Set the port number, converting to network byte order
    add.sin_addr.s_addr = INADDR_ANY; // -> Set the address to any available interface
    
    SerSocketFd = socket(AF_INET, SOCK_STREAM, 0); // -> Create a socket
    if (SerSocketFd < 0) // -> Check if the socket creation failed
    {
        std::cerr << "Error creating socket" << std::endl;
        exit(EXIT_FAILURE);
    }
    int en = 1; // -> Set the socket option to reuse the address

    if (setsockopt(SerSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) < 0) 
        throw(std::runtime_error("failed to set option (SO_REUSEADDR) on socket"));
    if (fcntl(SerSocketFd, F_SETFL, O_NONBLOCK) < 0) // -> Set the socket to non-blocking mode
        throw(std::runtime_error("failed to set socket to non-blocking mode"));
    if (bind(SerSocketFd, (struct sockaddr *)&add, sizeof(add)) < 0) // -> Bind the socket to the address
        throw(std::runtime_error("failed to bind socket"));
    if (listen(SerSocketFd, SOMAXCONN) < 0) // -> Listen for incoming connections
        throw(std::runtime_error("failed to listen on socket"));
    
    NewPoll.fd = SerSocketFd; // -> Add the server socket to the poll file descriptors structure
    NewPoll.events = POLLIN; // -> Set the events to poll for incoming connections
    NewPoll.revents = 0; // -> Initialize revents to 0
    fds.push_back(NewPoll); // -> Add the server socket to the vector of poll file descriptors
}

void Server::ServerInit()
{
    this->Port = 888; // -> Set the default port number for the server
    SerSocket(); // -> Create the server socket

    std::cout << "Server <" << SerSocketFd << "> Listening on Port <" << Port << ">" << std::endl;
    std::cout << "Waiting to accept a connection...\n" << std::endl;
}
