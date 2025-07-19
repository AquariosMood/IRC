/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 19:05:10 by crios             #+#    #+#             */
/*   Updated: 2025/07/16 16:54:13 by crios            ###   ########.fr       */
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
        std::string Username; // Username of the client
        std::string Realname; // Real name of the client
        bool authenticated; // Authentication status of the client
        bool registered; // Registration status of the client
    public:
        Client() : Fd(-1), IP(""), Nickname("*"), Username("*"), Realname("*"), authenticated(false), registered(false) {}
        Client(int fd, const std::string &ip) : Fd(fd), IP(ip), Nickname("*"), Username("*"), Realname("*"), authenticated(false), registered(false) {} // Parameterized constructor
        Client(const Client &other) : Fd(other.Fd), IP(other.IP), Nickname(other.Nickname), Username(other.Username), Realname(other.Realname), authenticated(other.authenticated), registered(other.registered) {} // Copy constructor
        Client& operator=(const Client &other) { // Assignment operator
            if (this != &other) {
                Fd = other.Fd;
                IP = other.IP;
                Nickname = other.Nickname;
                Username = other.Username;
                Realname = other.Realname;
                authenticated = other.authenticated;
                registered = other.registered;
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

        // Authentication methods
        bool isAuthenticated() const { return authenticated; }
        bool isRegistered() const { return registered; }
        void setAuthenticated(bool auth) { authenticated = auth; } // Setter for authentication status
        void setRegistered(bool reg) { registered = reg; } // Setter for registration status
        void setUsername(const std::string &username) {  Username = username; } // Setter for username
        void setNickname(const std::string &nickname) { Nickname = nickname; } // Setter for nickname
        void setRealname(const std::string &realname) { Realname = realname; } // Setter for real name
        const std::string& getNickname() const { return Nickname; } // Getter for nickname
        const std::string& getUsername() const { return Username; } // Getter for username
        const std::string& getRealname() const { return Realname; } // Getter for real name
};

#endif