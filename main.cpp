//
// Created by Aleksandr Lvov on 2021-06-10.
//
#include <array>
#include <optional>
#include <memory>

int main() {
    return 0;
}

// TODO: write tests for all these classes.
// It should be done using GTest framework.


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

inline auto offTheBoard(Coords0x88 coords) -> bool;
inline auto coordsTo8x8(Coords0x88 coords) -> unsigned char;
inline auto coordsFrom8x8(unsigned char coords) -> Coords0x88;

auto offTheBoard(Coords0x88 coords) -> bool {
    return static_cast<unsigned char>(coords) & 0x88;
}

auto coordsTo8x8(Coords0x88 coords) -> unsigned char {
    return (static_cast<unsigned char>(coords) +
           (static_cast<unsigned char>(coords) & 7)) >> 1;
}

auto coordsFrom8x8(unsigned char coords) -> Coords0x88 {
    return Coords0x88(coords + (coords & ~7));
}


// ========== GAME ========== //

struct MoveStage {
    MoveStage(Piece piece, Coords0x88 from, Coords0x88 to)
    : piece(piece), from(from), to(to) {}

    const Piece piece;
    const Coords0x88 from, to;
};

// First member is the main part.
// Second member can be used for castling and en passant.
using Move = const std::pair<MoveStage, std::optional<MoveStage>>;

struct GameState {
    virtual auto cell(Coords0x88) const -> std::optional<Piece> = 0;
    virtual auto colourToMove() const -> Piece::Colour = 0;
    virtual auto withMove(Move) const -> std::shared_ptr<GameState> = 0;
    virtual auto previousState() const -> std::shared_ptr<GameState> final;
private:
    const std::shared_ptr<GameState> previousState_;
};

auto GameState::previousState() const -> std::shared_ptr<GameState> {
    return previousState_;
}

struct FullGameState final : GameState {
    using Board = std::array<std::optional<Piece>, 128>;

    auto cell(Coords0x88 coords) const -> std::optional<Piece> final;
    auto colourToMove() const -> Piece::Colour final;
    auto withMove(Move) const -> std::shared_ptr<GameState> final;

private:
    Board board_;
    Piece::Colour colourToMove_ = Piece::Colour::White;
};

auto FullGameState::cell(Coords0x88 coords) const -> std::optional<Piece> {
    return board_[static_cast<unsigned char>(coords)];
}

auto FullGameState::colourToMove() const -> Piece::Colour {
    return colourToMove_;
}
