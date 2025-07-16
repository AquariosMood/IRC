/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   irc.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 10:52:51 by crios             #+#    #+#             */
/*   Updated: 2025/07/16 13:31:42 by crios            ###   ########.fr       */
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

#define BUFFER_SIZE 1024 // Define a buffer size for receiving data

// Une classe server
class Server {
    private:
        int Port; // Port number for the server
        int SerSocketFd; // Socket file descriptor for the server
        std::string serverPassword; // Password for the server
        static bool Signal; // Static variable to handle signals
        std::vector<Client> clients; // Vector to store connected clients
        std::vector<Channel> channels; // Vector to store channels
        std::vector<struct pollfd> fds; // Vector for poll file descriptors
    public:
        Server(int port, const std::string& password) 
            : Port(port), SerSocketFd(-1), serverPassword(password) {}

        void ServerInit(); // Method to initialize the server
        bool checkPassword(const std::string& password) const; // Method to check server password
        
        void SerSocket(); // Method to create the server socket
        void AcceptNewClient(); // Method to accept new clients
        void ReceiveNewData(int fd); // Method to receive data from clients
        void SendData(int fd); // Method to send data to clients
        static void SignalHandler(int signum); // Static method to handle signals
        void CloseFds(); // Method to close file descriptors
        void parseCommand(const std::string& message, int fd); // Method to parse commands from clients
        void ClearClients(int fd); // Method to clear the list of clients

        // Authentication methods
        void handlePass(Client* client, std::istringstream& iss); // Handle PASS command
        void handleNick(Client* client, std::istringstream& iss); // Handle NICK command
        void handleUser(Client* client, std::istringstream& iss); // Handle USER command
        void handleQuit(Client* client, std::istringstream& iss); // Handle QUIT command

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
        void handlePrivmsg(Client* client, std::istringstream& iss); // Handle PRIVMSG command
        
};

#endif