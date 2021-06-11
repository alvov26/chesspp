//
// Created by Aleksandr Lvov on 2021-06-10.
//

#include "chesspp.h"

#include <utility>

// ========== PIECES ========== //

Piece::Piece(Piece::Colour colour, Piece::Type type)
        : colour(colour), type(type){}

auto switched(Piece::Colour colour) -> Piece::Colour {
    return colour == Piece::Colour::White
    ? Piece::Colour::Black : Piece::Colour::White;
}

// ========== COORDINATES ========== //

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


// ========== DIRECTIONS ========== //

constexpr auto operator-(Direction lhs, Direction rhs) -> Direction {
    return Direction(static_cast<unsigned char>(lhs) - 0x77 + static_cast<unsigned char>(rhs));
}

constexpr auto apply(Coords0x88 c, Direction dir) -> Coords0x88 {
    return Coords0x88(static_cast<unsigned char>(c) + 0x77 - static_cast<unsigned char>(dir));
}

auto Rank(Coords0x88 coords) -> unsigned char {
    return static_cast<unsigned char>(coords) >> 4;
}

// ========== MOVES ========== //

MoveStage::MoveStage(Piece piece, Coords0x88 from, Coords0x88 to)
: piece(piece), from(from), to(to) {}

Move::Move(MoveStage first) noexcept
: first(std::move(first)), second({}) {}

Move::Move(MoveStage first, std::optional<MoveStage> second) noexcept
: first(std::move(first)), second(std::move(second)) {}

// ========== GAME STATE ========== //

GameState::GameState(Piece::Colour colourToMove, std::shared_ptr<const GameState> prev) noexcept
: colourToMove_(colourToMove), previousState_(std::move(prev)) {}

auto GameState::colourToMove() const -> Piece::Colour {
    return colourToMove_;
}

auto GameState::previousState() const -> std::shared_ptr<const GameState> {
    return previousState_;
}

auto GameState::availableMoves() const -> std::vector<Move> {
    using namespace Directions;

    static auto rookDirections    = DirectionList<4>{Up, Down, Right, Left};
    static auto bishopDirections  = DirectionList<4>{Up-Right, Down-Right, Up-Left, Down-Left};
    static auto queenAndKingDirections = DirectionList<8>{
        Up, Down, Right, Left, Up-Right, Down-Right, Up-Left, Down-Left
    };
    static auto knightDirections = DirectionList<8>{
        Up-Right-Right, Down-Right-Right, Up-Left-Left, Down-Left-Left,
        Right-Up-Up, Left-Up-Up, Right-Down-Down, Left-Down-Down
    };
    auto isValid = [&](Move move) -> bool {
        if (offTheBoard(move.first.to))
            return false;
        const auto piece = cell(move.first.to);
        if (piece && piece->colour == move.first.piece.colour)
            return false;
        if (move.second) {
            if (offTheBoard(move.second->to))
                return false;
            const auto piece2 = cell(move.second->to);
            if (piece2 && piece2->colour == move.second->piece.colour)
                return false;
        }
        return true;
    };

    auto isValidForPawnJustMoving = [&](Move move) -> bool {
        if (offTheBoard(move.first.to)) return false;
        const auto piece = cell(move.first.to);
        if (piece) return false;
        if (move.second) return false;
        return true;
    };

    auto isValidForPawnTaking = [&](Move move) -> bool {
        if (offTheBoard(move.first.to))
            return false;
        const auto piece = cell(move.first.to);
        if (!piece || piece->colour == move.first.piece.colour)
            return false;
        if (move.second) return false;
        return true;
    };

    auto result = std::vector<Move>();
    for (unsigned char c = 0; c < 0x78; ++c) {
        const auto currentCell  = Coords0x88(c);
        const auto currentPiece = cell(currentCell);
        if (!currentPiece || currentPiece->colour != colourToMove_) {
            continue;
        }

        auto generateRayMoves = [&](Direction direction, auto validator, unsigned n = 8){
            auto goal = apply(currentCell, direction);
            auto move = std::make_unique<Move>(MoveStage(*currentPiece, currentCell, goal));
            while (validator(*move) && n --> 0) {
                result.push_back(*move);
                goal = apply(goal, direction);
                move = std::make_unique<Move>(MoveStage(*currentPiece, currentCell, goal));
            };
        };

        switch (currentPiece->type) {
            case Piece::Type::Pawn:
                // TODO: add en passant
                if (currentPiece->colour == Piece::Colour::White){
                    generateRayMoves(Up-Left,  isValidForPawnTaking, 1);
                    generateRayMoves(Up-Right, isValidForPawnTaking, 1);
                    if (Rank(currentCell) == 1) generateRayMoves(Up, isValidForPawnJustMoving, 2);
                    else generateRayMoves(Up,  isValidForPawnJustMoving, 1);
                } else {
                    generateRayMoves(Down-Left,  isValidForPawnTaking, 1);
                    generateRayMoves(Down-Right, isValidForPawnTaking, 1);
                    if (Rank(currentCell) == 6) generateRayMoves(Down, isValidForPawnJustMoving, 2);
                    else generateRayMoves(Down,  isValidForPawnJustMoving, 1);
                }
                break;
            case Piece::Type::Bishop:
                for (const auto direction : bishopDirections)
                    generateRayMoves(direction, isValid);
                break;
            case Piece::Type::Knight:
                for (const auto direction : queenAndKingDirections) {
                    const auto move = Move(
                            MoveStage(*currentPiece, currentCell, apply(currentCell, direction)));
                    if (isValid(move)) result.push_back(move);
                }
                break;
            case Piece::Type::Rook:
                for (const auto direction : rookDirections)
                    generateRayMoves(direction, isValid);
                break;
            case Piece::Type::Queen:
                for (const auto direction : queenAndKingDirections)
                    generateRayMoves(direction, isValid);
                break;
            case Piece::Type::King:
                // TODO: generate castling
                for (const auto direction : queenAndKingDirections) {
                    const auto move = Move(
                            MoveStage(*currentPiece, currentCell, apply(currentCell, direction)));
                    if (isValid(move)) result.push_back(move);
                }
                break;
        }
    }
    return result;
}


FullGameState::FullGameState(
        Piece::Colour colour,
        std::shared_ptr<const GameState> prev,
        FullGameState::Board board) noexcept
        : GameState(colour, std::move(prev)), board_(std::move(board)) {}

auto FullGameState::cell(Coords0x88 coords) const -> std::optional<Piece> {
    return board_[static_cast<unsigned char>(coords)];
}

auto FullGameState::withMove(Move move) const -> std::shared_ptr<const GameState> {
    return std::make_shared<const PartialGameState>(switched(colourToMove_), shared_from_this(), move);
}


PartialGameState::PartialGameState(
        Piece::Colour colour,
        std::shared_ptr<const GameState> prev,
        Move move) noexcept
        : GameState(colour, std::move(prev)), move_(std::move(move)) {}

auto PartialGameState::cell(Coords0x88 coords) const -> std::optional<Piece> {
    if (move_.first.from == coords) {
        return {};
    } else if (move_.first.to == coords) {
        return move_.first.piece;
    } else if (move_.second) {
        if (move_.second->from == coords) {
            return {};
        } else if (move_.second->to == coords) {
            return move_.second->piece;
        }
    }
    return previousState_->cell(coords);
}

auto PartialGameState::withMove(Move move) const -> std::shared_ptr<const GameState> {
    return std::make_shared<const PartialGameState>(switched(colourToMove_), shared_from_this(), move);
}

