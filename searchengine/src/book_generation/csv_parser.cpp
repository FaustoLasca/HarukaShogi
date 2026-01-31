// Minimal CSV parser for shogi_games.csv
// Compile standalone: g++ -std=c++17 csv_parser.cpp -o csv_parser

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <iomanip>

#include "../types.h"
#include "../misc.h"
#include "../position.h"
#include "../movegen.h"

using namespace harukashogi;

// Split a string by delimiter
std::vector<std::string> split(const std::string& str, char delim) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Split by whitespace, skipping empty tokens (handles multiple spaces)
std::vector<std::string> split_whitespace(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream ss(str);
    std::string token;
    while (ss >> token) {  // >> skips all whitespace automatically
        tokens.push_back(token);
    }
    return tokens;
}

// Remove comments in {} from moves string
std::string strip_comments(const std::string& str) {
    std::string result;
    bool in_comment = false;
    for (char c : str) {
        if (c == '{') {
            in_comment = true;
        } else if (c == '}') {
            in_comment = false;
        } else if (!in_comment) {
            result += c;
        }
    }
    return result;
}


// Write the book to a .cpp file as an array of OBEntry
// Data packing: 3 moves (16 bits each) in bits 0-47, counts (5 bits each) in bits 48-62
void write_book(const std::map<uint64_t, std::unordered_map<uint16_t, int>>& book, 
                const std::string& filename) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Failed to open " << filename << " for writing\n";
        return;
    }

    // Write header
    out << "// Auto-generated opening book - DO NOT EDIT\n";
    out << "#include \"book_data.h\"\n\n";
    out << "namespace harukashogi {\n\n";
    out << "const OBEntry BookData[] = {\n";

    size_t entry_count = 0;
    size_t skipped_positions = 0;
    for (const auto& [key, moves] : book) {
        // Filter out moves with count <= 1, then sort by count (descending)
        std::vector<std::pair<uint16_t, int>> sorted_moves;
        for (const auto& [move, count] : moves) {
            if (count > 1) {
                sorted_moves.emplace_back(move, count);
            }
        }
        
        // Skip position if no moves remain after filtering
        if (sorted_moves.empty()) {
            skipped_positions++;
            continue;
        }
        
        std::sort(sorted_moves.begin(), sorted_moves.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });

        // Pack up to 3 moves into data
        // Bits 0-15: move 1, bits 16-31: move 2, bits 32-47: move 3
        // Bits 48-52: count 1 (5 bits, max 31), bits 53-57: count 2, bits 58-62: count 3
        uint64_t data = 0;
        
        for (size_t i = 0; i < std::min(sorted_moves.size(), size_t(3)); i++) {
            uint16_t move_raw = sorted_moves[i].first;
            int count = std::min(sorted_moves[i].second, 31);  // cap at 5 bits
            
            data |= uint64_t(move_raw) << (i * 16);           // move in bits 0-47
            data |= uint64_t(count) << (48 + i * 5);          // count in bits 48-62
        }

        out << "    {0x" << std::hex << std::uppercase << std::setfill('0') 
            << std::setw(16) << key << "ull, 0x" 
            << std::setw(16) << data << std::dec << "ull},\n";
        entry_count++;
    }
    
    std::cout << "Skipped " << skipped_positions << " positions with only single-count moves\n";

    out << "};\n\n";
    out << "const size_t OPENING_BOOK_SIZE = " << entry_count << ";\n\n";
    out << "} // namespace harukashogi\n";

    out.close();
    std::cout << "Wrote " << entry_count << " entries to " << filename << "\n";
}

// Parse a single move from CSV format: "P7g-7f", "B'5e", "+R3c-4c"
// Returns: from_sq, to_sq, is_drop, is_promotion (you adapt to your Move class)
Move parse_move(const std::string& move_str) {
    if (move_str.empty()) return Move::null();

    if (move_str.size() < 5) return Move::null();
    
    size_t i = 0;
    bool promotion = (move_str.back() == '+');

    // move the index forward if the moving piece is promoted (starts with +)
    if (move_str[i] == '+') i++;
    
    // Piece character (P, L, N, S, G, B, R, K)
    char pieceChar = move_str[i++];
    PieceType pieceType = type_of(Piece(PieceToChar.find(pieceChar)));
    
    // Check if it's a drop (has apostrophe)
    bool is_drop = (move_str[i] == '\'');
    
    if (is_drop) {
        i++; // skip apostrophe
        // Target square: e.g., "5e"
        
        File file = File(move_str[i++] - '1');  // '1'-'9'
        Rank rank = Rank(move_str[i++] - 'a');  // 'a'-'i'
        Square to_sq = make_square(file, rank);

        Move move = Move(pieceType, to_sq);
        
        return move;
    } else {
        // Regular move: e.g., "7g-7f" or "7g-7f+" (with promotion)
        File from_file = File(move_str[i++] - '1');
        Rank from_rank = Rank(move_str[i++] - 'a');
        Square from_sq = make_square(from_file, from_rank);
        i++; // skip '-' or 'x'
        File to_file = File(move_str[i++] - '1');
        Rank to_rank = Rank(move_str[i++] - 'a');
        Square to_sq = make_square(to_file, to_rank);

        Move move = Move(from_sq, to_sq, promotion);

        return move;
    }
}


int main() {
    std::ifstream file("shogi_games.csv");
    if (!file.is_open()) {
        std::cerr << "Failed to open shogi_games.csv\n";
        return 1;
    }

    Position::init();
    Position pos;

    // temporary data structure to store the opening moves
    std::map<uint64_t, std::unordered_map<uint16_t, int>> book;
    
    std::string line;
    
    // Skip header line
    std::getline(file, line);
    
    int game_count = 0;
    while (std::getline(file, line)) {
        // Strip comments FIRST (they can contain commas which break CSV parsing)
        line = strip_comments(line);
        
        // CSV columns (index):
        // 0: game_id, 4: sente, 5: gote, 10: result, 19: moves
        auto fields = split(line, ',');
        
        if (fields.size() < 21) continue;
        
        std::string game_id = fields[0];
        std::string result = fields[10];  // "1-0", "0-1", etc.
        // std::cout << "num fields: " << fields.size() << "\n";
        std::string moves = fields[fields.size() - 2];
        
        // std::cout << "=== Game: " << game_id << " === Game count: " << game_count << "\n";
        // std::cout << "Result: " << result << "\n";
        
        // Parse individual moves (whitespace-separated)
        auto moveStrings = split_whitespace(moves);

        // initialize the position
        pos.set();
        
        // store the first 30 moves in the book
        for (size_t i = 0; i < std::min(moveStrings.size(), size_t(30)); i++) {
            Move move = parse_move(moveStrings[i]);

            if (move.is_null())
                break;

            // check that the move is valid
            Move moveList[MAX_MOVES];
            Move* end = generate_moves(pos, moveList);
            if (std::find(moveList, end, move) == end) {
                std::cout << "Position: " << pos.sfen() << "\n";
                std::cout << "Move string: " << moveStrings[i] << "\n";
                std::cout << "Parsed move: " << move << "\n";
                std::cout << "Game moves: " << moves << "\n";
                std::cout << "Move number: " << i << "\n";
                std::cout << "Game ID: " << game_id << "\n";
                std::cout << "Game count: " << game_count << "\n";
                
                break;
            }
            
            // if the position is not in the book, add it
            if (book.count(pos.get_key()) == 0)
                book[pos.get_key()] = std::unordered_map<uint16_t, int>();

            // if the move is not in the book, add it
            if (book[pos.get_key()].count(move.raw()) == 0)
                book[pos.get_key()][move.raw()] = 0;

            // increment the count of the move
            book[pos.get_key()][move.raw()]++;

            try {
                pos.make_move(move);
            } catch (const std::exception& e) {
                std::cout << "Error making move: " << e.what() << "\n";
                std::cout << "Position: " << pos.sfen() << "\n";
                std::cout << "Move: " << move << "\n";
                std::cout << "Move number: " << i << "\n";
                std::cout << "Game ID: " << game_id << "\n";
                std::cout << "Game count: " << game_count << "\n";
            }
        }
        
        game_count++;
    }

    std::cout << "Total games: " << game_count << "\n";
    std::cout << "Total positions: " << book.size() << "\n";
    
    file.close();

    // Write the book to a .cpp file
    write_book(book, "../book_data.cpp");

    return 0;
}
