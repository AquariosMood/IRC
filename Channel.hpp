/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 19:07:10 by crios             #+#    #+#             */
/*   Updated: 2025/07/15 18:34:47 by crios            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include "Client.hpp"
#include "irc.hpp"
#include <vector>
#include <algorithm> // For std::remove

// Une classe channel
class Channel {
private:
    std::string name;
    std::string topic;
    std::string password;
    std::vector<Client*> clients;     // Add this member
    std::vector<Client*> operators;   // Add this member

public:
    // Constructor
    Channel(const std::string& name, const std::string& topic = "", const std::string& password = "")
        : name(name), topic(topic), password(password) {}

    const std::string& getTopic() const { return topic; }
    const std::vector<Client*>& getClients() const { return clients; }
    bool isOperator(Client* client) const;
    void addOperator(Client* client);
    void removeOperator(Client* client);
    void addClient(Client* client);
    void removeClient(Client* client);
    bool isClientInChannel(Client* client) const;
    
    const std::string& getName() const { return name; }
    void setTopic(const std::string& newTopic) { topic = newTopic; }
};

#endif