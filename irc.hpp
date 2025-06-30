/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   irc.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 10:52:51 by crios             #+#    #+#             */
/*   Updated: 2025/06/30 12:25:46 by crios            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <poll.h>


// Une classe client
class Client {
    private:
        int Fd; // File descriptor for the client
        std::string IP; // IP address of the client
    public:
        Client(){}; // Default constructor
        int getFd() const { return Fd; } // Getter for file descriptor
        void setFd(int fd) { Fd = fd; } // Setter for file descriptor
        void setIP(const std::string &ip) { IP = ip; } // Setter for IP address
};

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

        static void SignalHandler(int signum); // Static method to handle signals

        void CloseFds(); // Method to close file descriptors
        void ClearClients(int fd); // Method to clear the list of clients
};

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
