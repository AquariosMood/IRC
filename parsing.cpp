/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 18:08:29 by crios             #+#    #+#             */
/*   Updated: 2025/07/15 18:09:03 by crios            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"

void Server::parseCommand(const std::string& message, int fd)
{
    std::istringstream iss(message);
    std::string command;
    iss >> command;
    
    Client* client = getClientByFd(fd);
    if (!client) {
        std::cerr << "Error: Client with fd " << fd << " not found" << std::endl;
        return;
    }
    
    // std::cout << "Command from client " << fd << ": " << command << std::endl;
    
    // Commands that don't require authentication
    if (command == "PASS") {
        handlePass(client, iss);
    } else if (command == "QUIT") {
        handleQuit(client, iss);
    } 
    // Commands that require authentication but not full registration
    else if (command == "NICK") {
        handleNick(client, iss);
    } else if (command == "USER") {
        handleUser(client, iss);
    }
    // Commands that require full registration
    else if (command == "JOIN") {
        if (!client->isRegistered()) {
            sendIRCReply(fd, ":localhost 451 * :You have not registered");
            return;
        }
        std::cout << "JOIN command received from " << client->getNickname() << std::endl;
    } else if (command == "PRIVMSG") {
        if (!client->isRegistered()) {
            sendIRCReply(fd, ":localhost 451 * :You have not registered");
            return;
        }
        std::cout << "PRIVMSG command received from " << client->getNickname() << std::endl;
    } else {
        std::cout << "Unknown command: " << command << std::endl;
    }
}
