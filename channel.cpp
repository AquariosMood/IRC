/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 18:12:33 by crios             #+#    #+#             */
/*   Updated: 2025/07/15 18:34:49 by crios            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"
#include "Channel.hpp"

bool Channel::isClientInChannel(Client* client) const {
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i]->getFd() == client->getFd()) {
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
    
    std::cout << "Client " << client->getNickname() << " attempting to join " << channelName << std::endl;
    
    // Find or create channel
    Channel* channel = findChannel(channelName);
    if (!channel) {
        channel = createChannel(channelName, client);
        std::cout << "Created new channel: " << channelName << std::endl;
    }
    
    // Check if client is already in channel
    if (channel->isClientInChannel(client)) {
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
    Channel newChannel(name, "", ""); // name, topic, key
    channels.push_back(newChannel);
    Channel* channel = &channels.back();
    
    // Creator becomes operator
    channel->addOperator(creator);
    
    return channel;
}

void Server::addClientToChannel(Client* client, Channel* channel) {
    channel->addClient(client);
}

void Server::notifyChannelJoin(Client* client, Channel* channel) {
    std::string joinMessage = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost JOIN " + channel->getName();
    
    // Send join message to all clients in the channel
    std::vector<Client*> channelClients = channel->getClients();
    for (size_t i = 0; i < channelClients.size(); i++) {
        sendIRCReply(channelClients[i]->getFd(), joinMessage);
    }
}

void Server::sendChannelInfo(Client* client, Channel* channel) {
    std::string nick = client->getNickname();
    std::string channelName = channel->getName();
    
    // Send topic (if any)
    if (!channel->getTopic().empty()) {
        sendIRCReply(client->getFd(), ":localhost 332 " + nick + " " + channelName + " :" + channel->getTopic());
    } else {
        sendIRCReply(client->getFd(), ":localhost 331 " + nick + " " + channelName + " :No topic is set");
    }
    
    // Send names list (RPL_NAMREPLY)
    std::string namesList = ":localhost 353 " + nick + " = " + channelName + " :";
    std::vector<Client*> channelClients = channel->getClients();
    
    for (size_t i = 0; i < channelClients.size(); i++) {
        if (channel->isOperator(channelClients[i])) {
            namesList += "@"; // @ prefix for operators
        }
        namesList += channelClients[i]->getNickname();
        if (i < channelClients.size() - 1) {
            namesList += " ";
        }
    }
    
    sendIRCReply(client->getFd(), namesList);
    
    // End of names list (RPL_ENDOFNAMES)
    sendIRCReply(client->getFd(), ":localhost 366 " + nick + " " + channelName + " :End of /NAMES list");
}

// Add these implementations to the end of your channel.cpp file:

void Channel::addClient(Client* client) {
    // Check if client is already in the channel
    if (!isClientInChannel(client)) {
        clients.push_back(client);
    }
}

void Channel::addOperator(Client* client) {
    // Check if client is already an operator
    for (size_t i = 0; i < operators.size(); i++) {
        if (operators[i]->getFd() == client->getFd()) {
            return; // Already an operator
        }
    }
    operators.push_back(client);
}

bool Channel::isOperator(Client* client) const {
    for (size_t i = 0; i < operators.size(); i++) {
        if (operators[i]->getFd() == client->getFd()) {
            return true;
        }
    }
    return false;
}

void Channel::removeClient(Client* client) {
    // Remove from clients vector
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i]->getFd() == client->getFd()) {
            clients.erase(clients.begin() + i);
            break;
        }
    }
    
    // Also remove from operators if they were one
    for (size_t i = 0; i < operators.size(); i++) {
        if (operators[i]->getFd() == client->getFd()) {
            operators.erase(operators.begin() + i);
            break;
        }
    }
}

void Channel::removeOperator(Client* client) {
    for (size_t i = 0; i < operators.size(); i++) {
        if (operators[i]->getFd() == client->getFd()) {
            operators.erase(operators.begin() + i);
            break;
        }
    }
}
