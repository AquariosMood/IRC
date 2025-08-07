/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helpers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 17:59:13 by crios             #+#    #+#             */
/*   Updated: 2025/08/07 11:49:53 by crios            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"

Client* Server::getClientByFd(int fd) {
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getFd() == fd) {
            return &clients[i];
        }
    }
    return NULL;
}

void Server::sendIRCReply(int fd, const std::string& reply) {
    // Check if fd is ready for writing
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLOUT;
    pfd.revents = 0;
    
    int poll_result = poll(&pfd, 1, 0); // Non-blocking check
    if (poll_result <= 0 || !(pfd.revents & POLLOUT)) {
        // Si poll() échoue, le fd pourrait être fermé
        if (poll_result < 0 || (pfd.revents & (POLLHUP | POLLERR | POLLNVAL))) {
            return;
        }
        std::cerr << "Warning: fd " << fd << " not ready for writing, trying anyway" << std::endl;
    }
    
    std::string message = reply + "\r\n";
    ssize_t bytes_sent = send(fd, message.c_str(), message.length(), 0);
    if (bytes_sent < 0) {
        // Détecter si le client s'est déconnecté brutalement
        if (errno == EPIPE || errno == ECONNRESET || errno == ENOTCONN)
            return; // Client disconnected
    }
}


void Server::sendWelcome(Client* client) {
    std::string nick = client->getNickname();
    std::string user = client->getUsername();
    
    // RPL_WELCOME (001)
    sendIRCReply(client->getFd(), ":localhost 001 " + nick + " :Welcome to the IRC Network " + nick + "!" + user + "@localhost");
    
    // RPL_YOURHOST (002)
    sendIRCReply(client->getFd(), ":localhost 002 " + nick + " :Your host is localhost, running version 1.0");
    
    // RPL_CREATED (003)
    sendIRCReply(client->getFd(), ":localhost 003 " + nick + " :This server was created today");
    
    // RPL_MYINFO (004)
    sendIRCReply(client->getFd(), ":localhost 004 " + nick + " localhost 1.0 o o");
}

bool Server::checkPassword(const std::string& password) const {
    return password == serverPassword;
}

void Server::checkRegistration(Client* client) {
    if (client->isAuthenticated() && 
        !client->getNickname().empty() && client->getNickname() != "*" &&
        !client->getUsername().empty() && client->getUsername() != "*" &&
        !client->isRegistered()) {
        
        client->setRegistered(true);
        sendWelcome(client);
        std::cout << "Client " << client->getFd() << " (" << client->getNickname() 
                  << ") is now fully registered!" << std::endl;
    }
}
