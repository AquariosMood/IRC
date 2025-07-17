/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 11:58:19 by crios             #+#    #+#             */
/*   Updated: 2025/07/17 12:56:08 by crios            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"

void Server::handleMode(Client* client, std::istringstream& iss) {
    std::string target, modeString, param;
    iss >> target >> modeString >> param;

    if (target.empty()) {
        sendIRCReply(client->getFd(), ":localhost 461 " + client->getNickname() + " MODE :Not enough parameters");
        return;
    }
    
    // Check if target is a channel
    if (target[0] == '#' || target[0] == '&') {
        Channel* channel = findChannel(target);
        if (!channel) {
            sendIRCReply(client->getFd(), ":localhost 403 " + client->getNickname() + " " + target + " :No such channel");
            return;
        }

        // If no mode string is provided, show current modes
        if (modeString.empty()) {
            sendIRCReply(client->getFd(), ":localhost 324 " + client->getNickname() + " " + target + " :" + channel->getModes());
            return;
        }

        // Check if user is in the channel
        if (!channel->isClientInChannel(client->getFd())) {
            sendIRCReply(client->getFd(), ":localhost 442 " + client->getNickname() + " " + target + " :You're not on that channel");
            return;
        }

        // Check if user is an operator
        if (!channel->isOperator(client->getFd())) {
            sendIRCReply(client->getFd(), ":localhost 482 " + client->getNickname() + " " + target + " :You're not allowed to set modes");
            return;
        }

        // Handle +o and -o modes (give or remove operator status)
        if (modeString == "+o" && !param.empty()){
            // Give operator status
            Client* targetClient = NULL;
            for (size_t i = 0; i < clients.size(); i++) {
                if (clients[i].getNickname() == param) {
                    targetClient = &clients[i];
                    break;
                }
            }

            if (!targetClient) {
                sendIRCReply(client->getFd(), ":localhost 401 " + client->getNickname() + " " + param + " :No such nick");
                return;
            }

            if (!channel->isClientInChannel(targetClient->getFd())) {
                sendIRCReply(client->getFd(), ":localhost 441 " + client->getNickname() + " " + param + " " + target + " :They aren't on that channel");
                return;
            }

            if(channel->isOperator(targetClient->getFd())) {
                sendIRCReply(client->getFd(), ":localhost 696 " + client->getNickname() + " " + target + " :User is already an operator");
                return;
            }

            // Add operator status
            channel->addOperator(targetClient->getFd());
            
            // Notify all clients in the channel
            const std::vector<int>& clientFds = channel->getClientFds();
            for (size_t i = 0; i < clientFds.size(); i++) {
                sendIRCReply(clientFds[i], ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + target + " +o " + param);
            }
            std::cout << "Client " << param << " given operator status in " << target << " by " << client->getNickname() << std::endl;
        }
        else if (modeString == "-o" && !param.empty()){
            // Take away operator status
            Client* targetClient = NULL;
            for (size_t i = 0; i < clients.size(); i++) {
                if (clients[i].getNickname() == param) {
                    targetClient = &clients[i];
                    break;
                }
            }
            if (!targetClient) {
                sendIRCReply(client->getFd(), ":localhost 401 " + client->getNickname() + " " + param + " :No such nick");
                return;
            }
            
            if (!channel->isClientInChannel(targetClient->getFd())) {
                sendIRCReply(client->getFd(), ":localhost 441 " + client->getNickname() + " " + param + " " + target + " :They aren't on that channel");
                return;
            }

            if (!channel->isOperator(targetClient->getFd())) {
                sendIRCReply(client->getFd(), ":localhost 696 " + client->getNickname() + " " + target + " :User is not an operator");
                return;
            }
            
            // Remove operator status
            channel->removeOperator(targetClient->getFd());

            // Notify all clients in the channel
            const std::vector<int>& clientFds = channel->getClientFds();
            for (size_t i = 0; i < clientFds.size(); i++) {
                sendIRCReply(clientFds[i], ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + target + " -o " + param);
            }
            
            std::cout << "Client " << param << " removed from operator status in " << target << " by " << client->getNickname() << std::endl;
        } else if (modeString == "+i") {
            // Set invite-only mode
            channel->setInviteOnly(true);

            // Notify all clients in the channel
            const std::vector<int>& clientFds = channel->getClientFds();
            for (size_t i = 0; i < clientFds.size(); i++) {
                sendIRCReply(clientFds[i], ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + target + " +i");
            }
            std::cout << "Channel " << target << " set to invite-only mode by " << client->getNickname() << std::endl;
            
        } else if (modeString == "-i")
        {
            channel->setInviteOnly(false);

            // Notify all clients in the channel
            const std::vector<int>& clientFds = channel->getClientFds();
            for (size_t i = 0; i < clientFds.size(); i++) {
                sendIRCReply(clientFds[i], ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + target + " -i");
            }
            std::cout << "Channel " << target << " set to public mode by " << client->getNickname() << std::endl;
        } else if (modeString == "+t")
        {
            // Set topic-restricted mode
            channel->setTopicRestricted(true);

            // Notify all clients in the channel
            const std::vector<int>& clientFds = channel->getClientFds();
            for (size_t i = 0; i < clientFds.size(); i++) {
                sendIRCReply(clientFds[i], ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + target + " +t");
            }
            std::cout << "Channel " << target << " set to topic-restricted mode by " << client->getNickname() << std::endl;
        } else if (modeString == "-t") {
            // Remove topic-restricted mode
            channel->setTopicRestricted(false);

            // Notify all clients in the channel
            const std::vector<int>& clientFds = channel->getClientFds();
            for (size_t i = 0; i < clientFds.size(); i++) {
                sendIRCReply(clientFds[i], ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + target + " -t");
            }
            std::cout << "Channel " << target << " set to unrestricted topic mode by " << client->getNickname() << std::endl;
        } else if (modeString == "+k") {
            // Set channel password
            channel->setPassword(param);

            // Notify all clients in the channel
            const std::vector<int>& clientFds = channel->getClientFds();
            for (size_t i = 0; i < clientFds.size(); i++) {
                sendIRCReply(clientFds[i], ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + target + " +k " + param);
            }
            std::cout << "Channel " << target << " password set by " << client->getNickname() << std::endl;
        } else if (modeString == "-k") {
            // Remove channel password
            channel->setPassword("");

            // Notify all clients in the channel
            const std::vector<int>& clientFds = channel->getClientFds();
            for (size_t i = 0; i < clientFds.size(); i++) {
                sendIRCReply(clientFds[i], ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + target + " -k");
            }
            std::cout << "Channel " << target << " password removed by " << client->getNickname() << std::endl;
        } else if (modeString == "+l" && !param.empty()) {
            int limit = std::atoi(param.c_str());
            if (limit > 0 && limit <= 999) {
                channel->setUserLimit(limit);

                // Notify all clients in the channel
                const std::vector<int>& clientFds = channel->getClientFds();
                for (size_t i = 0; i < clientFds.size(); i++) {
                    sendIRCReply(clientFds[i], ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + target + " +l " + param);
                }
                std::cout << "Channel " << target << " user limit set to " << limit << " by " << client->getNickname() << std::endl;
            }
        } else if (modeString == "-l") {
            channel->setUserLimit(0);

            // Notify all clients in the channel
            const std::vector<int>& clientFds = channel->getClientFds();
            for (size_t i = 0; i < clientFds.size(); i++) {
                sendIRCReply(clientFds[i], ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + target + " -l");
            }
            std::cout << "Channel " << target << " user limit removed by " << client->getNickname() << std::endl;
        }
        else {
            sendIRCReply(client->getFd(), ":localhost 472 " + client->getNickname() + " " + target + " :Unknown mode flag");
        }
    } else {
        // User modes - just show current modes
        sendIRCReply(client->getFd(), ":localhost 221 " + client->getNickname() + " :" + client->getNickname() + " " + modeString);
    }
}