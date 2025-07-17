/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 18:12:33 by crios             #+#    #+#             */
/*   Updated: 2025/07/17 12:44:53 by crios            ###   ########.fr       */
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

void Channel::addInvited(int clientFd) {
    for (size_t i = 0; i < inviteUsers.size(); i++) {
        if (inviteUsers[i] == clientFd) {
            return; // Already invited
        }
    }
    inviteUsers.push_back(clientFd);
}

bool Channel::isInvited(int clientFd) const {
    for (size_t i = 0; i < inviteUsers.size(); i++) {
        if (inviteUsers[i] == clientFd) {
            return true; // Client is invited
        }
    }
    return false; // Not invited
}

void Channel::removeInvited(int clientFd) {
    for (size_t i = 0; i < inviteUsers.size(); i++) {
        if (inviteUsers[i] == clientFd) {
            inviteUsers.erase(inviteUsers.begin() + i);
            return; // Removed successfully
        }
    }
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
        // Create new channel
        channel = createChannel(channelName, client);
        std::cout << "Created new channel: " << channelName << std::endl;
        
        // Add client to the new channel
        channel->addClient(client->getFd());
    } else {
        // Channel exists - check permissions and limits
        
        // Check if client is already in channel
        if (channel->isClientInChannel(client->getFd())) {
            sendIRCReply(client->getFd(), ":localhost 443 " + client->getNickname() + " " + channelName + " :You are already on that channel");
            return;
        }
        
        // Check invite-only mode
        if (channel->isInviteOnly()) {
            if (!channel->isInvited(client->getFd())) {
                sendIRCReply(client->getFd(), ":localhost 473 " + client->getNickname() + " " + channelName + " :Cannot join channel (+i)");
                return;
            }
            // Remove from invite list after successful join
            channel->removeInvited(client->getFd());
        }

        // Check user limit
        if (channel->getUserLimit() > 0) {
            if ((int)channel->getClientFds().size() >= channel->getUserLimit()) {
                sendIRCReply(client->getFd(), ":localhost 471 " + client->getNickname() + " " + channelName + " :Cannot join channel (+l)");
                return;
            }
        }

        // Check if channel has a password
        if (!channel->getPassword().empty()) {
            std::string providedPassword;
            iss >> providedPassword;
            if (providedPassword != channel->getPassword()) {
                sendIRCReply(client->getFd(), ":localhost 475 " + client->getNickname() + " " + channelName + " :Cannot join channel (+k)");
                return;
            }
        }
        
        // Add client to existing channel
        channel->addClient(client->getFd());
    }
    
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

void Channel::removeOperator(int clientFd) {
    for (size_t i = 0; i < operatorFds.size(); i++) {
        if (operatorFds[i] == clientFd) {
            operatorFds.erase(operatorFds.begin() + i);
            break;
        }
    }
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

void Server::handlePart(Client *client, std::istringstream& iss)
{
    std::string channelName;
    iss >> channelName;

    if (channelName.empty()) {
        sendIRCReply(client->getFd(), ":localhost 461 " + client->getNickname() + " PART :Not enough parameters");
        return;
    }

    Channel* channel = findChannel(channelName);
    if (!channel) {
        sendIRCReply(client->getFd(), ":localhost 403 " + client->getNickname() + " " + channelName + " :No such channel");
        return;
    }

    if (!channel->isClientInChannel(client->getFd())) {
        sendIRCReply(client->getFd(), ":localhost 442 " + client->getNickname() + " " + channelName + " :You're not on that channel");
        return;
    }

    // Notify channel members about the part
    const std::vector<int>& clientFds = channel->getClientFds();
    for (size_t i = 0; i < clientFds.size(); i++) {
        sendIRCReply(clientFds[i], ":" + client->getNickname() + "!" + client->getUsername() + "@localhost PART " + channelName);
    }

    // Remove client from channel
    channel->removeClient(client->getFd());

    std::cout << "Client " << client->getNickname() << " left " << channelName << std::endl;
}

void Server::handleTopic(Client *client, std::istringstream& iss)
{
    std::string channelName;
    iss >> channelName;

    if (channelName.empty()) {
        sendIRCReply(client->getFd(), ":localhost 461 " + client->getNickname() + " TOPIC :Not enough parameters");
        return;
    }
    if (channelName[0] != '#' && channelName[0] != '&') {
        sendIRCReply(client->getFd(), ":localhost 403 " + client->getNickname() + " " + channelName + " :No such channel");
        return;
    }
    
    Channel* channel = findChannel(channelName);
    if (!channel) {
        sendIRCReply(client->getFd(), ":localhost 403 " + client->getNickname() + " " + channelName + " :No such channel");
        return;
    }

    if (!channel->isClientInChannel(client->getFd())) {
        sendIRCReply(client->getFd(), ":localhost 442 " + client->getNickname() + " " + channelName + " :You're not on that channel");
        return;
    }

    // Get remaining content after channel name
    std::string remainder;
    std::getline(iss, remainder);
    
    // PROPER TRIMMING: Remove all leading and trailing whitespace including \r\n\t
    size_t start = remainder.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        // String contains only whitespace - treat as empty
        remainder = "";
    } else {
        remainder = remainder.substr(start);
        // Also trim trailing whitespace
        size_t end = remainder.find_last_not_of(" \t\r\n");
        if (end != std::string::npos) {
            remainder = remainder.substr(0, end + 1);
        }
    }
    
    // Check if the user wants to set a new topic
    if (remainder.empty()) {
        // No parameters after channel name - just view current topic
        if (!channel->getTopic().empty()) {
            sendIRCReply(client->getFd(), ":localhost 332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic());
        } else {
            sendIRCReply(client->getFd(), ":localhost 331 " + client->getNickname() + " " + channelName + " :No topic is set");
        }
        return; // IMPORTANT: Stop here!
    }
    
    // If we reach here, user wants to change the topic
    std::string newTopic = remainder;

    if (channel->isTopicRestricted() && !channel->isOperator(client->getFd())) {
        sendIRCReply(client->getFd(), ":localhost 482 " + client->getNickname() + " " + channelName + " :You're not channel operator");
        return;
    }
    
    // Remove leading : if present
    if (!newTopic.empty() && newTopic[0] == ':') {
        newTopic = newTopic.substr(1);
    }
    
    // Set the new topic
    channel->setTopic(newTopic);

    // Notify all OTHER clients in channel about the new topic
    const std::vector<int>& clientFds = channel->getClientFds();
    for (size_t i = 0; i < clientFds.size(); i++) {
        if (clientFds[i] != client->getFd()) {
            sendIRCReply(clientFds[i], ":" + client->getNickname() + "!" + client->getUsername() + "@localhost TOPIC " + channelName + " :" + newTopic);
        }
    }
    
    // Send confirmation to the person who set the topic
    if (!newTopic.empty()) {
        sendIRCReply(client->getFd(), ":localhost 332 " + client->getNickname() + " " + channelName + " :" + newTopic);
    } else {
        sendIRCReply(client->getFd(), ":localhost 331 " + client->getNickname() + " " + channelName + " :No topic is set");
    }
    
    std::cout << "Topic changed in " << channelName << " by " << client->getNickname() << ": " << newTopic << std::endl;
}

void Server::handleKick(Client* client, std::istringstream& iss) {
    std::string channelName, targetNickname, reason;
    iss >> channelName >> targetNickname;
    
    std::getline(iss, reason);

    if (channelName.empty() || targetNickname.empty()) {
        sendIRCReply(client->getFd(), ":localhost 461 " + client->getNickname() + " KICK :Not enough parameters");
        return;
    }

    // Channel name must start with # or &
    if (channelName[0] != '#' && channelName[0] != '&')
    {
        sendIRCReply(client->getFd(), ":localhost 403 " + client->getNickname() + " " + channelName + " :No such channel");
        return;
    }
    
    Channel* channel = findChannel(channelName);
    if (!channel) {
        sendIRCReply(client->getFd(), ":localhost 403 " + client->getNickname() + " " + channelName + " :No such channel");
        return;
    }
    
    // Check if the kicker is in the channel
    if (!channel->isClientInChannel(client->getFd())) {
        sendIRCReply(client->getFd(), ":localhost 442 " + client->getNickname() + " " + channelName + " :You're not on that channel");
        return;
    }

    // Check if the target is an operator
    if (!channel->isOperator(client->getFd())) {
        sendIRCReply(client->getFd(), ":localhost 482 " + client->getNickname() + " " + channelName + " :You're not channel operator");
        return;
    }
    
    // Find the target client
    Client* targetClient = NULL;
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getNickname() == targetNickname) {
            targetClient = &clients[i];
            break;
        }
    }

    if (!targetClient) {
        sendIRCReply(client->getFd(), ":localhost 401 " + client->getNickname() + " " + targetNickname + " :No such nick");
        return;
    }

    // Check if the target is in the channel
    if (!channel->isClientInChannel(targetClient->getFd())) {
        sendIRCReply(client->getFd(), ":localhost 441 " + client->getNickname() + " " + targetNickname + " " + channelName + " :They aren't on that channel");
        return;
    }

    // Clean up reason
    if (!reason.empty() && reason[0] == ' ') {
        reason = reason.substr(1); // Remove leading space
    }
    if (!reason.empty() && reason[0] == ':') {
        reason = reason.substr(1); // Remove leading :
    }
    if (reason.empty()) {
        reason = "You have been kicked from the channel by " + client->getNickname();
    }
    

    // Notify all clients in the channel about the kick
    const std::vector<int>& clientFds = channel->getClientFds();
    for (size_t i = 0; i < clientFds.size(); i++)
    {
        if (clientFds[i] != client->getFd()) {
            sendIRCReply(clientFds[i], ":" + client->getNickname() + "!" + client->getUsername() + "@localhost KICK " + channelName + " " + targetNickname + " :" + reason);
        }
    }
    
    // Remove the target client from the channel
    channel->removeClient(targetClient->getFd());
    std::cout << "Client " << targetNickname << " kicked from " << channelName << " by " << client->getNickname() << ": " << reason << std::endl;
}
