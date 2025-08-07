/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 18:08:29 by crios             #+#    #+#             */
/*   Updated: 2025/08/07 11:46:24 by crios            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"

void Server::sendLoginInstructions(int fd) {
    sendIRCReply(fd, ":localhost NOTICE * :=== IRC Server Login Instructions ===");
    sendIRCReply(fd, ":localhost NOTICE * :1. PASS <password>     - Authenticate with server password");
    sendIRCReply(fd, ":localhost NOTICE * :2. NICK <nickname>     - Set your nickname");
    sendIRCReply(fd, ":localhost NOTICE * :3. USER <username> 0 * :<realname> - Set user info");
    sendIRCReply(fd, ":localhost NOTICE * :Example: USER alice 0 * :Alice Wonderland");
    sendIRCReply(fd, ":localhost NOTICE * :After login, use JOIN #channel to join a channel");
    sendIRCReply(fd, ":localhost NOTICE * :=======================================");
}

void Server::sendWelcomeMessage(int fd) {
    sendIRCReply(fd, ":localhost NOTICE * :Welcome to our IRC Server!");
    sendIRCReply(fd, ":localhost NOTICE * :Please authenticate to continue:");
    sendLoginInstructions(fd);
}

void Server::parseCommand(const std::string& message, int fd)
{
    std::istringstream iss(message);
    std::string command;
    iss >> command;
    
    Client* client = getClientByFd(fd);
    if (!client) {
        std::cerr << "Error: Client with fd " << fd << " not found" << std::endl;
        ClearClients(fd); // Clean up if client not found
        return;
    }
    // Trim whitespace and check for empty commands
    command.erase(0, command.find_first_not_of(" \t\r\n"));
    command.erase(command.find_last_not_of(" \t\r\n") + 1);
    
    if (command.empty()) {
        return; // Don't process empty commands
    }
    
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
            sendLoginInstructions(fd); // Ajouter les instructions
            return;
        }
        handleJoin(client, iss); // Appeler la vraie fonction JOIN
    } else if (command == "PRIVMSG") {
        if (!client->isRegistered()) {
            sendIRCReply(fd, ":localhost 451 * :You have not registered");
            sendLoginInstructions(fd);
            return;
        }
        handlePrivmsg(client, iss);
    
    } else if (command == "PART") {
        if (!client->isRegistered()) {
            sendIRCReply(fd, ":localhost 451 * :You have not registered");
            sendLoginInstructions(fd);
            return;
        }
        handlePart(client, iss);
    } else if (command == "TOPIC") {
        if (!client->isRegistered()){
            sendIRCReply(fd, ":localhost 451 * :You have not registered");
            sendLoginInstructions(fd);
            return;
        }
        handleTopic(client, iss);
    } else if (command == "KICK")
    {
        if (!client->isRegistered()) {
            sendIRCReply(fd, ":localhost 451 * :You have not registered");
            sendLoginInstructions(fd);
            return;
        }
        handleKick(client, iss);
    } else if (command == "MODE") {
        if (!client->isRegistered()) {
            sendIRCReply(fd, ":localhost 451 * :You have not registered");
            sendLoginInstructions(fd);
            return;
        }
        handleMode(client, iss);
    } else if (command == "INVITE") {
        if (!client->isRegistered()) {
            sendIRCReply(fd, ":localhost 451 * :You have not registered");
            sendLoginInstructions(fd);
            return;
        }
        handleInvite(client, iss);
    }
    else {
        // Commande inconnue - donner de l'aide
        if (!client->isRegistered()) {
            sendLoginInstructions(fd);
        } else {
            sendIRCReply(fd, ":localhost 421 " + client->getNickname() + " " + command + " :Unknown command");
        }
    }
}