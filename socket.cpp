/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 13:02:42 by crios             #+#    #+#             */
/*   Updated: 2025/08/07 17:04:01 by crios            ###   ########.fr       */
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
    
    if (fcntl(connectedFd, F_SETFL, O_NONBLOCK) < 0) {
        std::cerr << "Error setting client socket to non-blocking" << std::endl;
        close(connectedFd);
        return;
    }
    
    // Create and add new client to the clients vector
    Client newClient;
    newClient.setFd(connectedFd);
    newClient.setIP(inet_ntoa(clientAddr.sin_addr));

    // ✅ NE PAS définir username/nickname automatiquement !
    // Laissez vides jusqu'à ce que le client les définisse
    newClient.setUsername("");        // Vide jusqu'à USER
    newClient.setNickname("");        // Vide jusqu'à NICK
    newClient.setAuthenticated(false); // Pas encore authentifié
    newClient.setRegistered(false);    // Pas encore enregistré
    
    clients.push_back(newClient);
    std::cout << "New client connected with fd: " << connectedFd << std::endl;
    
    // Add client socket to poll structure
    struct pollfd newPoll;
    newPoll.fd = connectedFd;
    newPoll.events = POLLIN;
    newPoll.revents = 0;
    fds.push_back(newPoll);
    
    // Send welcome message to the NEW CLIENT (not server socket)
    SendData(connectedFd);
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
    std::cout << "Received " << bytesRead << " bytes from client fd:" << fd << std::endl;
    buffer[bytesRead] = '\0'; // -> Null-terminate the received data
    
    if (client) {
        // Ajouter les données reçues au buffer du client
        client->addToBuffer(std::string(buffer));
        
        // Traiter toutes les commandes complètes dans le buffer
        std::string completeCommand;
        while (client->extractCommand(completeCommand)) {
            parseCommand(completeCommand, fd);
        }
    } else {
        std::cerr << "Error: Client with fd " << fd << " not found in clients list" << std::endl;
        parseCommand(std::string(buffer), fd);
    }
}

void Server::SendData(int fd)
{
    const char message[] = 
        "Welcome to the IRC server!\n"
        "To log in, follow these steps:\n"
        "1. Authenticate with: PASS <password>\n"
        "2. Set your nickname: NICK <nickname>\n"
        "3. Set your username: USER <username> 0 * :<realname>\n"
        "4. Join a channel: JOIN #<channel>\n";
    ssize_t bytesSent = send(fd, message, sizeof(message), 0); // -> Send data to the client
    if (bytesSent < 0) // -> Check if the send call failed
    {
        std::cerr << "Error sending data to client <" << fd << ">" << std::endl;
        return;
    }
}

void Server::ServerInit() {
    SerSocket();

    std::cout << "Server is running on port: " << Port << std::endl;
    std::cout << "Waiting to accept a connection..." << std::endl;
    
    while (!Server::Signal) {
        // Timeout de 1 seconde au lieu de -1 (infini)
        if (poll(&fds[0], fds.size(), 1000) < 0 && !Server::Signal)
            throw(std::runtime_error("poll() failed"));
        
        for (size_t i = 0; i < fds.size(); i++) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == SerSocketFd)
                    AcceptNewClient();
                else
                    ReceiveNewData(fds[i].fd);
            }
        }
    }
}


void Server::ClearClients(int fd) {
    Client* disconnectedClient = getClientByFd(fd);
    
    if (disconnectedClient) {
        // Notifier tous les canaux MAIS vérifier chaque fd avant d'envoyer
        for (size_t i = 0; i < channels.size(); i++) {
            if (channels[i].isClientInChannel(fd)) {
                const std::vector<int>& clientFds = channels[i].getClientFds();
                for (size_t j = 0; j < clientFds.size(); j++) {
                    if (clientFds[j] != fd) {
                        // Envoyer directement sans validation poll() - l'erreur sera gérée automatiquement
                        sendIRCReply(clientFds[j], ":" + disconnectedClient->getNickname() + "!" + disconnectedClient->getUsername() + "@localhost QUIT :Client disconnected");
                    }
                }
                channels[i].removeClient(fd);
            }
        }
    }
    
    // Retirer de pollfd
    for (size_t i = 0; i < fds.size(); i++) {
        if (fds[i].fd == fd) {
            fds.erase(fds.begin() + i);
            break;
        }
    }
    
    // Retirer de clients
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getFd() == fd) {
            clients.erase(clients.begin() + i);
            break;
        }
    }
    
    close(fd);
    std::cout << "Client " << fd << " properly removed from server" << std::endl;
}
