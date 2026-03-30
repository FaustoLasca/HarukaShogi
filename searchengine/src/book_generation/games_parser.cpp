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
#include "../engine.h"

using namespace harukashogi;


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

        // max count
        int max_count = sorted_moves[0].second;

        // Pack up to 3 moves into data
        // Bits 0-15: move 1, bits 16-31: move 2, bits 32-47: move 3
        // Bits 48-52: count 1 (5 bits, max 31), bits 53-57: count 2, bits 58-62: count 3
        uint64_t data = 0;
        
        for (size_t i = 0; i < std::min(sorted_moves.size(), size_t(3)); i++) {
            // get the move and count
            uint16_t move_raw = sorted_moves[i].first;
            int count = sorted_moves[i].second;

            // scale the count to 5 bits
            if (max_count > 31) {
                count = (count * 31) / max_count;
            }
            
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


int main() {
    std::ifstream file("data/games.txt");
    if (!file.is_open()) {
        std::cerr << "Failed to open data/games.txt\n";
        return 1;
    }

    init();
    Position pos;

    // temporary data structure to store the opening moves
    std::map<uint64_t, std::unordered_map<uint16_t, int>> book;
    
    std::string line, token;
    Move moveList[MAX_MOVES];
    Move move, *end;
    
    int game_count = 0;
    while (std::getline(file, line)) {
        std::stringstream ss(line);

        pos.set();

        // store the first 30 moves in the book
        while (ss >> token) {
            move = move_from_string(token);
            // check that the move is valid
            end = generate<LEGAL>(pos, moveList);
            if (std::find(moveList, end, move) == end) {
                std::cout << "Position: " << pos.sfen() << "\n";
                std::cout << "Move string: " << token << "\n";
                std::cout << "Parsed move: " << move << "\n";
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
                std::cout << "Game count: " << game_count << "\n";
            }
        }
        
        game_count++;
    }

    std::cout << "Total games: " << game_count << "\n";
    std::cout << "Total positions: " << book.size() << "\n";
    
    file.close();

    // Write the book to a .cpp file
    write_book(book, "searchengine/src/book_data.cpp");

    return 0;
}
