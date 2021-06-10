//
// Created by Aleksandr Lvov on 2021-06-10.
//
#include <array>
#include <optional>


int main() {
    return 0;
}
// ========== PIECES ========== //

class Piece {
public:
    enum class Type : char {
        Pawn, Bishop, Knight,
        Rook, King, Queen,
    };
    enum class Colour : char {
        White, Black
    };

    Piece(Colour colour, Type type)
    : colour(colour), type(type){};

    Piece() = delete;

    const Colour colour;
    const Type   type;
};


// ========== COORDINATES ========== //

// I am using 0x88 coordinates system,
// because it is simple yet more convenient than using 0...64
// To know more: https://www.chessprogramming.org/0x88#Off_the_Board
enum class Coords0x88 : unsigned char {};

inline bool offTheBoard(Coords0x88 coords) {
    return static_cast<unsigned char>(coords) & 0x88;
}

inline unsigned char coordsTo8x8(Coords0x88 coords) {
    return (static_cast<unsigned char>(coords) + (static_cast<unsigned char>(coords) & 7)) >> 1;
}

inline Coords0x88 coordsFrom8x8(unsigned char coords) {
    return Coords0x88(coords + (coords & ~7));
}

// ========== GAME ========== //

// Will use this later
// using Board = std::array<const std::optional<Piece>, 128>;

struct GameState {
    virtual std::optional<Piece> cell(Coords0x88) = 0;
};

