/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   irc.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 10:52:51 by crios             #+#    #+#             */
/*   Updated: 2025/07/17 12:52:14 by crios            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRC_HPP
#define IRC_HPP

#include "Client.hpp"
#include <iostream>
#include <vector>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <cstdlib>
#include <poll.h>
#include <sstream>
#include "Channel.hpp"

#define BUFFER_SIZE 1024

class Server {
    private:
        int Port;                           // Port d'écoute du serveur (ex: 6667, port IRC standard)
        int SerSocketFd;                    // Socket principal du serveur - "porte d'entrée" du serveur
        std::string serverPassword;         // Mot de passe requis pour se connecter au serveur
        static bool Signal;                 // Variable statique pour détecter les signaux (Ctrl+C). Static car accessible depuis SignalHandler
        std::vector<Client> clients;        // Tableau dynamique contenant tous les clients connectés au serveur
        std::vector<Channel> channels;      // Tableau des canaux IRC disponibles (#general, #random, etc.)
        std::vector<struct pollfd> fds;     // Structure utilisée par poll() pour surveiller plusieurs sockets simultanément (I/O multiplexé)
        
        Server(const Server& other);
        Server& operator=(const Server& other);
        
    public :
        Server(int port, const std::string& password) 
            : Port(port), SerSocketFd(-1), serverPassword(password) {}
            // fd = -1 car aucun socket n'a encore ete cree     
        ~Server();

        void ServerInit();
        bool checkPassword(const std::string& password) const;
        
        void SerSocket();
        void AcceptNewClient();
        void ReceiveNewData(int fd);
        void SendData(int fd);
        static void SignalHandler(int signum);
        // static car la fonction signal() ou sigaction() (pour gérer les signaux comme CTRL+C)
        // attend une fonction C libre (non liée à une instance de classe).
        void CloseFds();
        void parseCommand(const std::string& message, int fd);
        void ClearClients(int fd);

        void handlePass(Client* client, std::istringstream& iss); // Handle PASS command
        void handleNick(Client* client, std::istringstream& iss); // Handle NICK command
        void handleUser(Client* client, std::istringstream& iss); // Handle USER command
        void handleQuit(Client* client, std::istringstream& iss); // Handle QUIT command
        void handlePart(Client* client, std::istringstream& iss); // Handle PART command
        void handleTopic(Client* client, std::istringstream& iss); // Handle TOPIC command
        void handlePrivmsg(Client* client, std::istringstream& iss); // Handle PRIVMSG command
        void handleKick(Client* client, std::istringstream& iss); // Handle KICK command
        void handleMode(Client* client, std::istringstream& iss); // Handle MODE command
        void handleInvite(Client* client, std::istringstream& iss); // Handle INVITE command

        // Helper methods
        void sendIRCReply(int fd, const std::string& message); // Method to send IRC reply to a client
        void sendWelcome(Client* client); // Method to send welcome message to clients
        void checkRegistration(Client* client); // Method to check if a client is fully registered
        Client* getClientByFd(int fd); // Method to get a client by file descriptor
        // Instructions for clients
        void sendLoginInstructions(int fd); // Send login instructions to a client
        void sendWelcomeMessage(int fd); // Send welcome message to a client

        // Channel
        void handleJoin(Client* client, std::istringstream& iss); // Handle JOIN command
        void sendChannelInfo(Client* client, Channel* channel); // Send channel info to a client
        Channel* findChannel(const std::string& name); // Find a channel by name
        Channel* createChannel(const std::string& name, Client* creator); // Create a new channel
        void addClientToChannel(Client* client, Channel* channel); // Add a client to a channel
        void notifyChannelJoin(Client* client, Channel* channel); // Notify channel members of a new join        
};

#endif