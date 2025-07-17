/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   invite.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 12:50:49 by crios             #+#    #+#             */
/*   Updated: 2025/07/17 13:07:20 by crios            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"

void Server::handleInvite(Client* client, std::istringstream& iss) {
    std::string targetNick, channelName;
    iss >> targetNick >> channelName;

    if (targetNick.empty() || channelName.empty()) {
        sendIRCReply(client->getFd(), ":localhost 461 " + client->getNickname() + " INVITE :Not enough parameters");
        return;
    }

    // Validate channel name
    if (channelName[0] != '#' && channelName[0] != '&') {
        sendIRCReply(client->getFd(), ":localhost 403 " + client->getNickname() + " " + channelName + " :No such channel");
        return;
    }

    Channel* channel = findChannel(channelName);
    if (!channel) {
        sendIRCReply(client->getFd(), ":localhost 403 " + client->getNickname() + " " + channelName + " :No such channel");
        return;
    }

    // Check if client is in the channel
    if (!channel->isClientInChannel(client->getFd())) {
        sendIRCReply(client->getFd(), ":localhost 442 " + client->getNickname() + " " + channelName + " :You're not on that channel");
        return;
    }

    // Check if channel is invite-only
    if (channel->isInviteOnly() && !channel->isOperator(client->getFd())) {
        sendIRCReply(client->getFd(), ":localhost 482 " + client->getNickname() + " " + channelName + " :You're not channel operator");
        return;
    }

    // Find the target client
    Client* targetClient = NULL;
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getNickname() == targetNick) {
            targetClient = &clients[i];
            break;
        }
    }

    if (!targetClient) {
        sendIRCReply(client->getFd(), ":localhost 401 " + client->getNickname() + " " + targetNick + " :No such nick");
        return;
    }

    // Check if the target client is already in the channel
    if (channel->isClientInChannel(targetClient->getFd())) {
        sendIRCReply(client->getFd(), ":localhost 443 " + client->getNickname() + " " + targetNick + " " + channelName + " :User is already in the channel");
        return;
    }

    // Add the target client to the invite list
    channel->addInvited(targetClient->getFd());

    // Send responses
    sendIRCReply(client->getFd(), ":localhost 341 " + client->getNickname() + " " + targetNick + " " + channelName);
    sendIRCReply(targetClient->getFd(), ":" + client->getNickname() + "!" + client->getUsername() + "@localhost INVITE " + targetNick + " :" + channelName);

    std::cout << "Client " << client->getNickname() << " invited " << targetNick << " to channel " << channelName << std::endl;
}