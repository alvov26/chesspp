//
// Created by Aleksandr Lvov on 2021-06-10.
//

#pragma once

#include <array>
#include <optional>
#include <memory>
#include <vector>
#include <iterator>

// TODO: write tests for all these classes.
// It should be done using GTest framework.


// ========== PIECES ========== //

class Piece {
public:
    enum class Type : unsigned char {
        Pawn, Bishop, Knight,
        Rook, King, Queen,
        };
    enum class Colour : unsigned char {
        White, Black
    };

    Piece(Colour colour, Type type);
    Piece() = delete;

    const Colour colour;
    const Type   type;
};

inline auto switched(Piece::Colour) -> Piece::Colour;

// ========== COORDINATES ========== //

// I am using 0x88 coordinates system,
// because it is simple yet more convenient than using 0...64
// To know more: https://www.chessprogramming.org/0x88#Off_the_Board
enum class Coords0x88 : unsigned char {};

inline auto Rank(Coords0x88) -> unsigned char;

inline auto offTheBoard(Coords0x88 coords) -> bool;
inline auto coordsTo8x8(Coords0x88 coords) -> unsigned char;
inline auto coordsFrom8x8(unsigned char coords) -> Coords0x88;

// ========== DIRECTIONS ========== //

// they will be useful in move generation

enum class Direction : unsigned char {};
namespace Directions {
    static const Direction Up    = Direction(0x77 + 0x00 - 0x10);
    static const Direction Down  = Direction(0x77 + 0x10 - 0x00);
    static const Direction Left  = Direction(0x77 + 0x01 - 0x00);
    static const Direction Right = Direction(0x77 + 0x00 - 0x01);

    template <size_t N>
    using DirectionList = const std::array<const Direction, N>;
}

// Can be used as Up-Right-Right for Knight, for example
constexpr auto operator- (Direction, Direction) -> Direction;

constexpr auto apply(Coords0x88 c, Direction dir) -> Coords0x88;

// ========== MOVE ========== //

struct MoveStage {
    MoveStage(Piece piece, Coords0x88 from, Coords0x88 to);

    const Piece piece;
    const Coords0x88 from, to;
};

// First member is the main part.
// Second member can be used for castling and en passant.
//using Move = std::pair<MoveStage, std::optional<MoveStage>>;

struct Move {
    explicit Move(MoveStage first) noexcept;
    Move(MoveStage first, std::optional<MoveStage> second) noexcept;

    const MoveStage first;
    const std::optional<MoveStage> second;
};


// ========== GAME STATE ========== //

struct GameState : std::enable_shared_from_this<GameState> {
    GameState(Piece::Colour, std::shared_ptr<const GameState>) noexcept;

    virtual auto cell(Coords0x88) const -> std::optional<Piece> = 0;
    virtual auto withMove(Move) const -> std::shared_ptr<const GameState> = 0;

    virtual auto previousState() const -> std::shared_ptr<const GameState> final;
    virtual auto availableMoves() const -> std::vector<Move> final;
    virtual auto colourToMove() const -> Piece::Colour final;

    virtual ~GameState() = default;

protected:
    const Piece::Colour colourToMove_;
    const std::shared_ptr<const GameState> previousState_;
};

struct FullGameState final : GameState {
    using Board = std::array<std::optional<Piece>, 128>;

    FullGameState(Piece::Colour, std::shared_ptr<const GameState>, Board) noexcept;

    auto cell(Coords0x88) const -> std::optional<Piece> final;
    auto withMove(Move) const -> std::shared_ptr<const GameState> final;

private:
    const Board board_;
};

struct PartialGameState final : GameState {
    PartialGameState(Piece::Colour, std::shared_ptr<const GameState>, Move) noexcept;

    auto cell(Coords0x88) const -> std::optional<Piece> final;
    auto withMove(Move) const -> std::shared_ptr<const GameState> final;

private:
    const Move move_;
};


