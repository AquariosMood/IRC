/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 19:07:10 by crios             #+#    #+#             */
/*   Updated: 2025/07/17 12:34:00 by crios            ###   ########.fr       */
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

    // Channel modes
    bool inviteOnly;               // For +i mode
    bool topicRestricted;          // For +t mode
    int userLimit;                 // For +l mode, 0 means no limit
    std::vector<int> inviteUsers;  // For +i mode
public:
    Channel(const std::string& channelName, const std::string& channelTopic = "", const std::string& channelPassword = "")
        : name(channelName), topic(channelTopic), password(channelPassword), inviteOnly(false), topicRestricted(true), userLimit(0) {}
    
    // Mode methods
    bool isInviteOnly() const { return inviteOnly; }
    bool isTopicRestricted() const { return topicRestricted; }
    int getUserLimit() const { return userLimit; }
    const std::string& getPassword() const { return password; }
    void setInviteOnly(bool mode) { inviteOnly = mode; }
    void setTopicRestricted(bool mode) { topicRestricted = mode; }
    void setUserLimit(int limit) { userLimit = limit; }
    void setPassword(const std::string& pass) { password = pass; }
    std::string getModes() const {
        std::string modes = "+";
        if (inviteOnly) modes += "i";
        if (topicRestricted) modes += "t";
        if (userLimit > 0) modes += "l";
        if (!password.empty()) modes += "k";
        return modes;
    }
    
    // Update method signatures
    bool isClientInChannel(int clientFd) const;
    void addClient(int clientFd);
    void addOperator(int clientFd);
    void removeOperator(int clientFd);
    void removeClient(int clientFd);
    bool isOperator(int clientFd) const;
    void addInvited(int clientFd);
    bool isInvited(int clientFd) const;
    void removeInvited(int clientFd);
    
    
    // Getters
    const std::string& getName() const { return name; }
    const std::string& getTopic() const { return topic; }
    const std::vector<int>& getClientFds() const { return clientFds; }
    const std::vector<int>& getOperatorFds() const { return operatorFds; }
    
    // Setters
    void setTopic(const std::string& newTopic) { topic = newTopic; }
};

#endif