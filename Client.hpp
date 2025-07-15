/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 19:05:10 by crios             #+#    #+#             */
/*   Updated: 2025/07/15 14:28:25 by crios            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>

// Une classe client
class Client {
    private:
        int Fd; // File descriptor for the client
        std::string IP; // IP address of the client
        std::string Nickname; // Nickname of the client
    public:
        Client(){}; // Default constructor
        Client(int fd, const std::string &ip) : Fd(fd), IP(ip) {} // Parameterized constructor
        Client(const Client &other) : Fd(other.Fd), IP(other.IP) {} // Copy constructor
        Client& operator=(const Client &other) { // Assignment operator
            if (this != &other) {
                Fd = other.Fd;
                IP = other.IP;
            }
            return *this;
        }
        ~Client() {} // Destructor
        
        bool operator==(const Client &other) const { // Equality operator
            return Fd == other.Fd && IP == other.IP;
        }
        
        int getFd() const { return Fd; } // Getter for file descriptor
        void setFd(int fd) { Fd = fd; } // Setter for file descriptor
        void setIP(const std::string &ip) { IP = ip; } // Setter for IP address

        void setNickname(const std::string &nickname) { 
            Nickname = nickname; // Setter for nickname
            std::cout << "Client <" << Fd << "> Nickname Changed to: " << nickname << std::endl;
        }
        const std::string& getNickname() const { return Nickname; } // Getter for nickname

};

#endif