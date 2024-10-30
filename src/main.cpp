#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>
#include <cstdlib>
#include <ctime>

class TicTacToe {
private:
    std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo
    std::mutex board_mutex; // Mutex para controle de acesso ao tabuleiro
    std::condition_variable turn_cv; // Variável de condição para alternância de turnos
    char current_player; // Jogador atual ('X' ou 'O')
    bool game_over; // Estado do jogo
    char winner; // Vencedor do jogo

public:
    TicTacToe() : current_player('X'), game_over(false), winner(' ') {
        // Inicializar o tabuleiro com espaços em branco
        for (auto& row : board) {
            row.fill(' ');
        }
    }

    void display_board() {
        std::cout << "\nTabuleiro atual:\n";
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                std::cout << board[i][j];
                if (j < 2) std::cout << " | ";
            }
            std::cout << "\n";
            if (i < 2) std::cout << "---------\n";
        }
        std::cout << std::endl;
    }

    bool make_move(char player, int row, int col) {
        std::unique_lock<std::mutex> lock(board_mutex);

        // Esperar até que seja a vez do jogador ou o jogo tenha terminado
        turn_cv.wait(lock, [this, player] { return current_player == player || game_over; });

        // Se o jogo terminou, sair imediatamente
        if (game_over) {
            return false;
        }

        if (board[row][col] == ' ') {
            board[row][col] = player;
            display_board();

            // Verificar vitória e empate
            if (check_win(player)) {
                game_over = true;
                winner = player;
            } else if (check_draw()) {
                game_over = true;
                winner = 'D';
            }

            // Alternar o jogador
            current_player = (player == 'X') ? 'O' : 'X';

            // Notificar o próximo jogador ou encerrar as threads se o jogo acabou
            turn_cv.notify_all();
            return true;
        }

        return false;
    }

    bool check_win(char player) {
        // Verificar linhas, colunas e diagonais
        for (int i = 0; i < 3; i++) {
            if (board[i][0] == player && board[i][1] == player && board[i][2] == player) return true;
            if (board[0][i] == player && board[1][i] == player && board[2][i] == player) return true;
        }
        if (board[0][0] == player && board[1][1] == player && board[2][2] == player) return true;
        if (board[0][2] == player && board[1][1] == player && board[2][0] == player) return true;

        return false;
    }

    bool check_draw() {
        for (const auto& row : board) {
            for (const auto& cell : row) {
                if (cell == ' ') return false;
            }
        }
        return true;
    }

    bool is_game_over() {
        std::unique_lock<std::mutex> lock(board_mutex);
        return game_over;
    }

    char get_winner() {
        return winner;
    }
};

// Classe Player
class Player {
private:
    TicTacToe& game; // Referência para o jogo
    char symbol; // Símbolo do jogador ('X' ou 'O')
    std::string strategy; // Estratégia do jogador

public:
    Player(TicTacToe& g, char s, const std::string& strat)
        : game(g), symbol(s), strategy(strat) {
        std::srand(std::time(0)); // Inicializar a semente do gerador de números aleatórios
    }

    void play() {
        while (true) {
            if (game.is_game_over()) {
                break;
            }

            if (strategy == "sequencial") {
                play_sequential();
            } else {
                play_random();
            }

            if (game.is_game_over()) {
                break;
            }
        }
    }

private:
    void play_sequential() {
        // Percorre o tabuleiro sequencialmente até encontrar uma posição livre para jogar
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                if (game.make_move(symbol, row, col)) {
                    return; // Sai da função após fazer a jogada
                }
            }
        }
    }

    void play_random() {
        // Tenta fazer uma jogada aleatória até encontrar uma posição livre
        int row, col;
        while (true) {
            row = std::rand() % 3; // Gera uma linha aleatória entre 0 e 2
            col = std::rand() % 3; // Gera uma coluna aleatória entre 0 e 2
            if (game.make_move(symbol, row, col)) {
                return; // Sai da função após fazer a jogada
            }
            if (game.is_game_over()) {
                return; // Verifica se o jogo terminou durante a busca
            }
        }
    }
};

// Função principal
int main() {
    // Inicializar o jogo
    TicTacToe game;

    // Inicializar os jogadores
    Player player1(game, 'X', "sequencial");
    Player player2(game, 'O', "aleatório");

    // Criar as threads para os jogadores
    std::thread t1(&Player::play, &player1);
    std::thread t2(&Player::play, &player2);

    // Aguardar o término das threads
    t1.join();
    t2.join();

    // Exibir o resultado final do jogo
    std::cout << "Jogo terminado! ";
    if (game.get_winner() == 'D') {
        std::cout << "Resultado: Empate! 'D' " << std::endl;
    } else {
        std::cout << "Vencedor: Jogador " << game.get_winner() << "!" << std::endl;
    }

    return 0;
}
