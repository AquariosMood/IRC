/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 18:12:33 by crios             #+#    #+#             */
/*   Updated: 2025/07/16 13:33:13 by crios            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"
#include "Channel.hpp"

bool Channel::isClientInChannel(int clientFd) const {
    for (size_t i = 0; i < clientFds.size(); i++) {
        if (clientFds[i] == clientFd) {
            return true;
        }
    }
    return false;
}

Channel* Server::findChannel(const std::string& name) {
    for (size_t i = 0; i < channels.size(); i++) {
        if (channels[i].getName() == name) {
            return &channels[i];
        }
    }
    return NULL;
}

void Server::handleJoin(Client* client, std::istringstream& iss) {
    std::string channelName;
    iss >> channelName;
    
    if (channelName.empty()) {
        sendIRCReply(client->getFd(), ":localhost 461 " + client->getNickname() + " JOIN :Not enough parameters");
        return;
    }
    
    // Channel name must start with # or &
    if (channelName[0] != '#' && channelName[0] != '&') {
        sendIRCReply(client->getFd(), ":localhost 403 " + client->getNickname() + " " + channelName + " :No such channel");
        return;
    }
    
    // Validate channel name length and characters
    if (channelName.length() > 50) {
        sendIRCReply(client->getFd(), ":localhost 479 " + client->getNickname() + " " + channelName + " :Channel name too long");
        return;
    }
    
    // Check for invalid characters (space, comma, etc.)
    if (channelName.find(' ') != std::string::npos || channelName.find(',') != std::string::npos) {
        sendIRCReply(client->getFd(), ":localhost 479 " + client->getNickname() + " " + channelName + " :Invalid channel name");
        return;
    }
    
    std::cout << "Client " << client->getNickname() << " attempting to join " << channelName << std::endl;
    
    // Find or create channel
    Channel* channel = findChannel(channelName);
    if (!channel) {
        channel = createChannel(channelName, client);
        std::cout << "Created new channel: " << channelName << std::endl;
    }
    
    // Check if client is already in channel
    if (channel->isClientInChannel(client->getFd())) {
        std::cout << "Client " << client->getNickname() << " already in " << channelName << std::endl;
        return;
    }
    
    // Add client to channel
    addClientToChannel(client, channel);
    
    // Notify all clients in channel about the join
    notifyChannelJoin(client, channel);
    
    // Send channel topic and user list
    sendChannelInfo(client, channel);
    
    std::cout << "Client " << client->getNickname() << " joined " << channelName << std::endl;
}

Channel* Server::createChannel(const std::string& name, Client* creator) {
    Channel newChannel(name, "", "");
    channels.push_back(newChannel);
    Channel* channel = &channels.back();
    
    // Creator becomes operator
    if (creator)
        channel->addOperator(creator->getFd());
    
    return channel;
}

void Server::addClientToChannel(Client* client, Channel* channel) {
        channel->addClient(client->getFd());
}

void Server::notifyChannelJoin(Client* client, Channel* channel) {
    if (!client || !channel) return;
    
    std::string joinMessage = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost JOIN " + channel->getName();
    
    const std::vector<int>& clientFds = channel->getClientFds();
    for (size_t i = 0; i < clientFds.size(); i++) {
        sendIRCReply(clientFds[i], joinMessage);
    }
}

void Server::sendChannelInfo(Client* client, Channel* channel) {
    if (!client || !channel) return;
    
    std::string nick = client->getNickname();
    std::string channelName = channel->getName();
    
    // Send topic
    if (!channel->getTopic().empty()) {
        sendIRCReply(client->getFd(), ":localhost 332 " + nick + " " + channelName + " :" + channel->getTopic());
    } else {
        sendIRCReply(client->getFd(), ":localhost 331 " + nick + " " + channelName + " :No topic is set");
    }
    
    // Build names list
    const std::vector<int>& clientFds = channel->getClientFds();
    std::string namesList = ":localhost 353 " + nick + " = " + channelName + " :";
    
    for (size_t i = 0; i < clientFds.size(); i++) {
        Client* channelClient = getClientByFd(clientFds[i]);
        if (!channelClient) continue;
        
        if (channel->isOperator(clientFds[i])) {
            namesList += "@";
        }
        namesList += channelClient->getNickname();
        if (i < clientFds.size() - 1) {
            namesList += " ";
        }
    }
    
    sendIRCReply(client->getFd(), namesList);
    sendIRCReply(client->getFd(), ":localhost 366 " + nick + " " + channelName + " :End of /NAMES list");
}

// Add these implementations to the end of your channel.cpp file:

void Channel::addClient(int clientFd) {
    if (!isClientInChannel(clientFd)) {
        clientFds.push_back(clientFd);
    }
}

void Channel::addOperator(int clientFd) {
    for (size_t i = 0; i < operatorFds.size(); i++) {
        if (operatorFds[i] == clientFd) {
            return; // Already an operator
        }
    }
    operatorFds.push_back(clientFd);
}

bool Channel::isOperator(int clientFd) const {
    for (size_t i = 0; i < operatorFds.size(); i++) {
        if (operatorFds[i] == clientFd) {
            return true;
        }
    }
    return false;
}

void Channel::removeClient(int clientFd) {
    // Remove from clients
    for (size_t i = 0; i < clientFds.size(); i++) {
        if (clientFds[i] == clientFd) {
            clientFds.erase(clientFds.begin() + i);
            break;
        }
    }
    
    // Remove from operators if they were one
    for (size_t i = 0; i < operatorFds.size(); i++) {
        if (operatorFds[i] == clientFd) {
            operatorFds.erase(operatorFds.begin() + i);
            break;
        }
    }
}

void Server::handlePrivmsg(Client* client, std::istringstream& iss) {
    std::string target;
    iss >> target;
    
    if (target.empty()) {
        sendIRCReply(client->getFd(), ":localhost 411 " + client->getNickname() + " :No recipient given");
        return;
    }
    
    // Get the message (everything after target)
    std::string message;
    std::getline(iss, message);
    if (!message.empty() && message[0] == ' ') {
        message = message.substr(1); // Remove leading space
    }
    if (!message.empty() && message[0] == ':') {
        message = message.substr(1); // Remove leading :
    }
    
    if (message.empty()) {
        sendIRCReply(client->getFd(), ":localhost 412 " + client->getNickname() + " :No text to send");
        return;
    }
    
    // Check if target is a channel (starts with # or &)
    if (target[0] == '#' || target[0] == '&') {
        Channel* channel = findChannel(target);
        if (!channel) {
            sendIRCReply(client->getFd(), ":localhost 403 " + client->getNickname() + " " + target + " :No such channel");
            return;
        }
        
        if (!channel->isClientInChannel(client->getFd())) {
            sendIRCReply(client->getFd(), ":localhost 404 " + client->getNickname() + " " + target + " :Cannot send to channel");
            return;
        }
        
        // Send message to all clients in channel except sender
        const std::vector<int>& clientFds = channel->getClientFds();
        for (size_t i = 0; i < clientFds.size(); i++) {
            if (clientFds[i] != client->getFd()) {
                sendIRCReply(clientFds[i], ":" + client->getNickname() + "!" + client->getUsername() + "@localhost PRIVMSG " + target + " :" + message);
            }
        }
        
        std::cout << "Message in " << target << " from " << client->getNickname() << ": " << message << std::endl;
    } else {
        // Private message to user
        Client* targetClient = NULL;
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].getNickname() == target) {
                targetClient = &clients[i];
                break;
            }
        }
        
        if (!targetClient) {
            sendIRCReply(client->getFd(), ":localhost 401 " + client->getNickname() + " " + target + " :No such nick");
            return;
        }
        
        sendIRCReply(targetClient->getFd(), ":" + client->getNickname() + "!" + client->getUsername() + "@localhost PRIVMSG " + target + " :" + message);
        std::cout << "Private message from " << client->getNickname() << " to " << target << ": " << message << std::endl;
    }
}

// void Channel::removeOperator(Client* client) {
//     for (size_t i = 0; i < operators.size(); i++) {
//         if (operators[i]->getFd() == client->getFd()) {
//             operators.erase(operators.begin() + i);
//             break;
//         }
//     }
// }
