/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 19:07:10 by crios             #+#    #+#             */
/*   Updated: 2025/07/16 13:21:04 by crios            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

// Forward declaration instead of including irc.hpp
class Client;

class Channel {
private:
    std::string name;
    std::string topic;
    std::string password;
    std::vector<int> clientFds;     // Store FDs instead of pointers
    std::vector<int> operatorFds;   // Store FDs instead of pointers

public:
    Channel(const std::string& channelName, const std::string& channelTopic = "", const std::string& channelPassword = "")
        : name(channelName), topic(channelTopic), password(channelPassword) {}

    // Update method signatures
    bool isClientInChannel(int clientFd) const;
    void addClient(int clientFd);
    void addOperator(int clientFd);
    void removeClient(int clientFd);
    bool isOperator(int clientFd) const;
    
    // Getters
    const std::string& getName() const { return name; }
    const std::string& getTopic() const { return topic; }
    const std::vector<int>& getClientFds() const { return clientFds; }
    const std::vector<int>& getOperatorFds() const { return operatorFds; }
    
    // Setters
    void setTopic(const std::string& newTopic) { topic = newTopic; }
};

#endif