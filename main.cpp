/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 10:53:55 by crios             #+#    #+#             */
/*   Updated: 2025/06/30 10:59:16 by crios            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <server> <port>" << std::endl;
        return 1;
    }

    std::string server = argv[1];
    int port = std::atoi(argv[2]);

    if (port <= 0 || port > 65535)
    {
        std::cerr << "Invalid port number." << std::endl;
        return 1;
    }


    
    return 0;
}