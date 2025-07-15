/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   irc.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 10:52:51 by crios             #+#    #+#             */
/*   Updated: 2025/07/15 14:21:08 by crios            ###   ########.fr       */
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
        static bool Signal; // Static variable to handle signals
        std::vector<Client> clients; // Vector to store connected clients
        std::vector<struct pollfd> fds; // Vector for poll file descriptors
    public:
        Server(){SerSocketFd = -1;}; // Default constructor initializes SerSocketFd to -1

        void ServerInit(); // Method to initialize the server
        void SerSocket(); // Method to create the server socket
        void AcceptNewClient(); // Method to accept new clients
        void ReceiveNewData(int fd); // Method to receive data from clients
        void SendData(int fd); // Method to send data to clients

        static void SignalHandler(int signum); // Static method to handle signals

        void CloseFds(); // Method to close file descriptors

        void parseCommand(const std::string& message, int fd); // Method to parse commands from clients
        
        void ClearClients(int fd); // Method to clear the list of clients
};

#endif