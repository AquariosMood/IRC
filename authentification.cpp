/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   authentification.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 17:57:53 by crios             #+#    #+#             */
/*   Updated: 2025/08/07 16:43:35 by crios            ###   ########.fr       */
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
    }
}

void Server::handleNick(Client* client, std::istringstream& iss) {
    std::string nickname;
    iss >> nickname;
    
    if (nickname.empty()) {
        sendIRCReply(client->getFd(), ":localhost 431 * :No nickname given");
        return;
    }
    
    
    // Check if nickname is already taken par AUTRES clients
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getFd() != client->getFd() && // Exclure le client actuel
            clients[i].getNickname() == nickname) {
            sendIRCReply(client->getFd(), ":localhost 433 * " + nickname + " :Nickname is already in use");
            return;
        }
    }
    
    // ✅ Mettre à jour le nickname
    std::string oldNick = client->getNickname();
    client->setNickname(nickname);
    
    std::cout << "Client " << client->getFd() << " set nickname to '" << nickname << "'" << std::endl;
    
    // Vérifier l'enregistrement complet SEULEMENT si authentifié
    if (client->isAuthenticated()) {
        checkRegistration(client);
    }
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
    if (!reason.empty() && reason[0] == ' ') {
        reason = reason.substr(1); // Remove leading space
    }
    if (reason.empty()) {
        reason = "Client quit"; // Default reason if none provided
    }

    // Notify other clients in channels
    for (size_t i = 0; i < channels.size(); i++) {
        if (channels[i].isClientInChannel(client->getFd())) {
            const std::vector<int>& clientFds = channels[i].getClientFds();
            for (size_t j = 0; j < clientFds.size(); j++) {
                if (clientFds[j] != client->getFd()) {
                    sendIRCReply(clientFds[j], ":" + client->getNickname() + "!" + client->getUsername() + "@localhost QUIT :" + reason);
                }
            }
            channels[i].removeClient(client->getFd());
        }
    }
    std::cout << "Client " << client->getFd() << " quit: " << reason << std::endl;
    ClearClients(client->getFd()); // Remove client from server
}
