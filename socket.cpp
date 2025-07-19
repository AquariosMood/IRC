/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 13:02:42 by crios             #+#    #+#             */
/*   Updated: 2025/07/16 16:44:27 by crios            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"

bool Server::Signal = false; // Initialize static variable


void Server::SignalHandler(int signum)
{
    (void)signum; // On le cast vers (void) car on n'en a pas besoin
    std::cout << std::endl << "Signal Received!" << std::endl;
    Server::Signal = true;
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

// 1. Le serveur a 1 socket d'écoute, mais N sockets clients (un par client).
// 2. accept() crée un nouveau socket dédié à chaque client.
// 3. poll() surveille tous les sockets (serveur + clients) pour détecter l'activité.
// 4. Mode non-bloquant (O_NONBLOCK) permet de gérer plusieurs clients sans bloquer.
void Server::SerSocket()
{
    struct sockaddr_in add; // -> Structure pour stocker l'adresse et le port du serveur
    
    add.sin_family = AF_INET; // Protocole réseau à utiliser : IPv4
    add.sin_port = htons(this->Port); // Host TO Network Short
    add.sin_addr.s_addr = INADDR_ANY;  // 0.0.0.0

    /*
    SOCK_STREAM = Appel téléphonique
    - Connexion établie avant de parler
    - Conversation continue
    - Ordre des mots respecté
    - Si coupure, on le sait

    SOCK_DGRAM = SMS
    - Chaque message indépendant
    - Peut arriver dans le désordre
    - Peut être perdu
    - Plus rapide
    */
    // 0 = protocole par défaut (TCP pour SOCK_STREAM)
    SerSocketFd = socket(AF_INET, SOCK_STREAM, 0); // Crée le socket du serveur
    if (SerSocketFd < 0)
    {
        std::cerr << "Error creating socket" << std::endl;
        exit(EXIT_FAILURE);
    }
    int en = 1; // en : enable

    // setsockopt() : socket options
    // SO_REUSEADDR : permet de réutiliser l'adresse immédiatement après la fermeture du socket
    // runtime_error = Erreurs à l'exécution
    if (setsockopt(SerSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) < 0) 
        throw(std::runtime_error("failed to set option (SO_REUSEADDR) on socket"));
    if (fcntl(SerSocketFd, F_SETFL, O_NONBLOCK) < 0)
        throw(std::runtime_error("failed to set socket to non-blocking mode"));
    // sinon se retrouve bloqué sur accept() si le client n'est pas connecté

    // bind() : Associe le socket à une adresse réseau spécifique (IP + port)
    if (bind(SerSocketFd, (struct sockaddr *)&add, sizeof(add)) < 0)
        throw(std::runtime_error("failed to bind socket"));
    // listen() : Met le socket en mode écoute
    if (listen(SerSocketFd, SOMAXCONN) < 0)
        throw(std::runtime_error("failed to listen on socket"));
    
    // Configuration du système de poll() pour surveiller les connexions.
    struct pollfd NewPoll
    NewPoll.fd = SerSocketFd; // Indique à poll() de surveiller le socket serveur
    NewPoll.events = POLLIN; // Surveille les données entrantes
    NewPoll.revents = 0; // toujours mettre a 0 avant d'utiliser poll(), sera rempli par poll() avec les événements qui se sont produits
    fds.push_back(NewPoll); // Ajoute le socket serveur à la liste des sockets à surveiller
    // fds.size() = 1
}

void Server::AcceptNewClient()
{
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    // accept() : cree un nouveau socket pour le client
    int connectedFd = accept(SerSocketFd, (struct sockaddr*) &clientAddr, &clientLen);
    
    if (connectedFd < 0) {
        std::cerr << "Error accepting new client" << std::endl;
        return;
    }

    Client newClient;
    newClient.setFd(connectedFd);
    newClient.setIP(inet_ntoa(clientAddr.sin_addr)); // (0x7F000001 -> 127.0.0.1) Internet Network TO ASCII

    std::ostringstream userStream; // ecrit dans une string
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
    SendData(connectedFd);
}

void Server::ReceiveNewData(int fd)
{
    Client *client = NULL;
    
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getFd() == fd)
        {
            client = &clients[i];
            break;
        }
    }
    
    char buffer[BUFFER_SIZE] = {0}; // -> Buffer to store incoming data
    ssize_t bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead < 0) // connection interrupted or error
    {
        std::cerr << "Error receiving data from client <" << fd << ">" << std::endl;
        return;
    }
    else if (bytesRead == 0) // client disconnected
    {
        if (client) {
            std::cout << client->getUsername() << "> Disconnected" << std::endl;
        } else {
            std::cout << "Client <" << fd << "> Disconnected" << std::endl;
        }
        ClearClients(fd);
        return;
    }

    buffer[bytesRead] = '\0'; // -> Null-terminate the received data
    // Check if client exists before using it
    if (client) {
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
    const char message[] = 
        "Welcome to the IRC server!\n"
        "To log in, follow these steps:\n"
        "1. Authenticate with: PASS <password>\n"
        "2. Set your nickname: NICK <nickname>\n"
        "3. Set your username: USER <username> 0 * :<realname>\n"
        "4. Join a channel: JOIN #<channel>\n";
    ssize_t bytesSent = send(fd, message, sizeof(message), 0); // 0 : comportement par défaut
    if (bytesSent < 0)
    {
        std::cerr << "Error sending data to client <" << fd << ">" << std::endl;
        return;
    }
}

void Server::ServerInit()
{
    SerSocket(); // Initialise le socket du serveur

    std::cout << "Server is running on port: " << Port << std::endl;
    std::cout << "Waiting to accept a connection..." << std::endl;
    while (!Server::Signal)
    {
        if (poll(&fds[0], fds.size(), -1) < 0 && !Server::Signal)
            throw(std::runtime_error("poll() failed"));
        
        for (size_t i = 0; i < fds.size(); i++)
        {
            // revents = 0x005 (POLLIN | POLLERR)
            // On veut détecter si POLLIN est présent, même si d'autres bits sont activés.
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == SerSocketFd)
                    AcceptNewClient();
                else
                    ReceiveNewData(fds[i].fd);
            }
        }
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