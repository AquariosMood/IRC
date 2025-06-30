# Nom de l'exécutable
NAME = ircserv

# Compilateur et flags
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# Sources et objets
SRCS = main.cpp \

OBJS = $(SRCS:.cpp=.o)

# Règle principale
all: $(NAME)

# Construction de l'exécutable
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

# Règle générique pour compiler les .cpp en .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Nettoyage des fichiers objets
clean:
	rm -f $(OBJS)

# Nettoyage complet
fclean: clean
	rm -f $(NAME)

# Reconstruction
re: fclean all

# Protection contre les noms de fichiers qui pourraient correspondre à des règles
.PHONY: all clean fclean re