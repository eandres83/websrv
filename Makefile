NAME = websrv

CC = c++ -Wall -Wextra -Werror -std=c++98 -g

RM = rm -rf

SRC_DIR = ./src/
OBJ_DIR = ./objects/

SRC = $(shell find $(SRC_DIR) -type f -name "*.cpp")
OBJS = $(SRC:$(SRC_DIR)%.cpp=$(OBJ_DIR)%.o)

all: $(NAME)

$(NAME): $(OBJS) 
	$(CC) $(OBJS) -o $(NAME)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	@mkdir -p $(dir $@)
	$(CC) -c $< -o $@ 

clean:
	$(RM) $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
