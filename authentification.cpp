/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   authentification.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 17:57:53 by crios             #+#    #+#             */
/*   Updated: 2025/07/15 17:58:50 by crios            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"

void Server::handlePass(Client* client, std::istringstream& iss) {
    std::string password;
    iss >> password;
    
    if (password.empty()) {
        sendIRCReply(client->getFd(), ":localhost 461 * PASS :Not enough parameters");
        return;
    }
    
    if (checkPassword(password)) {
        client->setAuthenticated(true);
        std::cout << "Client " << client->getFd() << " authenticated successfully" << std::endl;
    } else {
        sendIRCReply(client->getFd(), ":localhost 464 * :Password incorrect");
        std::cout << "Client " << client->getFd() << " failed authentication" << std::endl;
        ClearClients(client->getFd()); // Disconnect on wrong password
    }
}

void Server::handleNick(Client* client, std::istringstream& iss) {
    std::string nickname;
    iss >> nickname;
    
    if (nickname.empty()) {
        sendIRCReply(client->getFd(), ":localhost 431 * :No nickname given");
        return;
    }
    
    if (!client->isAuthenticated()) {
        sendIRCReply(client->getFd(), ":localhost 451 * :You have not registered");
        return;
    }
    
    // Check if nickname is already taken
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getFd() != client->getFd() && clients[i].getNickname() == nickname) {
            sendIRCReply(client->getFd(), ":localhost 433 * " + nickname + " :Nickname is already in use");
            return;
        }
    }
    
    client->setNickname(nickname);
    std::cout << "Client " << client->getFd() << " nickname set to: " << nickname << std::endl;
    
    // Check if registration is complete
    checkRegistration(client);
}

void Server::handleUser(Client* client, std::istringstream& iss) {
    std::string username, mode, unused;
    iss >> username >> mode >> unused;
    
    // Get realname (everything after the colon)
    std::string line;
    std::getline(iss, line);
    
    std::string realname;
    size_t colonPos = line.find(':');
    if (colonPos != std::string::npos) {
        realname = line.substr(colonPos + 1);
    } else {
        realname = username; // Default if no realname provided
    }
    
    if (username.empty()) {
        sendIRCReply(client->getFd(), ":localhost 461 * USER :Not enough parameters");
        return;
    }
    
    if (!client->isAuthenticated()) {
        sendIRCReply(client->getFd(), ":localhost 451 * :You have not registered");
        return;
    }
    
    client->setUsername(username);
    client->setRealname(realname);
    
    std::cout << "Client " << client->getFd() << " user info set:" << std::endl;
    std::cout << "  Username: " << username << std::endl;
    std::cout << "  Realname: " << realname << std::endl;
    
    // Check if registration is complete
    checkRegistration(client);
}

void Server::handleQuit(Client* client, std::istringstream& iss) {
    std::string reason;
    std::getline(iss, reason);
    if (!reason.empty() && reason[0] == ':') {
        reason = reason.substr(1);
    }
    
    std::cout << "Client " << client->getFd() << " (" << client->getNickname() 
              << ") quit: " << reason << std::endl;
    
    ClearClients(client->getFd());
}
