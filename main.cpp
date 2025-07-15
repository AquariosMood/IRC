/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: crios <crios@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 10:53:55 by crios             #+#    #+#             */
/*   Updated: 2025/07/15 14:14:51 by crios            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"
#include "Channel.hpp"
#include <cassert>

void testChannel() {
    std::cout << "=== Test Channel ===" << std::endl;
    
    // Test 1: Création d'un channel
    Channel channel("#test", "Welcome topic", "password123");
    std::cout << "✓ Channel créé: " << channel.getName() << std::endl;
    
    // Test 2: Création de clients
    Client client1(1, "192.168.1.100");
    Client client2(2, "192.168.1.101");
    Client client3(3, "192.168.1.102");
    
    // Test 3: Ajouter des membres
    channel.addMember(client1);
    channel.addMember(client2);
    channel.addMember(client3);
    
    std::cout << "✓ Membres ajoutés. Nombre de membres: " << channel.getMembers().size() << std::endl;
    assert(channel.getMembers().size() == 3);
    
    // Test 4: Supprimer un membre
    channel.removeMember(client2);
    std::cout << "✓ Membre supprimé. Nombre de membres: " << channel.getMembers().size() << std::endl;
    assert(channel.getMembers().size() == 2);
    
    // Test 5: Changer le topic
    channel.setTopic("Nouveau topic de test");
    std::cout << "✓ Topic modifié" << std::endl;
    
    // Test 6: Changer le mot de passe
    channel.setPassword("newpass456");
    std::cout << "✓ Mot de passe modifié" << std::endl;
    
    std::cout << "=== Tous les tests Channel réussis! ===" << std::endl;
}

int main(int argc, char **argv)
{
    if (argc > 1 && std::string(argv[1]) == "--test") {
        std::cout << "=== RUNNING TESTS ===" << std::endl;
        testChannel();
        std::cout << "=== TESTS COMPLETED ===" << std::endl;
        return 0;
    }
    
    // Uncomment for production use
    // if (argc != 3) {
    //     std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
    //     std::cerr << "       " << argv[0] << " --test (to run tests)" << std::endl;
    //     return 1;
    // }
    
    // For development - run tests then server
    std::cout << "=== RUNNING TESTS ===" << std::endl;
    testChannel();
    std::cout << "=== TESTS COMPLETED ===\n" << std::endl;
    
    Server ser;
    std::cout << "---- SERVER ----" << std::endl;
    try {
        signal(SIGINT, Server::SignalHandler);
        signal(SIGQUIT, Server::SignalHandler);
        ser.ServerInit();
    }
    catch(const std::exception& e) {
        ser.CloseFds();
        std::cerr << e.what() << std::endl;
        return 1; // Return error code
    }
    std::cout << "The Server Closed!" << std::endl;
    return 0; // Add this return statement
}
