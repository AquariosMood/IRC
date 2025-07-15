/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 13:02:42 by crios             #+#    #+#             */
/*   Updated: 2025/07/15 15:37:50 by crios            ###   ########.fr       */
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
    
    // Fonction principale - Voir le NOTION
    SerSocketFd = socket(AF_INET, SOCK_STREAM, 0); // -> Create a socket
    if (SerSocketFd < 0) // -> Check if the socket creation failed
    {
        std::cerr << "Error creating socket" << std::endl;
        exit(EXIT_FAILURE);
    }
    int en = 1; // -> Set the socket option to reuse the address

    // Set socket options - Je ne sais pas vraiment à quoi ça sert et si c'est nécessaire
    if (setsockopt(SerSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) < 0) 
        throw(std::runtime_error("failed to set option (SO_REUSEADDR) on socket"));
    if (fcntl(SerSocketFd, F_SETFL, O_NONBLOCK) < 0) // -> Set the socket to non-blocking mode
        throw(std::runtime_error("failed to set socket to non-blocking mode"));

    // Bind() et listen() - Voir le NOTION
    if (bind(SerSocketFd, (struct sockaddr *)&add, sizeof(add)) < 0) // -> Bind the socket to the address
        throw(std::runtime_error("failed to bind socket"));
    if (listen(SerSocketFd, SOMAXCONN) < 0) // -> Listen for incoming connections
        throw(std::runtime_error("failed to listen on socket"));
    
    // Server socket is successfully created and bound and now ready to accept connections

    
    NewPoll.fd = SerSocketFd; // -> Add the server socket to the poll file descriptors structure
    NewPoll.events = POLLIN; // -> Set the events to poll for incoming connections
    NewPoll.revents = 0; // -> Initialize revents to 0
    fds.push_back(NewPoll); // -> Add the server socket to the vector of poll file descriptors
}

void Server::AcceptNewClient()
{
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int connectedFd = accept(SerSocketFd, (struct sockaddr*) &clientAddr, &clientLen);
    
    if (connectedFd < 0) {
        std::cerr << "Error accepting new client" << std::endl;
        return;
    }
    
    // Create and add new client to the clients vector
    Client newClient;
    newClient.setFd(connectedFd);
    newClient.setIP(inet_ntoa(clientAddr.sin_addr));

    std::ostringstream userStream;
    userStream << "User" << connectedFd;
    newClient.setUsername(userStream.str());
    
    std::ostringstream nickStream;
    nickStream << "Guest" << connectedFd;
    newClient.setNickname(nickStream.str());
    
    clients.push_back(newClient);
    std::cout << "New client connected: " << newClient.getUsername() 
              << " (nickname: " << newClient.getNickname() << ")" << std::endl;
    
    // Add client socket to poll structure
    struct pollfd newPoll;
    newPoll.fd = connectedFd;
    newPoll.events = POLLIN;
    newPoll.revents = 0;
    fds.push_back(newPoll);
    
    // Send welcome message to the NEW CLIENT (not server socket)
    SendData(connectedFd);  // <- Fixed: using client FD instead of server FD
}

void Server::ReceiveNewData(int fd)
{
    Client *client = NULL; // -> Pointer to hold the client object
    
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getFd() == fd)
        {
            client = &clients[i];
            break;
        }
    }
    
    char buffer[BUFFER_SIZE] = {0}; // -> Buffer to store incoming data
    ssize_t bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0); // -> Receive data from the client

    // Verify if the receive call was successful
    if (bytesRead < 0) // -> Check if the receive call failed
    {
        std::cerr << "Error receiving data from client <" << fd << ">" << std::endl;
        return;
    }
    else if (bytesRead == 0) // -> Check if the client has disconnected
    {
        if (client) {
            std::cout << client->getUsername() << "> Disconnected" << std::endl;
        } else {
            std::cout << "Client <" << fd << "> Disconnected" << std::endl;
        }
        ClearClients(fd); // -> Clear the client from the list
        return;
    }

    buffer[bytesRead] = '\0'; // -> Null-terminate the received data
    
    // Check if client exists before using it
    if (client) {
        std::cout << client->getUsername() << ": " << buffer << std::endl;
        // Parse the received command
        parseCommand(std::string(buffer), fd);
    } else {
        std::cerr << "Error: Client with fd " << fd << " not found in clients list" << std::endl;
        std::cout << "Unknown client <" << fd << ">: " << buffer << std::endl;
        // Still parse the command, but without client context
        parseCommand(std::string(buffer), fd);
    }
}

void Server::SendData(int fd)
{
    const char message[] = "Hello from the server!\n";
    ssize_t bytesSent = send(fd, message, sizeof(message), 0); // -> Send data to the client
    if (bytesSent < 0) // -> Check if the send call failed
    {
        std::cerr << "Error sending data to client <" << fd << ">" << std::endl;
        return;
    }
}

void Server::ServerInit()
{
    this->Port = 8888; // -> Set the default port number for the server
    SerSocket(); // -> Create the server socket

    std::cout << "Waiting to accept a connection..." << std::endl;
    
     // Je ne comprends pas encore comment fonctionne poll(), je voulais surtout appeler la fonction AcceptNewClient() quand un client se connecte
    while (!Server::Signal)
    {
        if (poll(&fds[0], fds.size(), -1) < 0 && !Server::Signal)
            throw(std::runtime_error("poll() failed"));
        
        for (size_t i = 0; i < fds.size(); i++)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == SerSocketFd)
                    AcceptNewClient(); // -> Accept a new client connection
                else
                    ReceiveNewData(fds[i].fd); // -> Receive data from the client
            }
        }
    }
}

void Server::parseCommand(const std::string& message, int fd)
{
  std::istringstream iss(message);
    std::string command;
    iss >> command;
    
    // Find the client with the matching file descriptor
    Client* client = NULL;
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getFd() == fd) {
            client = &clients[i];
            break;
        }
    }
    
    // Check if client was found
    if (!client) {
        std::cerr << "Error: Client with fd " << fd << " not found" << std::endl;
        return;
    }
    
    if (command == "NICK") {
        std::string nickname;
        iss >> nickname; // Extract the nickname properly
        client->setNickname(nickname); // Use proper method name
        std::cout << "Nickname set to: " << client->getNickname() << std::endl;
    } else if (command == "JOIN") {
        std::cout << "JOIN command received" << std::endl;
    } else if (command == "PRIVMSG") {
        std::cout << "PRIVMSG command received" << std::endl;
    }
}

void Server::ClearClients(int fd){ //-> clear the clients
	for(size_t i = 0; i < fds.size(); i++){ //-> remove the client from the pollfd
		if (fds[i].fd == fd)
			{fds.erase(fds.begin() + i); break;}
	}
	for(size_t i = 0; i < clients.size(); i++){ //-> remove the client from the vector of clients
		if (clients[i].getFd() == fd)
			{clients.erase(clients.begin() + i); break;}
	}
}
