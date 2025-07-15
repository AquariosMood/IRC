/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 19:05:10 by crios             #+#    #+#             */
/*   Updated: 2025/07/15 15:34:52 by crios            ###   ########.fr       */
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
        std::string Username; // Username of the client (optional)
    public:
        Client() : Fd(-1), IP(""), Nickname("*"), Username("*") {} // Default constructor initializes Fd to -1 and IP to an empty string, Nickname and Username to "*"
        Client(int fd, const std::string &ip) : Fd(fd), IP(ip), Nickname("*"), Username("*") {} // Parameterized constructor
        Client(const Client &other) : Fd(other.Fd), IP(other.IP), Nickname(other.Nickname), Username(other.Username) {} // Copy constructor
        Client& operator=(const Client &other) { // Assignment operator
            if (this != &other) {
                Fd = other.Fd;
                IP = other.IP;
                Nickname = other.Nickname;
                Username = other.Username;
            }
            return *this;
        }
        ~Client() {} // Destructor
        
        bool operator==(const Client &other) const { // Equality operator
            return Fd == other.Fd && IP == other.IP;
        }
        
        int getFd() const { return Fd; } // Getter for file descriptor
        const std::string& ip() const { return IP; } // Getter for IP address
        void setFd(int fd) { Fd = fd; } // Setter for file descriptor
        void setIP(const std::string &ip) { IP = ip; } // Setter for IP address
        void setUsername(const std::string &username) { 
            Username = username; // Setter for username (nickname)
        }
        void setNickname(const std::string &nickname) { 
            Nickname = nickname; // Setter for nickname
        }
        const std::string& getNickname() const { return Nickname; } // Getter for nickname
        const std::string& getUsername() const { return Username; } // Getter for username

};

#endif