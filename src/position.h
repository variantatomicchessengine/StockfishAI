/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2015 Marco Costalba, Joona Kiiski, Tord Romstad
  Copyright (C) 2015-2016 Marco Costalba, Joona Kiiski, Gary Linscott, Tord Romstad

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef POSITION_H_INCLUDED
#define POSITION_H_INCLUDED

#include <cassert>
#include <cstddef>  // For offsetof()
#include <deque>
#include <memory>   // For std::unique_ptr
#include <string>
#include <vector>

#include "bitboard.h"
#include "types.h"

#define STANDARD_VARIANT 0
#define CHESS960_VARIANT 1 << 1
#define ATOMIC_VARIANT 1 << 2
#define HORDE_VARIANT 1 << 3
#define HOUSE_VARIANT 1 << 4
#define KOTH_VARIANT 1 << 5
#define RACE_VARIANT 1 << 6
#define THREECHECK_VARIANT 1 << 7

class Position;
class Thread;

namespace PSQT {

  extern Score psq[COLOR_NB][PIECE_TYPE_NB][SQUARE_NB];

  void init();
}

/// CheckInfo struct is initialized at constructor time and keeps info used to
/// detect if a move gives check.

struct CheckInfo {

  explicit CheckInfo(const Position&);

  Bitboard dcCandidates;
  Bitboard pinned;
  Bitboard checkSquares[PIECE_TYPE_NB];
  Square   ksq;
};


/// StateInfo struct stores information needed to restore a Position object to
/// its previous state when we retract a move. Whenever a move is made on the
/// board (by calling Position::do_move), a StateInfo object must be passed.

struct StateInfo {

  // Copied when making a move
  Key    pawnKey;
  Key    materialKey;
  Value  nonPawnMaterial[COLOR_NB];
  int    castlingRights;
  int    rule50;
  int    pliesFromNull;
#ifdef THREECHECK
  Checks checksGiven[COLOR_NB];
#endif
  Score  psq;
  Square epSquare;

  // Not copied when making a move
  Key        key;
  Bitboard   checkersBB;
  PieceType  capturedType;
#ifdef ATOMIC
  Piece      blast[SQUARE_NB];
#endif
  StateInfo* previous;
};

// In a std::deque references to elements are unaffected upon resizing
typedef std::unique_ptr<std::deque<StateInfo>> StateListPtr;


/// Position class stores information regarding the board representation as
/// pieces, side to move, hash keys, castling info, etc. Important methods are
/// do_move() and undo_move(), used by the search to update node info when
/// traversing the search tree.

class Position {

public:
  static void init();

  Position() = default;
  Position(const Position&) = delete;
  Position& operator=(const Position&) = delete;

  // FEN string input/output
  Position& set(const std::string& fenStr, int v, StateInfo* si, Thread* th);
  const std::string fen() const;

  // Position representation
  Bitboard pieces() const;
  Bitboard pieces(PieceType pt) const;
  Bitboard pieces(PieceType pt1, PieceType pt2) const;
  Bitboard pieces(Color c) const;
  Bitboard pieces(Color c, PieceType pt) const;
  Bitboard pieces(Color c, PieceType pt1, PieceType pt2) const;
  Piece piece_on(Square s) const;
  Square ep_square() const;
  bool empty(Square s) const;
  template<PieceType Pt> int count(Color c) const;
  template<PieceType Pt> const Square* squares(Color c) const;
  template<PieceType Pt> Square square(Color c) const;

  // Castling
  int can_castle(Color c) const;
  int can_castle(CastlingRight cr) const;
  bool castling_impeded(CastlingRight cr) const;
  Square castling_rook_square(CastlingRight cr) const;

  // Checking
  Bitboard checkers() const;
  Bitboard discovered_check_candidates() const;
  Bitboard pinned_pieces(Color c) const;

  // Attacks to/from a given square
  Bitboard attackers_to(Square s) const;
  Bitboard attackers_to(Square s, Bitboard occupied) const;
  Bitboard attacks_from(Piece pc, Square s) const;
  template<PieceType> Bitboard attacks_from(Square s) const;
  template<PieceType> Bitboard attacks_from(Square s, Color c) const;

  // Properties of moves
  bool legal(Move m, Bitboard pinned) const;
  bool pseudo_legal(const Move m) const;
  bool capture(Move m) const;
  bool capture_or_promotion(Move m) const;
  bool gives_check(Move m, const CheckInfo& ci) const;
  bool advanced_pawn_push(Move m) const;
  Piece moved_piece(Move m) const;
  PieceType captured_piece_type() const;

  // Piece specific
  bool pawn_passed(Color c, Square s) const;
  bool opposite_bishops() const;

  // Doing and undoing moves
  void do_move(Move m, StateInfo& st, bool givesCheck);
  void undo_move(Move m);
  void do_null_move(StateInfo& st);
  void undo_null_move();

  // Static exchange evaluation
  Value see(Move m) const;
  Value see_sign(Move m) const;

  // Accessing hash keys
  Key key() const;
  Key key_after(Move m) const;
  Key exclusion_key() const;
  Key material_key() const;
  Key pawn_key() const;

  // Other properties of the position
  Color side_to_move() const;
  Phase game_phase() const;
  int game_ply() const;
  int variant() const;
  bool is_chess960() const;
#ifdef ATOMIC
  bool is_atomic() const;
  bool is_atomic_win() const;
  bool is_atomic_loss() const;
#endif
#ifdef HORDE
  bool is_horde() const;
  bool is_horde_loss() const;
#endif
#ifdef HOUSE
  bool is_house() const;
#endif
#ifdef KOTH
  bool is_koth() const;
  bool is_koth_win() const;
  bool is_koth_loss() const;
  int koth_distance(Color c) const;
#endif
#ifdef RACE
  bool is_race() const;
  bool is_race_win() const;
  bool is_race_draw() const;
  bool is_race_loss() const;
#endif
#ifdef THREECHECK
  bool is_three_check() const;
  bool is_three_check_win() const;
  bool is_three_check_loss() const;
  int checks_count() const;
  Checks checks_given() const;
  Checks checks_taken() const;
#endif
  Thread* this_thread() const;
  uint64_t nodes_searched() const;
  void set_nodes_searched(uint64_t n);
  bool is_draw() const;
  int rule50_count() const;
  Score psq_score() const;
  Value non_pawn_material(Color c) const;

  // Position consistency check, for debugging
  bool pos_is_ok(int* failedStep = nullptr) const;
  void flip();

private:
  // Initialization helpers (used while setting up a position)
  void set_castling_right(Color c, Square rfrom);
  void set_state(StateInfo* si) const;

  // Other helpers
  Bitboard check_blockers(Color c, Color kingColor) const;
  void put_piece(Color c, PieceType pt, Square s);
  void remove_piece(Color c, PieceType pt, Square s);
  void move_piece(Color c, PieceType pt, Square from, Square to);
  template<bool Do>
  void do_castling(Color us, Square from, Square& to, Square& rfrom, Square& rto);

  // Data members
  Piece board[SQUARE_NB];
  Bitboard byTypeBB[PIECE_TYPE_NB];
  Bitboard byColorBB[COLOR_NB];
  int pieceCount[COLOR_NB][PIECE_TYPE_NB];
#ifdef HORDE
  Square pieceList[COLOR_NB][PIECE_TYPE_NB][SQUARE_NB];
#else
  Square pieceList[COLOR_NB][PIECE_TYPE_NB][16];
#endif
  int index[SQUARE_NB];
  int castlingRightsMask[SQUARE_NB];
  Square castlingRookSquare[CASTLING_RIGHT_NB];
  Bitboard castlingPath[CASTLING_RIGHT_NB];
  uint64_t nodes;
  int gamePly;
  Color sideToMove;
  Thread* thisThread;
  StateInfo* st;
  int var;

};

extern std::ostream& operator<<(std::ostream& os, const Position& pos);

inline Color Position::side_to_move() const {
  return sideToMove;
}

inline bool Position::empty(Square s) const {
  return board[s] == NO_PIECE;
}

inline Piece Position::piece_on(Square s) const {
  return board[s];
}

inline Piece Position::moved_piece(Move m) const {
  return board[from_sq(m)];
}

inline Bitboard Position::pieces() const {
  return byTypeBB[ALL_PIECES];
}

inline Bitboard Position::pieces(PieceType pt) const {
  return byTypeBB[pt];
}

inline Bitboard Position::pieces(PieceType pt1, PieceType pt2) const {
  return byTypeBB[pt1] | byTypeBB[pt2];
}

inline Bitboard Position::pieces(Color c) const {
  return byColorBB[c];
}

inline Bitboard Position::pieces(Color c, PieceType pt) const {
  return byColorBB[c] & byTypeBB[pt];
}

inline Bitboard Position::pieces(Color c, PieceType pt1, PieceType pt2) const {
  return byColorBB[c] & (byTypeBB[pt1] | byTypeBB[pt2]);
}

template<PieceType Pt> inline int Position::count(Color c) const {
  return pieceCount[c][Pt];
}

template<PieceType Pt> inline const Square* Position::squares(Color c) const {
  return pieceList[c][Pt];
}

template<PieceType Pt> inline Square Position::square(Color c) const {
#ifdef HORDE
  if (is_horde() && c == WHITE)
  {
      assert(pieceCount[c][Pt] == 0);
      return SQ_NONE;
  }
#endif
#ifdef ATOMIC
  if (is_atomic() && pieceCount[c][Pt] == 0)
      return SQ_NONE;
#endif
  assert(pieceCount[c][Pt] == 1);
  return pieceList[c][Pt][0];
}

#ifdef THREECHECK
inline bool Position::is_three_check() const {
  return var & THREECHECK_VARIANT;
}

inline bool Position::is_three_check_win() const {
  return st->checksGiven[sideToMove] == CHECKS_3;
}

inline bool Position::is_three_check_loss() const {
  return st->checksGiven[~sideToMove] == CHECKS_3;
}

inline int Position::checks_count() const {
  return st->checksGiven[WHITE] + st->checksGiven[BLACK];
}

inline Checks Position::checks_given() const {
  return st->checksGiven[sideToMove];
}

inline Checks Position::checks_taken() const {
  return st->checksGiven[~sideToMove];
}
#endif

inline Square Position::ep_square() const {
  return st->epSquare;
}

inline int Position::can_castle(CastlingRight cr) const {
  return st->castlingRights & cr;
}

inline int Position::can_castle(Color c) const {
  return st->castlingRights & ((WHITE_OO | WHITE_OOO) << (2 * c));
}

inline bool Position::castling_impeded(CastlingRight cr) const {
  return byTypeBB[ALL_PIECES] & castlingPath[cr];
}

inline Square Position::castling_rook_square(CastlingRight cr) const {
  return castlingRookSquare[cr];
}

template<PieceType Pt>
inline Bitboard Position::attacks_from(Square s) const {
  return  Pt == BISHOP || Pt == ROOK ? attacks_bb<Pt>(s, byTypeBB[ALL_PIECES])
        : Pt == QUEEN  ? attacks_from<ROOK>(s) | attacks_from<BISHOP>(s)
        : StepAttacksBB[Pt][s];
}

template<>
inline Bitboard Position::attacks_from<PAWN>(Square s, Color c) const {
  return StepAttacksBB[make_piece(c, PAWN)][s];
}

inline Bitboard Position::attacks_from(Piece pc, Square s) const {
  return attacks_bb(pc, s, byTypeBB[ALL_PIECES]);
}

inline Bitboard Position::attackers_to(Square s) const {
  return attackers_to(s, byTypeBB[ALL_PIECES]);
}

inline Bitboard Position::checkers() const {
  return st->checkersBB;
}

inline Bitboard Position::discovered_check_candidates() const {
  return check_blockers(sideToMove, ~sideToMove);
}

inline Bitboard Position::pinned_pieces(Color c) const {
  return check_blockers(c, c);
}

inline bool Position::pawn_passed(Color c, Square s) const {
#ifdef RACE
  if (is_race())
    return true;
#endif
#ifdef HORDE
  if (is_horde() && c == WHITE)
      return !(pieces(~c, PAWN) & forward_bb(c, s));
#endif
  return !(pieces(~c, PAWN) & passed_pawn_mask(c, s));
}

inline bool Position::advanced_pawn_push(Move m) const {
#ifdef RACE
  if (is_race())
    return   type_of(moved_piece(m)) == KING
          && rank_of(from_sq(m)) > RANK_4;
#endif
  return   type_of(moved_piece(m)) == PAWN
        && relative_rank(sideToMove, from_sq(m)) > RANK_4;
}

inline Key Position::key() const {
  return st->key;
}

inline Key Position::pawn_key() const {
  return st->pawnKey;
}

inline Key Position::material_key() const {
  return st->materialKey;
}

inline Score Position::psq_score() const {
  return st->psq;
}

inline Value Position::non_pawn_material(Color c) const {
  return st->nonPawnMaterial[c];
}

inline int Position::game_ply() const {
  return gamePly;
}

inline int Position::rule50_count() const {
  return st->rule50;
}

inline uint64_t Position::nodes_searched() const {
  return nodes;
}

inline void Position::set_nodes_searched(uint64_t n) {
  nodes = n;
}

inline bool Position::opposite_bishops() const {
  return   pieceCount[WHITE][BISHOP] == 1
        && pieceCount[BLACK][BISHOP] == 1
        && opposite_colors(square<BISHOP>(WHITE), square<BISHOP>(BLACK));
}

#ifdef ATOMIC
inline bool Position::is_atomic() const {
  return var & ATOMIC_VARIANT;
}

// Loss if king is captured (Atomic)
inline bool Position::is_atomic_win() const {
  return count<KING>(~sideToMove) == 0;
}

// Loss if king is captured (Atomic)
inline bool Position::is_atomic_loss() const {
  return count<KING>(sideToMove) == 0;
}
#endif

#ifdef HORDE
inline bool Position::is_horde() const {
  return var & HORDE_VARIANT;
}

// Loss if horde is captured (Horde)
inline bool Position::is_horde_loss() const {
  return count<ALL_PIECES>(WHITE) == 0;
}
#endif

#ifdef HOUSE
inline bool Position::is_house() const {
  return var & HOUSE_VARIANT;
}
#endif

#ifdef KOTH
inline bool Position::is_koth() const {
  return var & KOTH_VARIANT;
}

// Win if king is in the center (KOTH)
inline bool Position::is_koth_win() const {
  Square ksq = square<KING>(sideToMove);
  return (rank_of(ksq) == RANK_4 || rank_of(ksq) == RANK_5) &&
         (file_of(ksq) == FILE_D || file_of(ksq) == FILE_E);
}

// Loss if king is in the center (KOTH)
inline bool Position::is_koth_loss() const {
  Square ksq = square<KING>(~sideToMove);
  return (rank_of(ksq) == RANK_4 || rank_of(ksq) == RANK_5) &&
         (file_of(ksq) == FILE_D || file_of(ksq) == FILE_E);
}

inline int Position::koth_distance(Color c) const {
  Square ksq = square<KING>(c);
  return (distance(ksq, SQ_D4) + distance(ksq, SQ_E4) +
          distance(ksq, SQ_D5) + distance(ksq, SQ_E5)) / 4;
}
#endif

#ifdef RACE
inline bool Position::is_race() const {
  return var & RACE_VARIANT;
}

// Win if king is on the eighth rank (Racing Kings)
inline bool Position::is_race_win() const {
  return rank_of(square<KING>(sideToMove)) == RANK_8;
}

// Draw if kings are on the eighth rank (Racing Kings)
inline bool Position::is_race_draw() const {
  return is_race_win() && is_race_loss();
}

// Loss if king is on the eighth rank (Racing Kings)
inline bool Position::is_race_loss() const {
  return (sideToMove == WHITE || rank_of(square<KING>(sideToMove)) < RANK_7) &&
         rank_of(square<KING>(~sideToMove)) == RANK_8;
}
#endif

inline int Position::variant() const {
  return var;
}

inline bool Position::is_chess960() const {
  return var & CHESS960_VARIANT;
}

inline bool Position::capture_or_promotion(Move m) const {

  assert(is_ok(m));
#ifdef RACE
  if (is_race())
  {
    Square from = from_sq(m), to = to_sq(m);
    return (type_of(board[from]) == KING && rank_of(to) >= rank_of(from)) || !empty(to);
  }
#endif
  return type_of(m) != NORMAL ? type_of(m) != CASTLING : !empty(to_sq(m));
}

inline bool Position::capture(Move m) const {

  // Castling is encoded as "king captures the rook"
  assert(is_ok(m));
  return (!empty(to_sq(m)) && type_of(m) != CASTLING) || type_of(m) == ENPASSANT;
}

inline PieceType Position::captured_piece_type() const {
  return st->capturedType;
}

inline Thread* Position::this_thread() const {
  return thisThread;
}

inline void Position::put_piece(Color c, PieceType pt, Square s) {

  board[s] = make_piece(c, pt);
  byTypeBB[ALL_PIECES] |= s;
  byTypeBB[pt] |= s;
  byColorBB[c] |= s;
  index[s] = pieceCount[c][pt]++;
  pieceList[c][pt][index[s]] = s;
  pieceCount[c][ALL_PIECES]++;
}

inline void Position::remove_piece(Color c, PieceType pt, Square s) {

  // WARNING: This is not a reversible operation. If we remove a piece in
  // do_move() and then replace it in undo_move() we will put it at the end of
  // the list and not in its original place, it means index[] and pieceList[]
  // are not guaranteed to be invariant to a do_move() + undo_move() sequence.
  byTypeBB[ALL_PIECES] ^= s;
  byTypeBB[pt] ^= s;
  byColorBB[c] ^= s;
#ifdef ATOMIC
  if (is_atomic())
      board[s] = NO_PIECE;
#endif
  /* board[s] = NO_PIECE;  Not needed, overwritten by the capturing one */
  Square lastSquare = pieceList[c][pt][--pieceCount[c][pt]];
  index[lastSquare] = index[s];
  pieceList[c][pt][index[lastSquare]] = lastSquare;
  pieceList[c][pt][pieceCount[c][pt]] = SQ_NONE;
  pieceCount[c][ALL_PIECES]--;
}

inline void Position::move_piece(Color c, PieceType pt, Square from, Square to) {

  // index[from] is not updated and becomes stale. This works as long as index[]
  // is accessed just by known occupied squares.
  Bitboard from_to_bb = SquareBB[from] ^ SquareBB[to];
  byTypeBB[ALL_PIECES] ^= from_to_bb;
  byTypeBB[pt] ^= from_to_bb;
  byColorBB[c] ^= from_to_bb;
  board[from] = NO_PIECE;
  board[to] = make_piece(c, pt);
  index[to] = index[from];
  pieceList[c][pt][index[to]] = to;
}

#endif // #ifndef POSITION_H_INCLUDED
