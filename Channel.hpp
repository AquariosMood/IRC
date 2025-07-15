/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 19:07:10 by crios             #+#    #+#             */
/*   Updated: 2025/07/15 14:23:27 by crios            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include "Client.hpp"
#include <vector>
#include <algorithm> // For std::remove

// Une classe channel
class Channel {
    private:
        std::string name; // Name of the channel
        std::string topic; // Topic of the channel
        std::string password; // Password for the channel
        std::vector<Client> members; // Vector to store clients in the channel
        std::vector<Client> operators; // Vector to store channel operators
    public:
        Channel(const std::string& name, const std::string& topic = "", const std::string& password = "")
            : name(name), topic(topic), password(password) {}

        void addMember(const Client& client) {
            members.push_back(client);
        }

        void removeMember(const Client& client) {
            members.erase(std::remove(members.begin(), members.end(), client), members.end());
        }

        void setTopic(const std::string& newTopic) {
            topic = newTopic;
        }

        void setPassword(const std::string& newPassword) {
            password = newPassword;
        }

        const std::string& getName() const {
            return name;
        }

        const std::vector<Client>& getMembers() const {
            return members;
        }
};

#endif